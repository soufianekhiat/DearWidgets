#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

#include <imgui_internal.h>

//#include <algorithm>
//#include <string>
//#include <cmath>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
// Style TODO:
//	* Expose Style
//	* Line Thickness for Slider2D
//	* Line Color for Slider2D
//
// Known issue:
//	* Slider2DInt must take into account the half pixel
//	* Move 2D must store state "IsDrag"
//
// Optim TODO:
//	* ChromaticPlot draw only the internal MultiColorQuad where its needed "inside" or at least leave the option for style "transparency etc"
//	* ChromaticPlot: Bake some as much as possible values: provide different version: ChromaticPlotDynamic {From enum and compute info at each frame}, ChromaticPlotFromData {From Baked data}
//
// Write use case for:
//	* HueToHue:
//		- Color Remap
//	* LumToSat:
//		- Color Remap
//	* ColorRing:
//		- HDR Color Management {Shadow, MidTone, Highlight}
//	* Grid2D_AoS_Float:
//		- Color Remap:
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Perf TODO:
//		Cache the triangulation for concave shape/poly
//		Find a way to cache shape with hole too
// Create "Shape" struct
//////////////////////////////////////////////////////////////////////////

#if !__cpp_if_constexpr
#define constexpr
#endif

#if 0
#define nullptr NULL
#endif

namespace ImWidgets{
	typedef void* ImShaderID;
	struct ImShader
	{
		ImShaderID vs;
		ImShaderID ps;
	};
	struct ImVertex
	{
		ImVec2 pos;
		ImVec2 uv;
		ImU32 col;
	};
	struct ImEdgeIdx
	{
		ImDrawIdx a, b;
		constexpr ImEdgeIdx() : a( ( ImDrawIdx )( -1 ) ), b( ( ImDrawIdx )( -1 ) )
		{}
		constexpr ImEdgeIdx( ImDrawIdx _a, ImDrawIdx _b ) : a( _a ), b( _b )
		{}
		ImEdgeIdx& operator[] ( size_t idx )
		{
			IM_ASSERT( idx == 0 || idx == 1 );
			return ( ( ImEdgeIdx* )( void* )( char* )this )[ idx ];
		}
		ImEdgeIdx operator[] ( size_t idx ) const
		{
			IM_ASSERT( idx == 0 || idx == 1 );
			return ( ( const ImEdgeIdx* )( const void* )( const char* )this )[ idx ];
		}
		bool operator== ( ImEdgeIdx e ) const
		{
			return a == e.a && b == e.a;
		}
	};
	struct ImTriIdx
	{
		ImDrawIdx a, b, c;
		constexpr ImTriIdx() : a( ( ImDrawIdx )( -1 ) ), b( ( ImDrawIdx )( -1 ) ), c( ( ImDrawIdx )( -1 ) )
		{}
		constexpr ImTriIdx( ImDrawIdx _a, ImDrawIdx _b, ImDrawIdx _c ) : a( _a ), b( _b ), c( _c )
		{}
		ImDrawIdx& operator[] ( size_t idx )
		{
			IM_ASSERT( idx == 0 || idx == 1 || idx == 2 );
			return ( ( ImDrawIdx* )( void* )( char* )this )[ idx ];
		}
		ImDrawIdx operator[] ( size_t idx ) const
		{
			IM_ASSERT( idx == 0 || idx == 1 || idx == 2 );
			return ( ( const ImDrawIdx* )( const void* )( const char* )this )[ idx ];
		}
		ImTriIdx& operator= ( ImTriIdx const& rhs )
		{
			a = rhs.a;
			b = rhs.b;
			c = rhs.c;

			return *this;
		}
	};
	struct ImShape
	{
		ImVector<ImVertex>	vertices;
		ImVector<ImTriIdx>	triangles;
		ImRect				bb;
	};

	typedef ImU32( *ImColor1DCallback )( float x, void* );
	typedef ImU32( *ImColor2DCallback )( float x, float y, void* );

	enum StyleColor
	{
		StyleColor_Value,

		StyleColor_Count
	};

	enum StyleVar
	{
		StyleVar_HueSelector_Thickness_ZeroWidth,

		StyleVar_Count
	};

	struct Style
	{
		float	HueSelector_Thickness_ZeroWidth;
		ImVec4  Colors[ StyleColor_Count ];

		Style()
		{
			HueSelector_Thickness_ZeroWidth = 2.0f;
			Colors[ StyleColor_Value ] = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
		}

		void PushColor( StyleColor colorIndex, const ImVec4& color )
		{
			ColorModifier modifier;
			modifier.Index = colorIndex;
			modifier.Value = Colors[ colorIndex ];
			m_ColorStack.push_back( modifier );
			Colors[ colorIndex ] = color;
		}
		void PopColor( int count = 1 )
		{
			while ( count > 0 )
			{
				auto& modifier = m_ColorStack.back();
				Colors[ modifier.Index ] = modifier.Value;
				m_ColorStack.pop_back();
				--count;
			}
		}

		void PushVar( StyleVar varIndex, float value )
		{
			auto* var = GetVarFloatAddr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = ImVec4( *var, 0, 0, 0 );
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PushVar( StyleVar varIndex, const ImVec2& value )
		{
			auto* var = GetVarVec2Addr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = ImVec4( var->x, var->y, 0, 0 );
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PushVar( StyleVar varIndex, const ImVec4& value )
		{
			auto* var = GetVarVec4Addr( varIndex );
			IM_ASSERT( var != nullptr );
			VarModifier modifier;
			modifier.Index = varIndex;
			modifier.Value = *var;
			*var = value;
			m_VarStack.push_back( modifier );
		}
		void PopVar( int count = 1 )
		{
			while ( count > 0 )
			{
				auto& modifier = m_VarStack.back();
				if ( auto floatValue = GetVarFloatAddr( modifier.Index ) )
					*floatValue = modifier.Value.x;
				else if ( auto vec2Value = GetVarVec2Addr( modifier.Index ) )
					*vec2Value = ImVec2( modifier.Value.x, modifier.Value.y );
				else if ( auto vec4Value = GetVarVec4Addr( modifier.Index ) )
					*vec4Value = modifier.Value;
				m_VarStack.pop_back();
				--count;
			}
		}

		const char* GetColorName( StyleColor colorIndex ) const
		{
			switch ( colorIndex )
			{
			case StyleColor_Value: return "Value";
			case StyleColor_Count: break;
			}

			IM_ASSERT( 0 );
			return "Unknown";
		}

	private:
		struct ColorModifier
		{
			StyleColor  Index;
			ImVec4      Value;
		};

		struct VarModifier
		{
			StyleVar Index;
			ImVec4   Value;
		};

		float* GetVarFloatAddr( StyleVar idx )
		{
			switch ( idx )
			{
			case StyleVar_HueSelector_Thickness_ZeroWidth:	return &HueSelector_Thickness_ZeroWidth;
			default:				return nullptr;
			}
		}
		ImVec2* GetVarVec2Addr( StyleVar idx )
		{
			IM_UNUSED( idx );

			return NULL;
		}
		ImVec4* GetVarVec4Addr( StyleVar idx )
		{
			IM_UNUSED( idx );

			return NULL;
		}

		ImVector<ColorModifier>	m_ColorStack;
		ImVector<VarModifier>	m_VarStack	;
	};

	Style& GetStyle();

	inline
	const char* GetStyleColorName( StyleColor colorIndex )
	{
		GetStyle().GetColorName( colorIndex );
	}
	inline
	void PushStyleColor( StyleColor colorIndex, const ImVec4& color )
	{
		GetStyle().PushColor( colorIndex, color );
	}
	inline
	void PopStyleColor( int count = 1 )
	{
		GetStyle().PopColor( count );
	}
	inline
	void PushStyleVar( StyleVar varIndex, float value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( StyleVar varIndex, const ImVec2& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( StyleVar varIndex, const ImVec4& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PopStyleVar( int count = 1 )
	{
		GetStyle().PopVar( count );
	}

#define ImWidgets_Kibi (1024ull)
#define ImWidgets_Mibi (ImWidgets_Kibi*1024ull)
#define ImWidgets_Gibi (ImWidgets_Mibi*1024ull)
#define ImWidgets_Tebi (ImWidgets_Gibi*1024ull)
#define ImWidgets_Pebi (ImWidgets_Tebi*1024ull)

	typedef int ImWidgetsLengthUnit;
	typedef int ImWidgetsChromaticPlot;
	typedef int ImWidgetsObserver;
	typedef int ImWidgetsIlluminance;
	typedef int ImWidgetsColorSpace;
	typedef int ImWidgetsPointer;

	enum ImWidgetsLengthUnit_
	{
		ImWidgetsLengthUnit_Metric = 0,
		ImWidgetsLengthUnit_Imperial,
		ImWidgetsLengthUnit_COUNT
	};

	enum ImWidgetsObserver_
	{
		// Standard
		ImWidgetsObserverChromaticPlot_1931_2deg = 0,
		ImWidgetsObserverChromaticPlot_1964_10deg,
		ImWidgetsObserverChromaticPlot_COUNT
	};

	enum ImWidgetsIlluminance_
	{
		// White Points
		ImWidgetsWhitePointChromaticPlot_A = 0,
		ImWidgetsWhitePointChromaticPlot_B,
		ImWidgetsWhitePointChromaticPlot_C,
		ImWidgetsWhitePointChromaticPlot_D50,
		ImWidgetsWhitePointChromaticPlot_D55,
		ImWidgetsWhitePointChromaticPlot_D65,
		ImWidgetsWhitePointChromaticPlot_D75,
		ImWidgetsWhitePointChromaticPlot_D93,
		ImWidgetsWhitePointChromaticPlot_E,
		ImWidgetsWhitePointChromaticPlot_F1,
		ImWidgetsWhitePointChromaticPlot_F2,
		ImWidgetsWhitePointChromaticPlot_F3,
		ImWidgetsWhitePointChromaticPlot_F4,
		ImWidgetsWhitePointChromaticPlot_F5,
		ImWidgetsWhitePointChromaticPlot_F6,
		ImWidgetsWhitePointChromaticPlot_F7,
		ImWidgetsWhitePointChromaticPlot_F8,
		ImWidgetsWhitePointChromaticPlot_F9,
		ImWidgetsWhitePointChromaticPlot_F10,
		ImWidgetsWhitePointChromaticPlot_F11,
		ImWidgetsWhitePointChromaticPlot_F12,
		ImWidgetsWhitePointChromaticPlot_COUNT
	};

	enum ImWidgetsColorSpace_
	{
		// Color Spaces
		ImWidgetsColorSpace_AdobeRGB = 0,	// D65
		ImWidgetsColorSpace_AppleRGB,		// D65
		ImWidgetsColorSpace_Best,			// D50
		ImWidgetsColorSpace_Beta,			// D50
		ImWidgetsColorSpace_Bruce,			// D65
		ImWidgetsColorSpace_CIERGB,			// E
		ImWidgetsColorSpace_ColorMatch,		// D50
		ImWidgetsColorSpace_Don_RGB_4,		// D50
		ImWidgetsColorSpace_ECI,			// D50
		ImWidgetsColorSpace_Ekta_Space_PS5,	// D50
		ImWidgetsColorSpace_NTSC,			// C
		ImWidgetsColorSpace_PAL_SECAM,		// D65
		ImWidgetsColorSpace_ProPhoto,		// D50
		ImWidgetsColorSpace_SMPTE_C,		// D65
		ImWidgetsColorSpace_sRGB,			// D65
		ImWidgetsColorSpace_WideGamutRGB,	// D50
		ImWidgetsColorSpace_Rec2020,		// D65
		ImWidgetsColorSpace_COUNT
	};

	enum ImWidgetsChromaticPlot_
	{
		// Style
		ImWidgetsChromaticPlot_ShowWavelength,
		ImWidgetsChromaticPlot_ShowGrid,
		ImWidgetsChromaticPlot_ShowPrimaries,
		ImWidgetsChromaticPlot_ShowWhitePoint,

		ImWidgetsChromaticPlot_COUNT
	};

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	inline
	float ImCbrt( float x )
	{
		return cbrtf( x );
		//return ImPow( x, 1.0f / 3.0f );
	}

	inline
	float ImTan2( float x, float y )
	{
		return atan2f( y, x );
	}

	inline
	float ImFract(float x)
	{
		return x - ImFloor( x );
	}

	inline
	float ImRound(float x)
	{
		return roundf(x);
	}

	inline
	float ImSmoothStep(float edge0, float edge1, float x)
	{
		// Scale, bias and saturate x to 0..1 range
		x = ImClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		// Evaluate polynomial
		return x * x * (3.0f - 2.0f * x);
	}

	inline
	float ImDot( ImVec4 const& a, ImVec4 const& b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}
	inline
	float ImDot3( ImVec4 const& a, ImVec4 const& b )
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	inline
	float ImDot3( float* a, float* b )
	{
		return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
	}
	inline
	float	ImNormalize01(float const x, float const _min, float const _max)
	{
		return ( x - _min ) / ( _max - _min );
	}
	inline
	float	ImScaleFromNormalized(float const x, float const newMin, float const newMax)
	{
		return x * ( newMax - newMin ) + newMin;
	}
	inline
	float	ImRescale(float const x, float const _min, float const _max, float const newMin, float const newMax)
	{
		return ImScaleFromNormalized( ImNormalize01( x, _min, _max ), newMin, newMax );
	}
	template < typename Type >
	inline
	Type	Normalize01(Type const x, Type const _min, Type const _max)
	{
		return ( x - _min ) / ( _max - _min );
	}
	template < typename Type >
	inline
	Type	ScaleFromNormalized(Type const x, Type const newMin, Type const newMax)
	{
		return x * ( newMax - newMin ) + newMin;
	}
	template < typename Type >
	inline
	Type	Rescale(Type const x, Type const _min, Type const _max, Type const newMin, Type const newMax)
	{
		return ScaleFromNormalized( Normalize01( x, _min, _max ), newMin, newMax );
	}

	inline
	float  ImLengthSqr3( const ImVec4& lhs )
	{
		return ( lhs.x * lhs.x ) + ( lhs.y * lhs.y ) + ( lhs.z * lhs.z );
	}
	inline
	float ImLength(ImVec2 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
	}
	inline
	float ImLengthL1(ImVec2 v)
	{
		return ImAbs( v.x ) + ImAbs( v.y );
	}
	inline
	ImVec2 ImNormalized(ImVec2 v)
	{
		return v / ImLength( v );
	}
	inline
	ImVec2 ImHalfTurn(ImVec2 v)
	{
		return ImVec2(-v.y, v.x);
	}
	inline
	ImVec2 ImAntiHalfTurn(ImVec2 v)
	{
		return ImVec2(v.y, -v.x);
	}
	inline
	float ImLength(ImVec4 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
	}
	inline
	float ImLength3(ImVec4 v)
	{
		return ImSqrt( ImLengthSqr3( v ) );
	}
	float	ImLinearSample( float t, float* buffer, int count );
	inline
	float	ImFunctionFromData( float const x, float const minX, float const maxX, float* data, int const samples_count )
	{
		float const t = ImSaturate( ImNormalize01( x, minX, maxX ) );

		return ImLinearSample( t, data, samples_count );
	}

	inline
	float	ImsRGBToLinear( float x )
	{
		if ( x <= 0.04045f )
			return x / 12.92f;
		else
			return ImPow( ( x + 0.055f ) / 1.055f, 2.4f);
	}
	inline
	float	ImLinearTosRGB( float x )
	{
		if ( x <= 0.0031308f )
			return 12.92f * x;
		else
			return 1.055f * ImPow( x, 1.0f / 2.4f) - 0.055f;
	}
	IMGUI_API void          ColorConvertsRGBtosRGB( float r, float g, float b, float& out_r, float& out_g, float& out_b );
	IMGUI_API void          ColorConvertRGBtoLinear( float r, float g, float b, float& out_L, float& out_a, float& out_b );
	IMGUI_API void          ColorConvertLineartoRGB( float L, float a, float b, float& out_r, float& out_g, float& out_b );
	IMGUI_API void          ColorConvertRGBtoOKLAB( float r, float g, float b, float& out_L, float& out_a, float& out_b );
	IMGUI_API void          ColorConvertOKLABtoRGB( float L, float a, float b, float& out_r, float& out_g, float& out_b );
	IMGUI_API void          ColorConvertOKLCHtoOKLAB( float r, float g, float b, float& out_L, float& out_a, float& out_b );
	IMGUI_API void          ColorConvertOKLABtoOKLCH( float L, float a, float b, float& out_r, float& out_g, float& out_b );
	IMGUI_API void          ColorConvertsRGBtoOKLCH( float r, float g, float b, float& out_L, float& out_c, float& out_h );
	IMGUI_API void          ColorConvertOKLCHtosRGB( float L, float c, float h, float& out_r, float& out_g, float& out_b );
	ImU32	KelvinTemperatureTosRGBColors( float temperature ); // [ 1000 K; 12000 K ]

	inline
	void Mat33RowMajorMulVec3( float& x, float& y, float& z, float* mat33RowMajor, float* vec3 )
	{
		x = ImDot3( mat33RowMajor + 0, vec3 );
		y = ImDot3( mat33RowMajor + 3, vec3 );
		z = ImDot3( mat33RowMajor + 6, vec3 );
	}

	inline
	void ImU32ColorToImRGBColor(ImVector<float>& colorsConverted, ImU32* colors, int color_count)
	{
		ImU32* current = colors;
		colorsConverted.resize( 3 * color_count );
		for ( int k = 0; k < color_count; ++k )
		{
			ImVec4 col = ( ImVec4 )ImColor( *current );
			colorsConverted[ 3 * k + 0 ] = col.x;
			colorsConverted[ 3 * k + 1 ] = col.y;
			colorsConverted[ 3 * k + 2 ] = col.z;
			++current;
		}
	}

	inline
	void ImComputeRect( ImRect* p_bb, ImVec2* pts, int pts_count )
	{
		ImRect& bb = *p_bb;
		bb.Min = ImVec2( FLT_MAX, FLT_MAX );
		bb.Max = ImVec2( -FLT_MAX, -FLT_MAX );
		for ( int k = 0; k < pts_count; ++k )
		{
			bb.Min.x = ImMin( bb.Min.x, pts[ k ].x );
			bb.Min.y = ImMin( bb.Min.y, pts[ k ].y );
			bb.Max.x = ImMax( bb.Max.x, pts[ k ].x );
			bb.Max.y = ImMax( bb.Max.y, pts[ k ].y );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Color Functions
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API ImU32	ImColorFrom_xyz( float x, float y, float z, float* xyzToRGB, float gamma );

	//IMGUI_API void ColorConvertHWBtoRGB( float h, float w, float b, float& r, float& g, float& b );

	IMGUI_API ImU32 ImColorBlendsRGB( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLinear( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHSL( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHSLa( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHWB( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLCH( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLab( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendOklab( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendOkLCH( ImU32 col0, ImU32 col1, float t );

	//////////////////////////////////////////////////////////////////////////
	// Scalar Helpers
	//////////////////////////////////////////////////////////////////////////
	void	ScaleData( ImGuiDataType data_type, void* p_data, double value );
	void	ScaleData( ImGuiDataType data_type, void* p_data, ImU64 value );
	bool	IsNegativeScalar( ImGuiDataType data_type, ImU64* src );
	bool	IsPositiveScalar( ImGuiDataType data_type, ImU64* src );
	void	EqualScalar( ImGuiDataType data_type, ImU64* p_target, ImU64* p_source );
	void	SetScalarIndirect( ImGuiDataType data_type, void* p_source, int idx, ImU64* value );
	float	ScalarToFloat( ImGuiDataType data_type, ImU64* p_source );
	float	ScalarIndirectToFloat( ImGuiDataType data_type, void* p_source, int idx );
	ImU64	ScalarIndirectToScalar( ImGuiDataType data_type, void* p_source, int idx );
	ImU64	FloatToScalar( ImGuiDataType data_type, float f_value );
	ImU64	AddScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	SubScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	MulScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	DivScalar( ImGuiDataType data_type, void* p_a, void* p_b );
	ImU64	ClampScalar( ImGuiDataType data_type, void* p_value, void* p_min, void* p_max );
	ImU64	Normalize01( ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max );
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );

	//////////////////////////////////////////////////////////////////////////
	// Geometry Generation
	//////////////////////////////////////////////////////////////////////////
#ifdef DEAR_WIDGETS_TESSELATION
	void	ShapeTesselationUniform( ImShape& shape );
#endif

	void	ShapeTranslate( ImShape& shape, ImVec2 t );

	void	ShapeSetDefaultUV( ImShape& shape );
	void	ShapeSetDefaultUVCol( ImShape& shape );
	void	ShapeSetDefaultBoundUV( ImShape& shape );
	void	ShapeSetDefaultBoundUVWhiteCol( ImShape& shape );
	void	ShapeSetDefaultWhiteCol( ImShape& shape );

	void	GenShapeRect( ImShape& shape, ImRect const& r );
	void	GenShapeCircle( ImShape& shape, ImVec2 center, float radius, int side_count );
	void	GenShapeCircleArc( ImShape& shape, ImVec2 center, float radius, float angle_min, float angle_max, int side_count );
	void	GenShapeRegularNGon( ImShape& shape, ImVec2 center, float radius, int side_count );

	// TODO
	//void	GenShapeFromBezierCubicCurve( ImShape& shape, ImVector<ImVec2>& path, float thickness, int num_segments = 0 );
	//void	GenShapeFromBezierQuadraticCurve( ImShape& sshape, ImVector<ImVec2>& path, float thickness, int num_segments = 0 );

	// TODO Add Color Blend Option (Linear, sRGB, ...) cf. W3C rules
	typedef void	( *pfSpace2sRGB )( float, float, float, float&, float&, float& );
	typedef void	( *pfsRGB2Space )( float, float, float, float&, float&, float& );
	void	ShapeLinearGradientGeneric( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );
	void	ShapeRadialGradientGeneric( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );
	void	ShapeDiamondGradientGeneric( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );

	void	ShapeSRGBLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeSRGBRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeSRGBDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeLinearSRGBLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeLinearSRGBRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeLinearSRGBDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeHSVLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeHSVRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeHSVDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );

	//////////////////////////////////////////////////////////////////////////
	// DrawList
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API void DrawShapeDebugEx( ImDrawList* pDrawList, ImTextureID tex, ImShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawShapeEx( ImDrawList* pDrawList, ImTextureID tex, ImShape& shape );
	IMGUI_API void DrawShapeDebug( ImDrawList* pDrawList, ImShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawShape( ImDrawList* pDrawList, ImShape& shape );
	IMGUI_API void DrawImageShapeDebug( ImDrawList* pDrawList, ImTextureID tex, ImShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawImageShape( ImDrawList* pDrawList, ImTextureID tex, ImShape& shape );

	IMGUI_API void DrawTriangleCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, float thickness, ImU32 col );
	IMGUI_API void DrawTriangleCursorFilled( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, ImU32 col );

	IMGUI_API void DrawSignetCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, float thickness, ImU32 col );
	IMGUI_API void DrawSignetFilledCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, ImU32 col );

	IMGUI_API void DrawProceduralColor1DNearest( ImDrawList* pDrawList, ImColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
	IMGUI_API void DrawProceduralColor1DBilinear( ImDrawList* pDrawList, ImColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );

	IMGUI_API void DrawProceduralColor2DNearest( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
	IMGUI_API void DrawProceduralColor2DBilinear( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );

	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset );
	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[ 3 ], float alpha, float gamma );
	IMGUI_API void DrawLumianceBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );
	IMGUI_API void DrawSaturationBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );

	IMGUI_API void DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, ImColor1DCallback func, void* pUserData, int division, float colorOffset, bool bIsBilinear );

	IMGUI_API void DrawOkLabQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );
	IMGUI_API void DrawOkLchQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );

	// poly: Clockwise: Positive shape & Counter-clockwise for hole
	IMGUI_API void DrawShapeWithHole( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );

	IMGUI_API void DrawImageConvexShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										 ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );
	IMGUI_API void DrawImageConcaveShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										  ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );

	// TODO: find a clean way expose the style of the draws:
	// Triangle of ColorSpace
	// White Point
	IMGUI_API
	void	DrawChromaticityPlotGeneric( ImDrawList* pDrawList,
										 ImVec2 const curPos,
										 float width, float height,
										 ImVec2 primR, ImVec2 primG, ImVec2 primB,
										 ImVec2 whitePoint,
										 float* xyzToRGB,
										 int const chromeLineSamplesCount,
										 float* observerX, float* observerY, float* observerZ,
										 int const observerSampleCount,
										 float const observerWavelengthMin, float const observerWavelengthMax,
										 float* standardCIE,
										 int const standardCIESampleCount,
										 float const standardCIEWavelengthMin, float const standardCIEWavelengthMax,
										 float gamma,
										 int resX, int resY,
										 ImU32 maskColor,
										 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
										 float minX = 0.0f, float maxX = 0.8f,
										 float minY = 0.0f, float maxY = 0.9f,
										 bool showColorSpaceTriangle = true,
										 bool showWhitePoint = true,
										 bool showBorder = true,
										 ImU32 borderColor = IM_COL32( 0, 0, 0, 255 ),
										 float borderThickness = 1.0f );
	IMGUI_API void DrawChromaticityPlot( ImDrawList* draw,
										 ImWidgetsIlluminance illuminance,
										 ImWidgetsObserver observer,
										 ImWidgetsColorSpace colorSpace,
										 int chromeLineSamplesCount,
										 ImVec2 const vpos, ImVec2 const size,
										 int resolutionX, int resolutionY,
										 ImU32 maskColor,
										 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
										 float minX = 0.0f, float maxX = 0.8f,
										 float minY = 0.0f, float maxY = 0.9f,
										 bool showColorSpaceTriangle = true,
										 bool showWhitePoint = true,
										 bool showBorder = true,
										 ImU32 borderColor = IM_COL32( 0, 0, 0, 255 ),
										 float borderThickness = 1.0f );
	IMGUI_API
	void	DrawChromaticityPointsGeneric( ImDrawList* pDrawList,
										   ImVec2 curPos,
										   ImVec2 size,
										   float* rgbToXYZ,
										   float* colors4, // AoS
										   int color_count,
										   float minX, float maxX,
										   float minY, float maxY,
										   ImU32 plotColor, float radius, int num_segments,
										   int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else
	IMGUI_API void DrawChromaticityPoints( ImDrawList* pDrawList,
										   ImVec2 curPos,
										   ImVec2 size,
										   ImU32* colors4,
										   int color_count,
										   float minX, float maxX,
										   float minY, float maxY,
										   ImU32 plotColor, float radius, int num_segments );
	IMGUI_API void DrawChromaticityPoints( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImWidgetsColorSpace colorSpace,
										  float* colors4, // AoS
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, float radius, int num_segments,
										  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else );
	IMGUI_API
	void	DrawChromaticityLinesGeneric( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  float* rgbToXYZ,
										  float* colors4, // AoS
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, ImDrawFlags flags, float thickness,
										  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else );
	IMGUI_API void DrawChromaticityLines( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImU32* color,
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, ImDrawFlags flags, float thickness );
	IMGUI_API void DrawChromaticityLines( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImWidgetsColorSpace colorSpace,
										  float* colors4, // AoS
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, ImDrawFlags flags, float thickness,
										  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else );

	IMGUI_API void DrawLinearLineGraduation( ImDrawList* drawlist, ImVec2 start, ImVec2 end,
										 float mainLineThickness, ImU32 mainCol,
										 int division0, float height0, float thickness0, float angle0, ImU32 col0,
										 int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
										 int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );
	IMGUI_API void DrawLinearCircularGraduation( ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
												 float mainLineThickness, ImU32 mainCol,
												 int division0, float height0, float thickness0, float angle0, ImU32 col0,
												 int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
												 int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );

	IMGUI_API void DrawLogLineGraduation( ImDrawList* drawlist, ImVec2 start, ImVec2 end,
												float mainLineThickness, ImU32 mainCol,
												int division0, float height0, float thickness0, float angle0, ImU32 col0,
												int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );

	IMGUI_API void DrawLogCircularGraduation( ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
												float mainLineThickness, ImU32 mainCol,
												int division0, float height0, float thickness0, float angle0, ImU32 col0,
												int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );

	typedef void ( *ImDrawShape )( ImDrawList* drawlist, ImVec2* pts, int pts_count, ImU32 col, float thickness );
	typedef void ( *ImDrawShapeFilled )( ImDrawList* drawlist, ImVec2* pts, int pts_count, ImU32 col );
	IMGUI_API void RenderNavHighlightShape( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavHighlightFlags flags, ImDrawShape func );
	IMGUI_API void RenderNavHighlightConvex( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavHighlightFlags flags );
	IMGUI_API void RenderNavHighlightConcave( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavHighlightFlags flags );
	IMGUI_API void RenderNavHighlightWithHole( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavHighlightFlags flags );

	IMGUI_API void RenderFrameShape( ImVec2* pts, int pts_count, ImU32 fill_col, bool border, ImDrawShape outline, ImDrawShapeFilled fill );
	IMGUI_API void RenderFrameConcave( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );
	IMGUI_API void RenderFrameConvex( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );
	IMGUI_API void RenderFrameWithHole( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );

	//////////////////////////////////////////////////////////////////////////
	// Interactions
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool IsPolyConvexContains( ImVec2* pts, int pts_count, ImVec2 p );
	IMGUI_API bool IsPolyConcaveContains( ImVec2* pts, int pts_count, ImVec2 p );
	IMGUI_API bool IsPolyWithHoleContains( ImVec2* pts, int pts_count, ImVec2 p, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );

	IMGUI_API bool IsMouseHoveringPolyConvex( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
	IMGUI_API bool ItemHoverablePolyConvex( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
	IMGUI_API bool IsMouseHoveringPolyConcave( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
	IMGUI_API bool ItemHoverablePolyConcave( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
	IMGUI_API bool IsMouseHoveringPolyWithHole( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
	IMGUI_API bool ItemHoverablePolyWithHole( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );

	typedef bool (*ImItemHoverablePolyConvexFunc)( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
	IMGUI_API bool ButtonBehaviorShape( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags, ImItemHoverablePolyConvexFunc func );
	IMGUI_API bool ButtonBehaviorConvex( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorConcave( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorWithHole( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );

	//////////////////////////////////////////////////////////////////////////
	// Widgets
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool ButtonExShape( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags, ImItemHoverablePolyConvexFunc func, ImDrawShape outline, ImDrawShapeFilled fill );
	IMGUI_API bool ButtonExConvex( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExConcave( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExWithHole( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );

	IMGUI_API bool HueSelector( char const* label, float hueHeight, float cursorHeight, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f );
	IMGUI_API bool SliderNScalar( char const* label, ImGuiDataType data_type, void* ordered_value, int value_count, void* p_min, void* p_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNFloat( char const* label, ImGuiDataType data_type, float* ordered_value, int value_count, float v_min, float v_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNInt( char const* label, ImGuiDataType data_type, int* ordered_value, int value_count, int v_min, int v_max, float cursor_width, bool show_hover_by_region );
	// TODO: Add bool flipY
	IMGUI_API bool Slider2DScalar( char const* pLabel, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY );
	IMGUI_API bool Slider2DFloat( char const* pLabel, float* pValueX, float* pValueY, float v_minX, float v_maxX, float v_minY, float v_maxY );
	IMGUI_API bool Slider2DInt( char const* pLabel, int* pValueX, void* pValueY, int v_minX, int v_maxX, int v_minY, int v_maxY );

	//IMGUI_API bool SliderRingScalar( char const* name,
	//								 ImGuiDataType data_type,
	//								 void* p_value, void* p_min, void* p_max,
	//								 float v_angle_min, float v_angle_max,
	//								 float v_thickness, const char* format,
	//								 ImGuiSliderFlags flags, ImRect* out_grab_bb );

	//IMGUI_API bool DragFloatPrecise( char const* label, float* value, float v_min, float v_max, ImGuiSliderFlags flags );

	//////////////////////////////////////////////////////////////////////////
	// Window Customization
	//////////////////////////////////////////////////////////////////////////
	// Note: it will break the rounding.
	IMGUI_API void SetCurrentWindowBackgroundImage( ImTextureID id, ImVec2 imgSize, bool fixedSize = false, ImU32 col = IM_COL32( 255, 255, 255, 255 ) );
}
