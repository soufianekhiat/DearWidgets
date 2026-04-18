// dear_widgets_image_inspector.cpp — Color-managed raw-buffer image viewer.
//
// Companion to ImageViewer for inspecting raw image data of any sample type
// (U8/I8/U16/I16/U32/I32/U64/I64/F16/F32/F64) and channel count (1..4) with
// Halide-style strided layouts. Uses an HLSL uber-shader for shader-side
// decode + color management. The user's bytes are uploaded once into a
// packed RGBA32F texture; per-frame CPU cost is uniform updates only.
//
// Compiled as its own translation unit; resolved by the linker.
#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"

namespace ImWidgets {

// ============================================================================
// Internal helpers
// ============================================================================

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER

// ----------------------------------------------------------------------------
// IEEE half → float decode (used by CPU inspector readback for F16)
// ----------------------------------------------------------------------------
static float II_HalfToFloat( unsigned short h )
{
	unsigned int sign = (unsigned int)(h >> 15) & 0x1u;
	unsigned int exp  = (unsigned int)(h >> 10) & 0x1Fu;
	unsigned int mant = (unsigned int)(h)       & 0x3FFu;
	unsigned int f;
	if ( exp == 0 )
	{
		if ( mant == 0 )
		{
			f = sign << 31;
		}
		else
		{
			// Subnormal — normalize
			while ( ( mant & 0x400u ) == 0 ) { mant <<= 1; exp--; }
			exp++;
			mant &= ~0x400u;
			f = ( sign << 31 ) | ( ( exp + ( 127 - 15 ) ) << 23 ) | ( mant << 13 );
		}
	}
	else if ( exp == 31 )
	{
		f = ( sign << 31 ) | 0x7F800000u | ( mant << 13 );
	}
	else
	{
		f = ( sign << 31 ) | ( ( exp + ( 127 - 15 ) ) << 23 ) | ( mant << 13 );
	}
	float out;
	memcpy( &out, &f, 4 );
	return out;
}

// ----------------------------------------------------------------------------
// Gamut → XYZ (D65 unless noted) — standard published primaries
// ----------------------------------------------------------------------------
// Index by ImImageInspector_Gamut enum.
static const float kGamutToXYZ[ ImImageInspector_Gamut_COUNT ][ 9 ] =
{
	// Rec.709 / sRGB (D65)
	{ 0.4124564f, 0.3575761f, 0.1804375f,
	  0.2126729f, 0.7151522f, 0.0721750f,
	  0.0193339f, 0.1191920f, 0.9503041f },
	// Rec.2020 (D65)
	{ 0.6369580f, 0.1446169f, 0.1688810f,
	  0.2627002f, 0.6779981f, 0.0593017f,
	  0.0000000f, 0.0280727f, 1.0609851f },
	// DCI-P3 (D65 in this build — Display vs theatrical share primaries)
	{ 0.4451698f, 0.2771344f, 0.1722827f,
	  0.2094917f, 0.7215953f, 0.0689131f,
	  0.0000000f, 0.0470606f, 0.9073554f },
	// Display P3 (D65)
	{ 0.4865709f, 0.2656677f, 0.1982173f,
	  0.2289746f, 0.6917385f, 0.0792869f,
	  0.0000000f, 0.0451134f, 1.0439444f },
	// Adobe RGB (1998) (D65)
	{ 0.5767309f, 0.1855540f, 0.1881852f,
	  0.2973769f, 0.6273491f, 0.0752741f,
	  0.0270343f, 0.0706872f, 0.9911085f },
	// ProPhoto / ROMM RGB (D50)
	{ 0.7976749f, 0.1351917f, 0.0313534f,
	  0.2880402f, 0.7118741f, 0.0000857f,
	  0.0000000f, 0.0000000f, 0.8252100f },
	// ACES AP0 (D60)
	{ 0.9525523959f, 0.0000000000f, 0.0000936786f,
	  0.3439664498f, 0.7281660966f,-0.0721325464f,
	  0.0000000000f, 0.0000000000f, 1.0088251844f },
	// ACES AP1 / ACEScg (D60)
	{ 0.6624541811f, 0.1340042065f, 0.1561876870f,
	  0.2722287168f, 0.6740817658f, 0.0536895174f,
	 -0.0055746495f, 0.0040607335f, 1.0103391003f },
};

// ----------------------------------------------------------------------------
// 3×3 matrix helpers
// ----------------------------------------------------------------------------
static void II_Mat3Mul( const float a[ 9 ], const float b[ 9 ], float out[ 9 ] )
{
	float r[ 9 ];
	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			r[ i * 3 + j ] = a[ i * 3 + 0 ] * b[ 0 + j ]
			               + a[ i * 3 + 1 ] * b[ 3 + j ]
			               + a[ i * 3 + 2 ] * b[ 6 + j ];
	memcpy( out, r, sizeof( r ) );
}

static bool II_Mat3Inverse( const float m[ 9 ], float out[ 9 ] )
{
	float det =
		  m[ 0 ] * ( m[ 4 ] * m[ 8 ] - m[ 5 ] * m[ 7 ] )
		- m[ 1 ] * ( m[ 3 ] * m[ 8 ] - m[ 5 ] * m[ 6 ] )
		+ m[ 2 ] * ( m[ 3 ] * m[ 7 ] - m[ 4 ] * m[ 6 ] );
	if ( det == 0.0f ) return false;
	float inv = 1.0f / det;
	float r[ 9 ];
	r[ 0 ] =  ( m[ 4 ] * m[ 8 ] - m[ 5 ] * m[ 7 ] ) * inv;
	r[ 1 ] = -( m[ 1 ] * m[ 8 ] - m[ 2 ] * m[ 7 ] ) * inv;
	r[ 2 ] =  ( m[ 1 ] * m[ 5 ] - m[ 2 ] * m[ 4 ] ) * inv;
	r[ 3 ] = -( m[ 3 ] * m[ 8 ] - m[ 5 ] * m[ 6 ] ) * inv;
	r[ 4 ] =  ( m[ 0 ] * m[ 8 ] - m[ 2 ] * m[ 6 ] ) * inv;
	r[ 5 ] = -( m[ 0 ] * m[ 5 ] - m[ 2 ] * m[ 3 ] ) * inv;
	r[ 6 ] =  ( m[ 3 ] * m[ 7 ] - m[ 4 ] * m[ 6 ] ) * inv;
	r[ 7 ] = -( m[ 0 ] * m[ 7 ] - m[ 1 ] * m[ 6 ] ) * inv;
	r[ 8 ] =  ( m[ 0 ] * m[ 4 ] - m[ 1 ] * m[ 3 ] ) * inv;
	memcpy( out, r, sizeof( r ) );
	return true;
}

// Build a 3×3 transform from gamut `from` → gamut `to`. Identity if equal.
static void II_BuildGamutTransform( int from, int to, float out[ 9 ] )
{
	if ( from == to )
	{
		memset( out, 0, sizeof( float ) * 9 );
		out[ 0 ] = out[ 4 ] = out[ 8 ] = 1.0f;
		return;
	}
	if ( from < 0 || from >= ImImageInspector_Gamut_COUNT ) from = ImImageInspector_Gamut_Rec709;
	if ( to   < 0 || to   >= ImImageInspector_Gamut_COUNT ) to   = ImImageInspector_Gamut_Rec709;

	float to_xyz_inv[ 9 ];
	II_Mat3Inverse( kGamutToXYZ[ to ], to_xyz_inv );
	II_Mat3Mul( to_xyz_inv, kGamutToXYZ[ from ], out );
}

// ----------------------------------------------------------------------------
// Per-frame drawcall pool (mirrors marker pattern)
// ----------------------------------------------------------------------------
// Each ImageInspector draw allocates a snapshot of its uniforms; the callback
// reads from it. Pool is double-buffered: we swap pools on frame boundary
// so the previous frame's data stays alive long enough to be rendered.
struct ImageInspectorParams
{
	float    imgSize[ 4 ];          // (w, h, 1/w, 1/h)
	float    panZoom[ 4 ];          // (pan.x, pan.y, zoom, fitScale)
	float    viewportPx[ 4 ];       // (widget_w, widget_h, 1/widget_w, 1/widget_h)
	unsigned packedTexDims[ 4 ];    // (texW, texH, totalBytes, log2texW)
	unsigned layoutPack[ 4 ];       // (gpu_x_stride, gpu_y_stride, gpu_c_stride, 0)
	unsigned formatPack[ 4 ];       // (sample_type, channels, mosaic_pattern, filter)
	float    exposureParams[ 4 ];   // (exposure_stops, black, white, gamma)
	float    tempTint[ 4 ];         // (temp, tint, 0, 0)
	float    inGamut_r0[ 4 ];
	float    inGamut_r1[ 4 ];
	float    inGamut_r2[ 4 ];
	float    outGamut_r0[ 4 ];
	float    outGamut_r1[ 4 ];
	float    outGamut_r2[ 4 ];
	unsigned pipelinePack[ 4 ];     // (input_xfer, output_xfer, tonemap, false_color)
	float    channelMask[ 4 ];
	float    nanColor[ 4 ];
	unsigned modePack[ 4 ];         // (mosaic_mode, 0, 0, 0)
};

struct ImageInspectorDrawCallData
{
	ImPlatform_ShaderProgram program;
	ImTextureID              packedTexture;
	ImageInspectorParams     params;
};

static ImVector<ImageInspectorDrawCallData*> s_iiDrawCalls[ 2 ];
static int s_iiLastFrame = -1;
static int s_iiPoolIdx   = 0;

static void ImageInspectorFrameCleanup()
{
	int curFrame = ImGui::GetFrameCount();
	if ( curFrame != s_iiLastFrame )
	{
		s_iiPoolIdx = 1 - s_iiPoolIdx;
		for ( int i = 0; i < s_iiDrawCalls[ s_iiPoolIdx ].Size; i++ )
			IM_DELETE( s_iiDrawCalls[ s_iiPoolIdx ][ i ] );
		s_iiDrawCalls[ s_iiPoolIdx ].clear();
		s_iiLastFrame = curFrame;
	}
}

// ----------------------------------------------------------------------------
// Render callback — uploads the whole cbuffer in one shot, then activates
// the custom shader. Mirrors the marker widget pattern. The C++ struct
// layout above MUST match the HLSL ImageInspectorParams cbuffer field order.
// ----------------------------------------------------------------------------
static void ImageInspectorShaderCallback( const ImDrawList* /*parent_list*/, const ImDrawCmd* cmd )
{
	ImageInspectorDrawCallData* data = ( ImageInspectorDrawCallData* )cmd->UserCallbackData;
	if ( !data || !data->program )
		return;

	ImPlatform_SetShaderUniform( data->program, "ImageInspectorParams",
	                             &data->params, ( unsigned int )sizeof( data->params ) );
	ImPlatform_BeginCustomShader_Render( data->program );
}

// ----------------------------------------------------------------------------
// EnsurePackedTexture — tight-pack the source bytes and upload to GPU
// ----------------------------------------------------------------------------
// Re-uploads only when the buffer's version changes. The packed texture is a
// flat byte array reshaped as a 2D RGBA32F texture (4096 wide, height as
// needed). Tight-packed source data hits the fast memcpy path; arbitrary
// strides take the slow per-pixel walk.
static const int kPackedTexWidth = 4096;     // Texels (RGBA32F = 16 bytes each)

static void EnsurePackedTexture( const ImImageBuffer& buffer, ImImageInspectorState& state )
{
	if ( state.PackedTexture != ImTextureID_Invalid
	  && state.LastUploadedVersion == buffer.version
	  && state.LastUploadedWidth == ( int )buffer.width
	  && state.LastUploadedHeight == ( int )buffer.height
	  && state.LastUploadedChannels == ( int )buffer.channels
	  && state.LastUploadedSampleType == ( int )buffer.type )
		return;

	if ( buffer.host == NULL || buffer.width == 0 || buffer.height == 0 || buffer.channels == 0 || buffer.channels > 4 )
	{
		state.BufferTooLarge = true;
		return;
	}

	size_t sample_size = ImPlatform_SampleTypeSize( buffer.type );
	size_t total_bytes = ( size_t )buffer.width * ( size_t )buffer.height * ( size_t )buffer.channels * sample_size;
	if ( total_bytes == 0 )
	{
		state.BufferTooLarge = true;
		return;
	}

	// Compute texture dimensions
	size_t texels_needed = ( total_bytes + 15 ) / 16;
	int    tex_w = kPackedTexWidth;
	int    tex_h = ( int )( ( texels_needed + tex_w - 1 ) / tex_w );
	if ( tex_h > 16384 )    // Conservative max texture height
	{
		state.BufferTooLarge = true;
		return;
	}
	state.BufferTooLarge = false;

	// Allocate scratch padded to RGBA32F texel boundary. Only the tail padding
	// past total_bytes needs zeroing (done after the source copy below); the
	// full-buffer memset would waste ~20 ms per upload on 192 MB photos.
	size_t scratch_bytes = ( size_t )tex_w * ( size_t )tex_h * 16;
	state.Scratch.resize( ( int )scratch_bytes );

	// Pack the source bytes tightly
	const unsigned char* src_base = ( const unsigned char* )buffer.host + buffer.byte_offset;
	unsigned char*       dst      = state.Scratch.Data;

	bool tight = ( buffer.x_stride_bytes == ( ptrdiff_t )( buffer.channels * sample_size ) )
	          && ( buffer.y_stride_bytes == ( ptrdiff_t )( buffer.width * buffer.channels * sample_size ) )
	          && ( buffer.c_stride_bytes == ( ptrdiff_t )sample_size );
	if ( tight )
	{
		// Hot path for raw photography — single memcpy of the entire buffer
		memcpy( dst, src_base, total_bytes );
	}
	else
	{
		// Slow path: walk the source strides and pack tightly
		size_t dst_x_stride = sample_size * buffer.channels;
		size_t dst_y_stride = dst_x_stride * buffer.width;
		for ( unsigned int y = 0; y < buffer.height; y++ )
		{
			for ( unsigned int x = 0; x < buffer.width; x++ )
			{
				unsigned char* dpix = dst + ( size_t )y * dst_y_stride + ( size_t )x * dst_x_stride;
				for ( unsigned int c = 0; c < buffer.channels; c++ )
				{
					const unsigned char* sp = src_base
					                        + ( ptrdiff_t )y * buffer.y_stride_bytes
					                        + ( ptrdiff_t )x * buffer.x_stride_bytes
					                        + ( ptrdiff_t )c * buffer.c_stride_bytes;
					memcpy( dpix + ( size_t )c * sample_size, sp, sample_size );
				}
			}
		}
	}

	// Zero only the tail padding past the packed source bytes, so the final
	// (possibly partial) RGBA32F row doesn't feed garbage to the GPU.
	if ( scratch_bytes > total_bytes )
		memset( dst + total_bytes, 0, scratch_bytes - total_bytes );

	// Prefer in-place upload when dimensions match — for large RGBA32F
	// inspectors (e.g. 4096×7716 / ~500 MB) destroy+create blocks the UI
	// thread for hundreds of ms on every buffer.version bump.
	bool dims_match = ( state.PackedTexture != ImTextureID_Invalid )
	               && ( state.PackedTexW == tex_w )
	               && ( state.PackedTexH == tex_h );

	bool updated = false;
	if ( dims_match )
	{
		updated = ImPlatform_UpdateTexture(
			state.PackedTexture,
			state.Scratch.Data,
			0, 0,
			( unsigned int )tex_w,
			( unsigned int )tex_h );
	}

	if ( !updated )
	{
		if ( state.PackedTexture != ImTextureID_Invalid )
		{
			ImPlatform_DestroyTexture( state.PackedTexture );
			state.PackedTexture = ImTextureID_Invalid;
		}

		ImPlatform_TextureDesc desc = ImPlatform_TextureDesc_Default( ( unsigned int )tex_w, ( unsigned int )tex_h );
		desc.format     = ImPlatform_PixelFormat_RGBA32F;
		desc.min_filter = ImPlatform_TextureFilter_Nearest;
		desc.mag_filter = ImPlatform_TextureFilter_Nearest;
		desc.wrap_u     = ImPlatform_TextureWrap_Clamp;
		desc.wrap_v     = ImPlatform_TextureWrap_Clamp;
		state.PackedTexture = ImPlatform_CreateTexture( state.Scratch.Data, &desc );
	}

	state.PackedTexW            = tex_w;
	state.PackedTexH            = tex_h;
	state.LastUploadedVersion   = buffer.version;
	state.LastUploadedWidth     = ( int )buffer.width;
	state.LastUploadedHeight    = ( int )buffer.height;
	state.LastUploadedChannels  = ( int )buffer.channels;
	state.LastUploadedSampleType = ( int )buffer.type;
	state.GpuXStride            = ( int )( buffer.channels * sample_size );
	state.GpuYStride            = ( int )( buffer.width * buffer.channels * sample_size );
	state.GpuCStride            = ( int )sample_size;
}

// ----------------------------------------------------------------------------
// CPU pixel readback — exact precision, all 11 sample types
// ----------------------------------------------------------------------------
// Outputs into both a `double[4]` lane (for display formatting) and an
// `int64[4]` lane (for exact integer values where the double would lose bits).
// Unused channels are zeroed.
static void ReadImageBufferSample( const ImImageBuffer& buf, int x, int y,
                                   double out_d[ 4 ], ImS64 out_i[ 4 ] )
{
	out_d[ 0 ] = out_d[ 1 ] = out_d[ 2 ] = out_d[ 3 ] = 0.0;
	out_i[ 0 ] = out_i[ 1 ] = out_i[ 2 ] = out_i[ 3 ] = 0;
	if ( buf.host == NULL ) return;
	if ( x < 0 || y < 0 || x >= ( int )buf.width || y >= ( int )buf.height ) return;

	const unsigned char* base = ( const unsigned char* )buf.host + buf.byte_offset;

	int chans = ( int )buf.channels;
	if ( chans > 4 ) chans = 4;
	for ( int c = 0; c < chans; c++ )
	{
		const unsigned char* p = base
		                       + ( ptrdiff_t )y * buf.y_stride_bytes
		                       + ( ptrdiff_t )x * buf.x_stride_bytes
		                       + ( ptrdiff_t )c * buf.c_stride_bytes;
		switch ( buf.type )
		{
		case ImSampleType_U8:  { ImU8 v;  memcpy( &v, p, 1 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 255.0; break; }
		case ImSampleType_I8:  { signed char v; memcpy( &v, p, 1 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 127.0; break; }
		case ImSampleType_U16: { ImU16 v; memcpy( &v, p, 2 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 65535.0; break; }
		case ImSampleType_I16: { ImS16 v; memcpy( &v, p, 2 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 32767.0; break; }
		case ImSampleType_U32: { ImU32 v; memcpy( &v, p, 4 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 4294967295.0; break; }
		case ImSampleType_I32: { ImS32 v; memcpy( &v, p, 4 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v / 2147483647.0; break; }
		case ImSampleType_U64: { ImU64 v; memcpy( &v, p, 8 ); out_i[ c ] = ( ImS64 )v; out_d[ c ] = ( double )v; break; }
		case ImSampleType_I64: { ImS64 v; memcpy( &v, p, 8 ); out_i[ c ] = v;          out_d[ c ] = ( double )v; break; }
		case ImSampleType_F16: { unsigned short v; memcpy( &v, p, 2 ); out_d[ c ] = ( double )II_HalfToFloat( v ); out_i[ c ] = ( ImS64 )out_d[ c ]; break; }
		case ImSampleType_F32: { float v;  memcpy( &v, p, 4 ); out_d[ c ] = ( double )v; out_i[ c ] = ( ImS64 )v; break; }
		case ImSampleType_F64: { double v; memcpy( &v, p, 8 ); out_d[ c ] = v;            out_i[ c ] = ( ImS64 )v; break; }
		default: break;
		}
	}
}

// ----------------------------------------------------------------------------
// Per-format display formatting for the inspector loupe
// ----------------------------------------------------------------------------
static void II_FormatChannel( char* buf, size_t bufsz, ImSampleType type, double d, ImS64 i )
{
	switch ( type )
	{
	case ImSampleType_U8:  ImFormatString( buf, bufsz, "%u",       ( unsigned int )i ); break;
	case ImSampleType_I8:  ImFormatString( buf, bufsz, "%d",       ( int )i ); break;
	case ImSampleType_U16: ImFormatString( buf, bufsz, "%u",       ( unsigned int )i ); break;
	case ImSampleType_I16: ImFormatString( buf, bufsz, "%d",       ( int )i ); break;
	case ImSampleType_U32: ImFormatString( buf, bufsz, "%u",       ( unsigned int )i ); break;
	case ImSampleType_I32: ImFormatString( buf, bufsz, "%d",       ( int )i ); break;
	case ImSampleType_U64: ImFormatString( buf, bufsz, "%llu",     ( unsigned long long )i ); break;
	case ImSampleType_I64: ImFormatString( buf, bufsz, "%lld",     ( long long )i ); break;
	case ImSampleType_F16: ImFormatString( buf, bufsz, "%.4f",     d ); break;
	case ImSampleType_F32: ImFormatString( buf, bufsz, "%.6f",     d ); break;
	case ImSampleType_F64: ImFormatString( buf, bufsz, "%.12f",    d ); break;
	default:               ImFormatString( buf, bufsz, "?" ); break;
	}
}

static const char* II_ChannelLabel( int channels, int idx )
{
	static const char* one[]  = { "Y" };
	static const char* two[]  = { "R", "G" };
	static const char* three[]= { "R", "G", "B" };
	static const char* four[] = { "R", "G", "B", "A" };
	if ( channels == 1 ) return one  [ idx % 1 ];
	if ( channels == 2 ) return two  [ idx % 2 ];
	if ( channels == 3 ) return three[ idx % 3 ];
	return four[ idx % 4 ];
}

// ----------------------------------------------------------------------------
// Capability probe + state release
// ----------------------------------------------------------------------------
bool ImageInspectorSupported()
{
#if ( IM_CURRENT_GFX == IM_GFX_DIRECTX9 )
	return false;
#else
	return true;
#endif
}

void ImageInspectorReleaseState( ImImageInspectorState& state )
{
	if ( state.PackedTexture != ImTextureID_Invalid )
	{
		ImPlatform_DestroyTexture( state.PackedTexture );
		state.PackedTexture = ImTextureID_Invalid;
	}
	state.Scratch.clear();
	state.LastUploadedVersion   = 0;
	state.LastUploadedWidth     = 0;
	state.LastUploadedHeight    = 0;
	state.LastUploadedChannels  = 0;
	state.LastUploadedSampleType = 0;
}

// ----------------------------------------------------------------------------
// Fill the cbuffer for one draw (used by both main image and loupe quads)
// ----------------------------------------------------------------------------
static void II_FillParams( ImageInspectorParams& p,
                           const ImImageBuffer& buffer,
                           const ImImageInspectorState& state,
                           ImVec2 widgetPx,
                           ImVec2 pan,
                           float zoom,
                           float fitScale )
{
	p.imgSize[ 0 ] = ( float )buffer.width;
	p.imgSize[ 1 ] = ( float )buffer.height;
	p.imgSize[ 2 ] = ( buffer.width  > 0 ) ? 1.0f / ( float )buffer.width  : 0.0f;
	p.imgSize[ 3 ] = ( buffer.height > 0 ) ? 1.0f / ( float )buffer.height : 0.0f;

	p.panZoom[ 0 ] = pan.x;
	p.panZoom[ 1 ] = pan.y;
	p.panZoom[ 2 ] = zoom;
	p.panZoom[ 3 ] = fitScale;

	p.viewportPx[ 0 ] = widgetPx.x;
	p.viewportPx[ 1 ] = widgetPx.y;
	p.viewportPx[ 2 ] = ( widgetPx.x > 0.0f ) ? 1.0f / widgetPx.x : 0.0f;
	p.viewportPx[ 3 ] = ( widgetPx.y > 0.0f ) ? 1.0f / widgetPx.y : 0.0f;

	p.packedTexDims[ 0 ] = ( unsigned )state.PackedTexW;
	p.packedTexDims[ 1 ] = ( unsigned )state.PackedTexH;
	p.packedTexDims[ 2 ] = ( unsigned )( ImPlatform_ImageBufferTightByteSize( &buffer ) & 0xFFFFFFFFu );
	p.packedTexDims[ 3 ] = 12;    // log2(4096) — kPackedTexWidth

	p.layoutPack[ 0 ] = ( unsigned )state.GpuXStride;
	p.layoutPack[ 1 ] = ( unsigned )state.GpuYStride;
	p.layoutPack[ 2 ] = ( unsigned )state.GpuCStride;
	p.layoutPack[ 3 ] = 0;

	p.formatPack[ 0 ] = ( unsigned )buffer.type;
	p.formatPack[ 1 ] = ( unsigned )buffer.channels;
	p.formatPack[ 2 ] = ( unsigned )state.MosaicPattern;
	p.formatPack[ 3 ] = ( unsigned )state.Filter;

	p.exposureParams[ 0 ] = state.Exposure;
	p.exposureParams[ 1 ] = state.Black;
	p.exposureParams[ 2 ] = state.White;
	p.exposureParams[ 3 ] = state.Gamma;

	p.tempTint[ 0 ] = state.Temperature;
	p.tempTint[ 1 ] = state.Tint;
	p.tempTint[ 2 ] = 0.0f;
	p.tempTint[ 3 ] = 0.0f;

	float in_to_work [ 9 ];
	float work_to_out[ 9 ];
	II_BuildGamutTransform( state.InputGamut,   state.WorkingGamut, in_to_work  );
	II_BuildGamutTransform( state.WorkingGamut, state.OutputGamut,  work_to_out );

	p.inGamut_r0[ 0 ] = in_to_work[ 0 ]; p.inGamut_r0[ 1 ] = in_to_work[ 1 ]; p.inGamut_r0[ 2 ] = in_to_work[ 2 ]; p.inGamut_r0[ 3 ] = 0.0f;
	p.inGamut_r1[ 0 ] = in_to_work[ 3 ]; p.inGamut_r1[ 1 ] = in_to_work[ 4 ]; p.inGamut_r1[ 2 ] = in_to_work[ 5 ]; p.inGamut_r1[ 3 ] = 0.0f;
	p.inGamut_r2[ 0 ] = in_to_work[ 6 ]; p.inGamut_r2[ 1 ] = in_to_work[ 7 ]; p.inGamut_r2[ 2 ] = in_to_work[ 8 ]; p.inGamut_r2[ 3 ] = 0.0f;

	p.outGamut_r0[ 0 ] = work_to_out[ 0 ]; p.outGamut_r0[ 1 ] = work_to_out[ 1 ]; p.outGamut_r0[ 2 ] = work_to_out[ 2 ]; p.outGamut_r0[ 3 ] = 0.0f;
	p.outGamut_r1[ 0 ] = work_to_out[ 3 ]; p.outGamut_r1[ 1 ] = work_to_out[ 4 ]; p.outGamut_r1[ 2 ] = work_to_out[ 5 ]; p.outGamut_r1[ 3 ] = 0.0f;
	p.outGamut_r2[ 0 ] = work_to_out[ 6 ]; p.outGamut_r2[ 1 ] = work_to_out[ 7 ]; p.outGamut_r2[ 2 ] = work_to_out[ 8 ]; p.outGamut_r2[ 3 ] = 0.0f;

	p.pipelinePack[ 0 ] = ( unsigned )state.InputTransfer;
	p.pipelinePack[ 1 ] = ( unsigned )state.OutputTransfer;
	p.pipelinePack[ 2 ] = ( unsigned )state.Tonemap;
	p.pipelinePack[ 3 ] = ( unsigned )state.FalseColor;

	p.channelMask[ 0 ] = state.ChannelMask[ 0 ];
	p.channelMask[ 1 ] = state.ChannelMask[ 1 ];
	p.channelMask[ 2 ] = state.ChannelMask[ 2 ];
	p.channelMask[ 3 ] = state.ChannelMask[ 3 ];

	p.nanColor[ 0 ] = state.NaNColor.x;
	p.nanColor[ 1 ] = state.NaNColor.y;
	p.nanColor[ 2 ] = state.NaNColor.z;
	p.nanColor[ 3 ] = state.NaNColor.w;

	p.modePack[ 0 ] = ( unsigned )state.MosaicMode;
	p.modePack[ 1 ] = 0;
	p.modePack[ 2 ] = 0;
	p.modePack[ 3 ] = 0;
}

// ----------------------------------------------------------------------------
// Issue an inspector quad with custom shader
// ----------------------------------------------------------------------------
static void II_DrawShaderQuad( ImDrawList* dl,
                               const ImImageBuffer& buffer,
                               ImImageInspectorState& state,
                               ImVec2 quadMin, ImVec2 quadMax,
                               ImVec2 pan, float zoom, float fitScale )
{
	ImWidgetsContext* ctx = GetCurrentContext();
	if ( !ctx || !ctx->imageInspectorShader.program )
		return;
	if ( state.PackedTexture == ImTextureID_Invalid )
		return;

	ImageInspectorDrawCallData* call = IM_NEW( ImageInspectorDrawCallData );
	call->program       = ctx->imageInspectorShader.program;
	call->packedTexture = state.PackedTexture;
	II_FillParams( call->params, buffer, state,
	               ImVec2( quadMax.x - quadMin.x, quadMax.y - quadMin.y ),
	               pan, zoom, fitScale );
	s_iiDrawCalls[ s_iiPoolIdx ].push_back( call );

	dl->AddCallback( &ImageInspectorShaderCallback, call );
	dl->AddImage( state.PackedTexture, quadMin, quadMax, ImVec2( 0, 0 ), ImVec2( 1, 1 ) );
	dl->AddCallback( ImDrawCallback_ResetRenderState, NULL );
}

#endif  // IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER

// ============================================================================
// ImageInspector — public widget function
// ============================================================================
bool ImageInspector( char const* label, const ImImageBuffer& buffer, ImImageInspectorState& state, ImVec2 widgetSize )
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if ( window->SkipItems )
		return false;

	ImGuiContext&     g     = *GImGui;
	const ImGuiID     id    = window->GetID( label );

	// --- Widget rect ---
	ImVec2 sz = widgetSize;
	if ( sz.x <= 0.0f ) sz.x = ImGui::GetContentRegionAvail().x;
	if ( sz.y <= 0.0f )
	{
		float aspect = ( buffer.width > 0 && buffer.height > 0 )
		             ? ( float )buffer.height / ( float )buffer.width : 1.0f;
		sz.y = sz.x * aspect;
		sz.y = ImMin( sz.y, sz.x );
	}

	ImRect bb( window->DC.CursorPos, window->DC.CursorPos + sz );
	ImGui::ItemSize( bb );
	if ( !ImGui::ItemAdd( bb, id ) )
		return false;

	const bool hovered = ImGui::IsItemHovered();
	bool       changed = false;

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
	// --- DX9 / unsupported guard ---
	if ( !ImageInspectorSupported() )
	{
		ImDrawList* dl = window->DrawList;
		dl->AddRectFilled( bb.Min, bb.Max, IM_COL32( 30, 30, 30, 255 ) );
		dl->AddRect      ( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border ) );
		const char* msg = "ImageInspector: not supported on this backend";
		ImVec2 ts = ImGui::CalcTextSize( msg );
		dl->AddText( ImVec2( bb.Min.x + ( sz.x - ts.x ) * 0.5f, bb.Min.y + ( sz.y - ts.y ) * 0.5f ),
		             IM_COL32( 200, 100, 100, 255 ), msg );
		return false;
	}

	// --- Lazy shader init ---
	ImWidgetsContext* ctx = GetCurrentContext();
	if ( ctx && ctx->imageInspectorShader.program == NULL )
	{
		CreateInternalShader( &ctx->imageInspectorShader, "image_inspector", 0, NULL, 0, NULL );
		if ( ctx->imageInspectorShader.program == NULL )
		{
			ImDrawList* dl = window->DrawList;
			dl->AddRectFilled( bb.Min, bb.Max, IM_COL32( 30, 30, 30, 255 ) );
			dl->AddRect      ( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border ) );
			const char* msg = "ImageInspector: shader load failed";
			ImVec2 ts = ImGui::CalcTextSize( msg );
			dl->AddText( ImVec2( bb.Min.x + ( sz.x - ts.x ) * 0.5f, bb.Min.y + ( sz.y - ts.y ) * 0.5f ),
			             IM_COL32( 200, 100, 100, 255 ), msg );
			return false;
		}
	}

	// --- Per-frame pool cleanup ---
	ImageInspectorFrameCleanup();

	// --- Upload (re-uploads only on version change) ---
	EnsurePackedTexture( buffer, state );
	if ( state.BufferTooLarge || state.PackedTexture == ImTextureID_Invalid )
	{
		ImDrawList* dl = window->DrawList;
		dl->AddRectFilled( bb.Min, bb.Max, IM_COL32( 30, 30, 30, 255 ) );
		dl->AddRect      ( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border ) );
		const char* msg = "ImageInspector: image too large or invalid";
		ImVec2 ts = ImGui::CalcTextSize( msg );
		dl->AddText( ImVec2( bb.Min.x + ( sz.x - ts.x ) * 0.5f, bb.Min.y + ( sz.y - ts.y ) * 0.5f ),
		             IM_COL32( 200, 100, 100, 255 ), msg );
		return false;
	}

	// --- Pan/zoom math (mirrors ImageViewer) ---
	float fitScale = 1.0f;
	if ( buffer.width > 0 && buffer.height > 0 )
		fitScale = ImMin( sz.x / ( float )buffer.width, sz.y / ( float )buffer.height );
	float totalScale = fitScale * state.Zoom;
	ImVec2 rectCenter = ( bb.Min + bb.Max ) * 0.5f;
	ImVec2 imgCenter  = ImVec2( ( float )buffer.width  * 0.5f + state.Pan.x,
	                            ( float )buffer.height * 0.5f + state.Pan.y );

	const ImVec2 mp = g.IO.MousePos;

	auto screenToImg = [ & ]( ImVec2 s ) -> ImVec2 {
		return ImVec2(
			imgCenter.x + ( s.x - rectCenter.x ) / totalScale,
			imgCenter.y + ( s.y - rectCenter.y ) / totalScale );
	};

	// Claim mouse wheel
	if ( hovered )
		ImGui::SetKeyOwner( ImGuiKey_MouseWheelY, id );

	// Scroll-wheel zoom
	if ( hovered && g.IO.MouseWheel != 0.0f )
	{
		ImVec2 mouseImg = screenToImg( mp );
		float  newZoom  = ImClamp( state.Zoom * powf( 1.15f, g.IO.MouseWheel ), 0.05f, 64.0f );
		float  newTS    = fitScale * newZoom;
		state.Pan.x = mouseImg.x - ( float )buffer.width  * 0.5f - ( mp.x - rectCenter.x ) / newTS;
		state.Pan.y = mouseImg.y - ( float )buffer.height * 0.5f - ( mp.y - rectCenter.y ) / newTS;
		state.Zoom  = newZoom;
		totalScale  = newTS;
		imgCenter   = ImVec2( ( float )buffer.width * 0.5f + state.Pan.x,
		                      ( float )buffer.height * 0.5f + state.Pan.y );
		changed = true;
	}

	// Left-click drag pan
	if ( hovered && g.IO.MouseClicked[ 0 ] && !g.IO.MouseDoubleClicked[ 0 ] )
	{
		ImGui::SetActiveID( id, window );
		ImGui::SetFocusID( id, window );
		ImGui::FocusWindow( window );
	}
	if ( g.ActiveId == id )
	{
		if ( ImGui::IsMouseDown( 0 ) )
		{
			ImGui::SetKeyOwner( ImGuiKey_MouseLeft, id );
			state.Pan.x -= g.IO.MouseDelta.x / totalScale;
			state.Pan.y -= g.IO.MouseDelta.y / totalScale;
			imgCenter = ImVec2( ( float )buffer.width * 0.5f + state.Pan.x,
			                    ( float )buffer.height * 0.5f + state.Pan.y );
			ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeAll );
			changed = true;
		}
		else
		{
			ImGui::ClearActiveID();
		}
	}

	// Double-click reset
	if ( hovered && g.IO.MouseDoubleClicked[ 0 ] )
	{
		state.Zoom = 1.0f;
		state.Pan  = ImVec2( 0.0f, 0.0f );
		totalScale = fitScale;
		imgCenter  = ImVec2( ( float )buffer.width * 0.5f, ( float )buffer.height * 0.5f );
		changed = true;
	}

	// --- Render ---
	ImDrawList* dl = window->DrawList;
	dl->PushClipRect( bb.Min, bb.Max, true );

	// Checkerboard background
	{
		const float csz = 8.0f;
		const ImU32 ca  = IM_COL32( 50, 50, 50, 255 );
		const ImU32 cb  = IM_COL32( 75, 75, 75, 255 );
		int nx = ( int )ceilf( sz.x / csz );
		int ny = ( int )ceilf( sz.y / csz );
		for ( int cy = 0; cy < ny; cy++ )
			for ( int cx = 0; cx < nx; cx++ )
			{
				ImU32  col = ( ( cx + cy ) & 1 ) ? cb : ca;
				ImVec2 tl  = ImVec2( bb.Min.x + cx * csz, bb.Min.y + cy * csz );
				ImVec2 br  = ImVec2( ImMin( tl.x + csz, bb.Max.x ), ImMin( tl.y + csz, bb.Max.y ) );
				dl->AddRectFilled( tl, br, col );
			}
	}

	// Image quad through the custom shader
	II_DrawShaderQuad( dl, buffer, state, bb.Min, bb.Max, state.Pan, state.Zoom, fitScale );

	// Pixel grid overlay (when zoomed in enough)
	if ( state.Zoom * fitScale > 8.0f )
	{
		float pixSz  = totalScale;
		float startX = bb.Min.x + fmodf( rectCenter.x - bb.Min.x - imgCenter.x * totalScale, pixSz );
		float startY = bb.Min.y + fmodf( rectCenter.y - bb.Min.y - imgCenter.y * totalScale, pixSz );
		for ( float x = startX; x < bb.Max.x; x += pixSz )
			dl->AddLine( ImVec2( x, bb.Min.y ), ImVec2( x, bb.Max.y ), IM_COL32( 0, 0, 0, 60 ) );
		for ( float y = startY; y < bb.Max.y; y += pixSz )
			dl->AddLine( ImVec2( bb.Min.x, y ), ImVec2( bb.Max.x, y ), IM_COL32( 0, 0, 0, 60 ) );
	}

	dl->PopClipRect();
	dl->AddRect( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border ) );

	// Zoom % label
	{
		char zoomBuf[ 16 ];
		ImFormatString( zoomBuf, sizeof( zoomBuf ), "%.0f%%", state.Zoom * 100.0f );
		ImVec2 ts = ImGui::CalcTextSize( zoomBuf );
		ImVec2 tp = ImVec2( bb.Max.x - ts.x - 5.0f, bb.Max.y - ts.y - 3.0f );
		dl->AddRectFilled( ImVec2( tp.x - 2, tp.y - 1 ), ImVec2( tp.x + ts.x + 2, tp.y + ts.y + 1 ), IM_COL32( 0, 0, 0, 140 ) );
		dl->AddText( tp, IM_COL32( 200, 200, 200, 255 ), zoomBuf );
	}

	// Hint
	if ( hovered && ( fabsf( state.Zoom - 1.0f ) > 0.01f || state.Pan.x != 0.0f || state.Pan.y != 0.0f ) )
	{
		const char* hint = "dbl-click to reset";
		ImVec2 hs = ImGui::CalcTextSize( hint );
		ImVec2 hp = ImVec2( bb.Min.x + 4.0f, bb.Max.y - hs.y - 3.0f );
		dl->AddRectFilled( ImVec2( hp.x - 2, hp.y - 1 ), ImVec2( hp.x + hs.x + 2, hp.y + hs.y + 1 ), IM_COL32( 0, 0, 0, 140 ) );
		dl->AddText( hp, IM_COL32( 180, 180, 180, 200 ), hint );
	}

	// --- Right-click pixel inspector loupe ---
	if ( hovered && ImGui::IsMouseDown( 1 ) )
	{
		ImVec2 mouseImg = screenToImg( mp );
		int    px       = ( int )floorf( mouseImg.x );
		int    py       = ( int )floorf( mouseImg.y );

		// CPU readback for exact channel values
		double dvals[ 4 ] = { 0, 0, 0, 0 };
		ImS64  ivals[ 4 ] = { 0, 0, 0, 0 };
		bool inRange = ( px >= 0 ) && ( px < ( int )buffer.width )
		            && ( py >= 0 ) && ( py < ( int )buffer.height );
		if ( inRange )
			ReadImageBufferSample( buffer, px, py, dvals, ivals );

		int chans = ( int )buffer.channels;
		if ( chans > 4 ) chans = 4;

		char chBufs[ 4 ][ 32 ];
		for ( int c = 0; c < 4; c++ )
		{
			if ( c < chans && inRange )
				II_FormatChannel( chBufs[ c ], sizeof( chBufs[ c ] ), buffer.type, dvals[ c ], ivals[ c ] );
			else
				ImFormatString( chBufs[ c ], sizeof( chBufs[ c ] ), "--" );
		}

		float u = ( buffer.width  > 0 ) ? mouseImg.x / ( float )buffer.width  : 0.0f;
		float v = ( buffer.height > 0 ) ? mouseImg.y / ( float )buffer.height : 0.0f;

		char posLine[ 64 ], uvLine[ 64 ];
		ImFormatString( posLine, sizeof( posLine ), "x: %-5d  y: %d",      px, py );
		ImFormatString( uvLine,  sizeof( uvLine  ), "u: %-7.4f  v: %.4f", u, v );

		// Build channel display lines (2 channels per line)
		char chLine0[ 96 ], chLine1[ 96 ];
		const char* l0 = II_ChannelLabel( chans, 0 );
		const char* l1 = ( chans >= 2 ) ? II_ChannelLabel( chans, 1 ) : "-";
		const char* l2 = ( chans >= 3 ) ? II_ChannelLabel( chans, 2 ) : "-";
		const char* l3 = ( chans >= 4 ) ? II_ChannelLabel( chans, 3 ) : "-";
		ImFormatString( chLine0, sizeof( chLine0 ), "%s: %-12s  %s: %s", l0, chBufs[ 0 ], l1, ( chans >= 2 ? chBufs[ 1 ] : "--" ) );
		ImFormatString( chLine1, sizeof( chLine1 ), "%s: %-12s  %s: %s", l2, ( chans >= 3 ? chBufs[ 2 ] : "--" ),
		                                                                  l3, ( chans >= 4 ? chBufs[ 3 ] : "--" ) );

		// --- Layout: [loupe] [text] [swatch] ---
		const float pad      = 7.0f;
		const float colGap   = 8.0f;
		const float lineH    = ImGui::GetTextLineHeight();
		const float lineGap  = 3.0f;
		const float sepGap   = 5.0f;
		const float rounding = 4.0f;

		float textH  = 4.0f * lineH + 3.0f * lineGap + sepGap;
		float innerH = ImMax( textH, lineH * 5.0f );
		float squareSz = innerH;

		float textW = ImGui::CalcTextSize( posLine ).x;
		textW = ImMax( textW, ImGui::CalcTextSize( uvLine  ).x );
		textW = ImMax( textW, ImGui::CalcTextSize( chLine0 ).x );
		textW = ImMax( textW, ImGui::CalcTextSize( chLine1 ).x );

		float panelW = pad + squareSz + colGap + textW + colGap + squareSz + pad;
		float panelH = pad + innerH + pad;

		float px0 = mp.x + 14.0f;
		float py0 = mp.y - panelH - 6.0f;
		if ( px0 + panelW > g.IO.DisplaySize.x - 2.0f ) px0 = mp.x - 14.0f - panelW;
		if ( py0 < 2.0f )                                py0 = mp.y + 10.0f;
		if ( py0 + panelH > g.IO.DisplaySize.y - 2.0f ) py0 = g.IO.DisplaySize.y - panelH - 2.0f;

		ImVec2 panelMin = ImVec2( px0, py0 );
		ImVec2 panelMax = ImVec2( px0 + panelW, py0 + panelH );

		ImDrawList* fg = ImGui::GetForegroundDrawList();

		fg->AddRectFilled( panelMin, panelMax, IM_COL32( 18, 18, 18, 235 ), rounding );
		fg->AddRect( panelMin, panelMax, IM_COL32( 90, 90, 90, 220 ), rounding, 0, 1.0f );

		// --- Loupe (left square): show a small region of the image through the same shader ---
		ImVec2 lTL  = ImVec2( panelMin.x + pad, panelMin.y + pad );
		ImVec2 lBR  = ImVec2( lTL.x + squareSz, lTL.y + squareSz );

		// Loupe checkerboard
		fg->PushClipRect( lTL, lBR, true );
		{
			const float csz = 5.0f;
			int nx = ( int )ceilf( squareSz / csz );
			int ny = ( int )ceilf( squareSz / csz );
			for ( int cy2 = 0; cy2 < ny; cy2++ )
				for ( int cx2 = 0; cx2 < nx; cx2++ )
				{
					ImU32  c2  = ( ( cx2 + cy2 ) & 1 ) ? IM_COL32( 70, 70, 70, 255 ) : IM_COL32( 45, 45, 45, 255 );
					ImVec2 tl2 = ImVec2( lTL.x + cx2 * csz, lTL.y + cy2 * csz );
					ImVec2 br2 = ImVec2( ImMin( tl2.x + csz, lBR.x ), ImMin( tl2.y + csz, lBR.y ) );
					fg->AddRectFilled( tl2, br2, c2 );
				}
		}
		fg->PopClipRect();

		// Loupe through custom shader: pan/zoom such that an 11×11 source region fits the loupe rect
		const float loupePixels = 11.0f;
		float loupeFitScale = squareSz / loupePixels;    // src px -> screen px
		float loupeZoom     = 1.0f;
		ImVec2 loupePan = ImVec2( mouseImg.x - ( float )buffer.width  * 0.5f,
		                          mouseImg.y - ( float )buffer.height * 0.5f );
		II_DrawShaderQuad( fg, buffer, state, lTL, lBR, loupePan, loupeZoom, loupeFitScale );

		// Pixel grid + crosshair + highlight
		fg->PushClipRect( lTL, lBR, true );
		float pixSz = squareSz / loupePixels;
		ImVec2 lCtr = ( lTL + lBR ) * 0.5f;
		for ( int i = 0; i <= ( int )loupePixels; i++ )
		{
			float x2 = lTL.x + i * pixSz;
			float y2 = lTL.y + i * pixSz;
			fg->AddLine( ImVec2( x2, lTL.y ), ImVec2( x2, lBR.y ), IM_COL32( 0, 0, 0, 70 ) );
			fg->AddLine( ImVec2( lTL.x, y2 ), ImVec2( lBR.x, y2 ), IM_COL32( 0, 0, 0, 70 ) );
		}
		{
			float fracX  = mouseImg.x - floorf( mouseImg.x );
			float fracY  = mouseImg.y - floorf( mouseImg.y );
			float pxLeft = lCtr.x - fracX * pixSz;
			float pxTop  = lCtr.y - fracY * pixSz;
			ImVec2 ctr2  = ImVec2( pxLeft + pixSz * 0.5f, pxTop + pixSz * 0.5f );
			fg->AddLine( ImVec2( lTL.x, ctr2.y ), ImVec2( lBR.x, ctr2.y ), IM_COL32( 255, 60, 60, 100 ) );
			fg->AddLine( ImVec2( ctr2.x, lTL.y ), ImVec2( ctr2.x, lBR.y ), IM_COL32( 255, 60, 60, 100 ) );
			fg->AddRect( ImVec2( pxLeft, pxTop ), ImVec2( pxLeft + pixSz, pxTop + pixSz ),
			             IM_COL32( 255, 60, 60, 255 ), 0.0f, 0, 1.5f );
		}
		fg->PopClipRect();
		fg->AddRect( lTL, lBR, IM_COL32( 110, 110, 110, 200 ), 0.0f, 0, 1.0f );

		// --- Text column (4 rows: pos, uv, channels0, channels1) ---
		const ImU32 colValue = IM_COL32( 230, 230, 230, 255 );
		const ImU32 colLabel = IM_COL32( 130, 130, 130, 255 );
		const ImU32 colDim   = IM_COL32(  90,  90,  90, 255 );

		float tx = lBR.x + colGap;
		float ty = panelMin.y + pad + ( innerH - textH ) * 0.5f;

		fg->AddText( ImVec2( tx, ty ), colValue, posLine ); ty += lineH + lineGap;
		fg->AddText( ImVec2( tx, ty ), colLabel, uvLine  ); ty += lineH + lineGap;
		ty += sepGap;
		fg->AddText( ImVec2( tx, ty ), inRange ? colValue : colDim, chLine0 ); ty += lineH + lineGap;
		fg->AddText( ImVec2( tx, ty ), inRange ? colValue : colDim, chLine1 );

		// --- Color swatch (right square) — approximate via a 3-channel display ---
		ImVec2 sTL = ImVec2( panelMax.x - pad - squareSz, panelMin.y + pad );
		ImVec2 sBR = ImVec2( panelMax.x - pad,            panelMin.y + pad + squareSz );

		ImVec4 swColor( 0.0f, 0.0f, 0.0f, 1.0f );
		if ( inRange )
		{
			if ( chans == 1 ) { swColor.x = swColor.y = swColor.z = ( float )dvals[ 0 ]; }
			else if ( chans == 2 ) { swColor.x = ( float )dvals[ 0 ]; swColor.y = ( float )dvals[ 1 ]; swColor.z = 0.0f; }
			else { swColor.x = ( float )dvals[ 0 ]; swColor.y = ( float )dvals[ 1 ]; swColor.z = ( float )dvals[ 2 ]; }
		}
		ImU32 swCol = inRange
		            ? ImGui::ColorConvertFloat4ToU32( ImVec4( swColor.x, swColor.y, swColor.z, 1.0f ) )
		            : IM_COL32( 45, 45, 45, 255 );
		fg->AddRectFilled( sTL, sBR, swCol, 2.0f );
		if ( !inRange )
		{
			fg->PushClipRect( sTL, sBR, true );
			for ( float d = 0.0f; d < squareSz * 2.0f; d += 8.0f )
				fg->AddLine( ImVec2( sTL.x + d, sTL.y ), ImVec2( sTL.x, sTL.y + d ), IM_COL32( 70, 70, 70, 255 ) );
			fg->PopClipRect();
		}
		fg->AddRect( sTL, sBR, IM_COL32( 90, 90, 90, 180 ), 2.0f, 0, 1.0f );
	}

	// --- Expand button ---
	bool* pExpanded = WidgetExpandButton( id, bb );
	if ( pExpanded && *pExpanded )
	{
		if ( BeginExpandedWindow( label, id, pExpanded, ImVec2( 900, 600 ) ) )
		{
			ImVec2 avail = ImGui::GetContentRegionAvail();
			float  widgetW = avail.x * 0.75f;
			if ( ImageInspector( "##exp", buffer, state, ImVec2( widgetW, avail.y ) ) )
				changed = true;
			ImGui::SameLine();
			ImGui::BeginChild( "##info", ImVec2( 0, avail.y ), ImGuiChildFlags_Borders );
			ImGui::TextUnformatted( "Image" );
			if ( buffer.width > 0 && buffer.height > 0 )
				ImGui::Text( "Size: %u x %u  Channels: %u", buffer.width, buffer.height, buffer.channels );
			ImGui::Text( "Sample type: %d", ( int )buffer.type );
			ImGui::Separator();
			ImGui::Text( "Zoom: %.1f%%", state.Zoom * 100.0f );
			ImGui::Text( "Pan: (%.1f, %.1f)", state.Pan.x, state.Pan.y );
			ImGui::Separator();
			ImGui::SliderFloat( "Exposure",   &state.Exposure, -10.0f, 10.0f, "%.2f stops" );
			ImGui::SliderFloat( "Black",      &state.Black,    -1.0f,  1.0f, "%.3f" );
			ImGui::SliderFloat( "White",      &state.White,     0.0f,  4.0f, "%.3f" );
			ImGui::SliderFloat( "Gamma",      &state.Gamma,     0.1f,  4.0f, "%.2f" );
			ImGui::SliderFloat( "Temperature",&state.Temperature, -1.0f, 1.0f );
			ImGui::SliderFloat( "Tint",       &state.Tint,        -1.0f, 1.0f );
			ImGui::EndChild();
			EndExpandedWindow();
		}
	}
#else
	// Custom shader unsupported at compile time — render placeholder
	ImDrawList* dl = window->DrawList;
	dl->AddRectFilled( bb.Min, bb.Max, IM_COL32( 30, 30, 30, 255 ) );
	dl->AddRect      ( bb.Min, bb.Max, ImGui::GetColorU32( ImGuiCol_Border ) );
	const char* msg = "ImageInspector: not supported on this backend";
	ImVec2 ts = ImGui::CalcTextSize( msg );
	dl->AddText( ImVec2( bb.Min.x + ( sz.x - ts.x ) * 0.5f, bb.Min.y + ( sz.y - ts.y ) * 0.5f ),
	             IM_COL32( 200, 100, 100, 255 ), msg );
	(void)buffer; (void)state;
#endif  // IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER

	return changed;
}

}  // namespace ImWidgets
