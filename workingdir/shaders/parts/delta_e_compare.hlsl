// DeltaE Compare: A/B image Delta E map. texture0 is side A (the image drawn), texture1 side B
// (ImPlatform_SetShaderTexture slot 1); Delta E per pixel in the selected formula, mapped to a false-color ramp.
// Inputs are treated as sRGB.
// Shader parts (ImPlatform_ParseShaderParts): generated for every backend at run time.

//@params
uint4  modePack;    // (formula: 0 dE76, 1 dE-OK, 2 dE94 approx, 3 dE2000 approx; ramp: 0 gray, 1 viridis, 2 turbo, 3 magma; show_threshold; 0)
float4 rangePack;   // (max_dE, threshold, gain, bias)
float4 mixPack;     // (show_A_alpha, show_B_alpha, show_de_alpha, 0)

//@textures
1 texture1

//@functions
float3 srgb_to_linear( float3 c )
{
	float3 lo = c / 12.92;
	float3 hi = pow( ( c + 0.055 ) / 1.055, float3( 2.4, 2.4, 2.4 ) );
	return lerp( lo, hi, step( float3( 0.04045, 0.04045, 0.04045 ), c ) );
}

// Linear -> OKLab (Bjorn Ottosson)
float3 linear_to_oklab( float3 rgb )
{
	float l = 0.4122214708 * rgb.r + 0.5363325363 * rgb.g + 0.0514459929 * rgb.b;
	float m = 0.2119034982 * rgb.r + 0.6806995451 * rgb.g + 0.1073969566 * rgb.b;
	float s = 0.0883024619 * rgb.r + 0.2817188376 * rgb.g + 0.6299787005 * rgb.b;
	float l_ = pow( max( l, 0.0 ), 1.0 / 3.0 );
	float m_ = pow( max( m, 0.0 ), 1.0 / 3.0 );
	float s_ = pow( max( s, 0.0 ), 1.0 / 3.0 );
	return float3(
		0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
		1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
		0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_ );
}

// Linear RGB (D65) -> CIE XYZ
float3 linear_to_xyz( float3 rgb )
{
	return float3(
		0.4124564 * rgb.r + 0.3575761 * rgb.g + 0.1804375 * rgb.b,
		0.2126729 * rgb.r + 0.7151522 * rgb.g + 0.0721750 * rgb.b,
		0.0193339 * rgb.r + 0.1191920 * rgb.g + 0.9503041 * rgb.b );
}

float lab_f( float t )
{
	return t > 0.008856 ? pow( t, 1.0 / 3.0 ) : ( 7.787 * t + 16.0 / 116.0 );
}

float3 xyz_to_lab( float3 xyz )
{
	float3 n = xyz / float3( 0.95047, 1.00000, 1.08883 );     // D65 reference white
	float fx = lab_f( n.x ), fy = lab_f( n.y ), fz = lab_f( n.z );
	return float3( 116.0 * fy - 16.0, 500.0 * ( fx - fy ), 200.0 * ( fy - fz ) );
}

// Polynomial fits of the matplotlib colormaps (viridis / magma: Matt Zucker, turbo: Google's reference approximation)
float3 viridis( float t )
{
	t = saturate( t );
	const float3 c0 = float3( 0.2777273272234177, 0.005407344544966578, 0.3340998053353061 );
	const float3 c1 = float3( 0.1050930431085774, 1.404613529898575, 1.384590162594685 );
	const float3 c2 = float3( -0.3308618287255563, 0.214847559468213, 0.09509516302823659 );
	const float3 c3 = float3( -4.634230498983486, -5.799100973351585, -19.33244095627987 );
	const float3 c4 = float3( 6.228269936347081, 14.17993336680509, 56.69055260068105 );
	const float3 c5 = float3( 4.776384997670288, -13.74514537774601, -65.35303263337234 );
	const float3 c6 = float3( -5.435455855934631, 4.645852612178535, 26.3124352495832 );
	return saturate( c0 + t * ( c1 + t * ( c2 + t * ( c3 + t * ( c4 + t * ( c5 + t * c6 ) ) ) ) ) );
}

float3 turbo( float t )
{
	t = saturate( t );
	const float4 kRed4   = float4( 0.13572138, 4.61539260, -42.66032258, 132.13108234 );
	const float4 kGreen4 = float4( 0.09140261, 2.19418839, 4.84296658, -14.18503333 );
	const float4 kBlue4  = float4( 0.10667330, 12.64194608, -60.58204836, 110.36276771 );
	const float2 kRed2   = float2( -152.94239396, 59.28637943 );
	const float2 kGreen2 = float2( 4.27729857, 2.82956604 );
	const float2 kBlue2  = float2( -89.90310912, 27.34824973 );
	float4 v4 = float4( 1.0, t, t * t, t * t * t );
	float2 v2 = v4.zw * v4.z;
	return saturate( float3( dot( v4, kRed4 ) + dot( v2, kRed2 ), dot( v4, kGreen4 ) + dot( v2, kGreen2 ), dot( v4, kBlue4 ) + dot( v2, kBlue2 ) ) );
}

float3 magma( float t )
{
	t = saturate( t );
	const float3 c0 = float3( -0.002136485053939582, -0.000749655052795221, -0.005386127855323933 );
	const float3 c1 = float3( 0.2516605407371642, 0.6775232436837668, 2.494026599312351 );
	const float3 c2 = float3( 8.353717279216625, -3.577719514958484, 0.3144679030132573 );
	const float3 c3 = float3( -27.66873308576866, 14.26473078096533, -13.64921318813922 );
	const float3 c4 = float3( 52.17613981234068, -27.94360607168351, 12.94416944238394 );
	const float3 c5 = float3( -50.76852536473588, 29.04658282127291, 4.23415299384598 );
	const float3 c6 = float3( 18.65570506591883, -11.48977351997711, -5.601961508734096 );
	return saturate( c0 + t * ( c1 + t * ( c2 + t * ( c3 + t * ( c4 + t * ( c5 + t * c6 ) ) ) ) ) );
}

float3 ramp( float t, uint mode )
{
	if ( mode == 1u ) return viridis( t );
	if ( mode == 2u ) return turbo( t );
	if ( mode == 3u ) return magma( t );
	return float3( t, t, t );
}

//@ps
float3 a_srgb = IMP_SAMPLE( texture0, v.uv ).rgb;
float3 b_srgb = IMP_SAMPLE( texture1, v.uv ).rgb;
float3 a_lin = srgb_to_linear( a_srgb );
float3 b_lin = srgb_to_linear( b_srgb );

uint formula = modePack.x;
float dE = 0.0;
if ( formula == 1u )    // dE-OK: OkLab distances are ~0..1, scaled to the Lab range for display
{
	dE = length( linear_to_oklab( a_lin ) - linear_to_oklab( b_lin ) ) * 100.0;
}
else                    // dE76 / dE94 / dE2000 approximations, all from the CIE Lab distance
{
	float3 lab_a = xyz_to_lab( linear_to_xyz( a_lin ) );
	float3 lab_b = xyz_to_lab( linear_to_xyz( b_lin ) );
	dE = length( lab_a - lab_b );
	if ( formula == 2u )
	{
		// CIE94 simplified: L, C, H weighted separately
		float dL = lab_a.x - lab_b.x;
		float dC = length( lab_a.yz ) - length( lab_b.yz );
		float dH2 = max( dot( lab_a.yz - lab_b.yz, lab_a.yz - lab_b.yz ) - dC * dC, 0.0 );
		float SC = 1.0 + 0.045 * length( lab_a.yz );
		float SH = 1.0 + 0.015 * length( lab_a.yz );
		dE = sqrt( dL * dL + ( dC / SC ) * ( dC / SC ) + dH2 / ( SH * SH ) );
	}
	else if ( formula == 3u )
		dE = dE * 0.85;     // A tighter CIE94-like approximation of dE2000
}

float max_dE = max( rangePack.x, 1e-3 );
float t = ( dE + rangePack.w ) * rangePack.z / max_dE;
float3 de_col = ramp( saturate( t ), modePack.y );

// Layers mixed by the mixPack alphas: the Delta E map, A and B
float3 result = de_col * mixPack.z + a_srgb * mixPack.x + b_srgb * mixPack.y;

// Threshold overlay: red where dE exceeds the threshold
if ( modePack.z > 0u && dE > rangePack.y )
	result = lerp( result, float3( 1.0, 0.0, 0.0 ), 0.35 );

col_out = float4( saturate( result ), 1.0 ) * v.col;
