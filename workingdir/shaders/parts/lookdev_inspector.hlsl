// LookDevInspector: two-image A/B compare. texture0 is side A (the image drawn), texture1 side B
// (ImPlatform_SetShaderTexture slot 1). A divider through a pivot at an angle picks the side per pixel; each side has
// its own exposure, black / white points and gamma.
// Shader parts (ImPlatform_ParseShaderParts): generated for every backend at run time.

//@params
float4 sideA;       // (exposure, black, white, gamma)
float4 sideB;
float4 divider;     // (pivot_u, pivot_v, angle_rad, swap)
float4 canvas;      // (w_px, h_px, 1/w_px, 1/h_px)

//@textures
1 texture1

//@functions
float3 apply_tone( float3 rgb, float4 tone )
{
	rgb *= exp2( tone.x );                                      // exposure in stops
	rgb = ( rgb - tone.y ) / max( tone.z - tone.y, 1e-6 );      // black / white
	rgb = max( rgb, 0.0 );
	float g = 1.0 / max( tone.w, 1e-4 );                        // gamma
	return pow( rgb, float3( g, g, g ) );
}

//@ps
// The side test is in pixel space (u scaled by the aspect) so it matches the widget's divider gizmo.
float aspect = canvas.x / max( canvas.y, 1.0 );
float cs = cos( divider.z );
float sn = sin( divider.z );
float2 p     = float2( v.uv.x * aspect, v.uv.y );
float2 pivot = float2( divider.x * aspect, divider.y );
float d = ( p.x - pivot.x ) * cs + ( p.y - pivot.y ) * sn;
bool onB = d < 0.0;
if ( divider.w > 0.5 )
	onB = !onB;     // swap

float4 c;
if ( onB )
{
	c = IMP_SAMPLE( texture1, v.uv );
	c.rgb = apply_tone( c.rgb, sideB );
}
else
{
	c = IMP_SAMPLE( texture0, v.uv );
	c.rgb = apply_tone( c.rgb, sideA );
}
c.rgb = saturate( c.rgb );
col_out = c * v.col;
