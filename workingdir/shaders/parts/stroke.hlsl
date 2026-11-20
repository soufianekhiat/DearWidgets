// Stroke fill: winding number per pixel over a "line soup" of directed segments (the Linebender GPU stroke expansion
// paper, HPG 2024); the distance to the nearest segment antialiases the outside edge.
// Shader parts (ImPlatform_ParseShaderParts): generated for every backend at run time. Shader model 3 (DirectX 9)
// cannot index constants in a loop: there the segments are read from segTex (t1, RGBA32F, one texel per segment,
// refilled before each draw) and a draw holds at most STROKE_SEGS of them (the C++ side splits strokes into bands).

//@defines
#ifdef IMP_SM3
#define STROKE_SEGS 192
#else
#define STROKE_SEGS 1024
#endif

//@params
float4 params;          // x = segment count, z = antialiasing width, w = 1 / segTex width
float4 strokeColor;     // RGBA
float4 bounds;          // xy = rect min, zw = rect max (screen space)
float4 segments[STROKE_SEGS];  // xy = p0, zw = p1 of each directed segment

//@textures
1 segTex

//@ps
// UV [0, 1] mapped to screen space within the bounds
float2 P = lerp( bounds.xy, bounds.zw, v.uv );
int num = IMP_CAST( int, params.x );
float aa = params.z;

// Winding number: a horizontal ray from P to +infinity crossing each segment p0 -> p1
int winding = 0;
float min_dist_sq = 1e10;
IMP_LOOP for ( int i = 0; i < STROKE_SEGS; i++ )
{
	if ( i >= num )
		break;
#ifdef IMP_SM3
	float4 s = IMP_SAMPLE_LEVEL( segTex, float2( ( IMP_CAST( float, i ) + 0.5 ) * params.w, 0.5 ), 0.0 );
#else
	float4 s = segments[ i ];
#endif
	float2 p0 = s.xy;
	float2 p1 = s.zw;
	float side = ( p1.x - p0.x ) * ( P.y - p0.y ) - ( P.x - p0.x ) * ( p1.y - p0.y );
	if ( p0.y <= P.y )
	{
		if ( p1.y > P.y && side > 0.0 )
			winding++;
	}
	else if ( p1.y <= P.y && side < 0.0 )
		winding--;

	// Distance to the segment, for the antialiasing
	float2 d = p1 - p0;
	float len_sq = dot( d, d );
	float t = len_sq > 1e-8 ? saturate( dot( P - p0, d ) / len_sq ) : 0.0;
	float2 diff = P - ( p0 + d * t );
	min_dist_sq = min( min_dist_sq, dot( diff, diff ) );
}

col_out = strokeColor;      // Inside: full alpha
if ( winding == 0 )
{
	// Outside: faded by the distance to the nearest edge
	float min_dist = sqrt( min_dist_sq );
	if ( !( aa > 0.0 && min_dist < aa ) )
		discard;
	col_out = float4( strokeColor.rgb, strokeColor.a * ( 1.0 - min_dist / aa ) );
}
