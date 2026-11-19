// GPU shape markers (DrawMarker): a signed distance per marker type, filled / stroked / outlined with antialiasing.
// Shader parts (ImPlatform_ParseShaderParts): generated for every backend at run time.
// Parameters must match ImWidgetsMarkerBuffer in dear_widgets.h.

//@params
float4 fg_color;
float4 bg_color;
float2 rotation;
float  linewidth;
float  size;
float  type;
float  antialiasing;
float  draw_type;
float  pad0;

//@defines
PI=3.14159265358979323846264
SQRT_2=1.4142135623730951

//@functions
float disc( float2 P, float size )
{
	return length( P ) - size * 0.5;
}

float square( float2 P, float size )
{
	return max( abs( P.x ), abs( P.y ) ) - size / ( 2.0 * SQRT_2 );
}

float triangle2( float2 P, float size )
{
	float x = SQRT_2 / 2.0 * ( P.x - P.y );
	float y = SQRT_2 / 2.0 * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / ( 2.0 * SQRT_2 );
	float r2 = P.y;
	return max( r1, r2 );
}

float diamond( float2 P, float size )
{
	float x = SQRT_2 / 2.0 * ( P.x - P.y );
	float y = SQRT_2 / 2.0 * ( P.x + P.y );
	return max( abs( x ), abs( y ) ) - size / ( 2.0 * SQRT_2 );
}

float heart( float2 P, float size )
{
	float x = SQRT_2 / 2.0 * ( P.x - P.y );
	float y = SQRT_2 / 2.0 * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / 3.5;
	float r2 = length( P - SQRT_2 / 2.0 * float2( 1.0, -1.0 ) * size / 3.5 ) - size / 3.5;
	float r3 = length( P - SQRT_2 / 2.0 * float2( -1.0, -1.0 ) * size / 3.5 ) - size / 3.5;
	return min( min( r1, r2 ), r3 );
}

float spade( float2 P, float size )
{
	// Reversed heart (diamond + 2 circles)
	float s = size * 0.85 / 3.5;
	float x = SQRT_2 / 2.0 * ( P.x + P.y ) + 0.4 * s;
	float y = SQRT_2 / 2.0 * ( P.x - P.y ) - 0.4 * s;
	float r1 = max( abs( x ), abs( y ) ) - s;
	float r2 = length( P - SQRT_2 / 2.0 * float2( 1.0, 0.2 ) * s ) - s;
	float r3 = length( P - SQRT_2 / 2.0 * float2( -1.0, 0.2 ) * s ) - s;
	float r4 = min( min( r1, r2 ), r3 );

	// Root (2 circles and 2 planes)
	float2 c1 = float2( 0.65, 0.125 );
	float2 c2 = float2( -0.65, 0.125 );
	float r5 = length( P - c1 * size ) - size / 1.6;
	float r6 = length( P - c2 * size ) - size / 1.6;
	float r7 = P.y - 0.5 * size;
	float r8 = 0.1 * size - P.y;
	float r9 = max( -min( r5, r6 ), max( r7, r8 ) );

	return min( r4, r9 );
}

float club( float2 P, float size )
{
	// Clover (3 discs)
	float t1 = -PI / 2.0;
	float2 c1 = 0.225 * float2( cos( t1 ), sin( t1 ) );
	float t2 = t1 + 2.0 * PI / 3.0;
	float2 c2 = 0.225 * float2( cos( t2 ), sin( t2 ) );
	float t3 = t2 + 2.0 * PI / 3.0;
	float2 c3 = 0.225 * float2( cos( t3 ), sin( t3 ) );
	float r1 = length( P - c1 * size ) - size / 4.25;
	float r2 = length( P - c2 * size ) - size / 4.25;
	float r3 = length( P - c3 * size ) - size / 4.25;
	float r4 = min( min( r1, r2 ), r3 );

	// Root (2 circles and 2 planes)
	float2 c4 = float2( 0.65, 0.125 );
	float2 c5 = float2( -0.65, 0.125 );
	float r5 = length( P - c4 * size ) - size / 1.6;
	float r6 = length( P - c5 * size ) - size / 1.6;
	float r7 = P.y - 0.5 * size;
	float r8 = 0.2 * size - P.y;
	float r9 = max( -min( r5, r6 ), max( r7, r8 ) );

	return min( r4, r9 );
}

float chevron( float2 P, float size )
{
	float x = 1.0 / SQRT_2 * ( P.x - P.y );
	float y = 1.0 / SQRT_2 * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / 3.0;
	float r2 = max( abs( x - size / 3.0 ), abs( y - size / 3.0 ) ) - size / 3.0;
	return max( r1, -r2 );
}

float clover( float2 P, float size )
{
	float t1 = -PI / 2.0;
	float2 c1 = 0.25 * float2( cos( t1 ), sin( t1 ) );
	float t2 = t1 + 2.0 * PI / 3.0;
	float2 c2 = 0.25 * float2( cos( t2 ), sin( t2 ) );
	float t3 = t2 + 2.0 * PI / 3.0;
	float2 c3 = 0.25 * float2( cos( t3 ), sin( t3 ) );

	float r1 = length( P - c1 * size ) - size / 3.5;
	float r2 = length( P - c2 * size ) - size / 3.5;
	float r3 = length( P - c3 * size ) - size / 3.5;
	return min( min( r1, r2 ), r3 );
}

float ring( float2 P, float size )
{
	float r1 = length( P ) - size / 2.0;
	float r2 = length( P ) - size / 4.0;
	return max( r1, -r2 );
}

float tag( float2 P, float size )
{
	float r1 = max( abs( P.x ) - size / 2.0, abs( P.y ) - size / 6.0 );
	float r2 = abs( P.x - size / 1.5 ) + abs( P.y ) - size;
	return max( r1, 0.75 * r2 );
}

float marker_cross( float2 P, float size )
{
	float x = SQRT_2 / 2.0 * ( P.x - P.y );
	float y = SQRT_2 / 2.0 * ( P.x + P.y );
	float r1 = max( abs( x - size / 3.0 ), abs( x + size / 3.0 ) );
	float r2 = max( abs( y - size / 3.0 ), abs( y + size / 3.0 ) );
	float r3 = max( abs( x ), abs( y ) );
	float r = max( min( r1, r2 ), r3 );
	r -= size / 2.0;
	return r;
}

float asterisk( float2 P, float size )
{
	float x = SQRT_2 / 2.0 * ( P.x - P.y );
	float y = SQRT_2 / 2.0 * ( P.x + P.y );
	float r1 = max( abs( x ) - size / 2.0, abs( y ) - size / 10.0 );
	float r2 = max( abs( y ) - size / 2.0, abs( x ) - size / 10.0 );
	float r3 = max( abs( P.x ) - size / 2.0, abs( P.y ) - size / 10.0 );
	float r4 = max( abs( P.y ) - size / 2.0, abs( P.x ) - size / 10.0 );
	return min( min( r1, r2 ), min( r3, r4 ) );
}

float infinity( float2 P, float size )
{
	float2 c1 = float2( 0.2125, 0.0 );
	float2 c2 = float2( -0.2125, 0.0 );
	float r1 = length( P - c1 * size ) - size / 3.5;
	float r2 = length( P - c1 * size ) - size / 7.5;
	float r3 = length( P - c2 * size ) - size / 3.5;
	float r4 = length( P - c2 * size ) - size / 7.5;
	return min( max( r1, -r2 ), max( r3, -r4 ) );
}

float pin( float2 P, float size )
{
	float2 c1 = float2( 0.0, -0.15 ) * size;
	float r1 = length( P - c1 ) - size / 2.675;
	float2 c2 = float2( 1.49, -0.80 ) * size;
	float r2 = length( P - c2 ) - 2.0 * size;
	float2 c3 = float2( -1.49, -0.80 ) * size;
	float r3 = length( P - c3 ) - 2.0 * size;
	float r4 = length( P - c1 ) - size / 5.0;
	return max( min( r1, max( max( r2, r3 ), -P.y ) ), -r4 );
}

float arrow( float2 P, float size )
{
	float r1 = abs( P.x ) + abs( P.y ) - size / 2.0;
	float r2 = max( abs( P.x + size / 2.0 ), abs( P.y ) ) - size / 2.0;
	float r3 = max( abs( P.x - size / 6.0 ) - size / 4.0, abs( P.y ) - size / 4.0 );
	return min( r3, max( 0.75 * r1, r2 ) );
}

// Created by Inigo Quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
float ellipse( float2 P, float size )
{
	float2 ab = float2( size / 3.0, size / 2.0 );
	float2 p = abs( P );
	if ( p.x > p.y )
	{
		p = p.yx;
		ab = ab.yx;
	}
	float l = ab.y * ab.y - ab.x * ab.x;
	float m = ab.x * p.x / l;
	float n = ab.y * p.y / l;
	float m2 = m * m;
	float n2 = n * n;

	float c = ( m2 + n2 - 1.0 ) / 3.0;
	float c3 = c * c * c;

	float q = c3 + m2 * n2 * 2.0;
	float d = c3 + m2 * n2;
	float g = m + m * n2;

	float co;
	if ( d < 0.0 )
	{
		float h = acos( q / c3 ) / 3.0;
		float s = cos( h );
		float t = sin( h ) * sqrt( 3.0 );
		float rx = sqrt( -c * ( s + t + 2.0 ) + m2 );
		float ry = sqrt( -c * ( s - t + 2.0 ) + m2 );
		co = ( ry + sign( l ) * rx + abs( g ) / ( rx * ry ) - m ) / 2.0;
	}
	else
	{
		float h = 2.0 * m * n * sqrt( d );
		float s = sign( q + h ) * pow( abs( q + h ), 1.0 / 3.0 );
		float u = sign( q - h ) * pow( abs( q - h ), 1.0 / 3.0 );
		float rx = -s - u - c * 4.0 + 2.0 * m2;
		float ry = ( s - u ) * sqrt( 3.0 );
		float rm = sqrt( rx * rx + ry * ry );
		float k = ry / sqrt( rm - rx );
		co = ( k + 2.0 * g / rm - m ) / 2.0;
	}

	float si = sqrt( 1.0 - co * co );
	float2 closestPoint = float2( ab.x * co, ab.y * si );
	return length( closestPoint - p ) * sign( p.y - closestPoint.y );
}

float ellipse_fast( float2 P, float size )
{
	float a = 1.0;
	float b = 3.0;
	float r = 0.9;
	float f = length( P * float2( a, b ) );
	f = f * ( f - r ) / length( P * float2( a * a, b * b ) );
	return f;
}

float4 marker_stroke( float dist, float lw, float antialias, float4 color )
{
	float t = lw / 2.0 - antialias;
	float border_distance = abs( dist ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );
	if ( border_distance < 0.0 )
		return color;
	return float4( color.rgb, color.a * alpha );
}

float4 marker_filled( float dist, float lw, float antialias, float4 fill )
{
	float t = lw / 2.0 - antialias;
	float border_distance = abs( dist ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );
	if ( border_distance < 0.0 || dist < 0.0 )
		return fill;
	return float4( fill.rgb, alpha * fill.a );
}

float4 marker_outline( float dist, float lw, float antialias, float4 color, float4 fill )
{
	float t = lw / 2.0 - antialias;
	float border_distance = abs( dist ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );
	if ( border_distance < 0.0 )
		return color;
	if ( dist < 0.0 )
		return lerp( fill, color, sqrt( alpha ) );
	return float4( color.rgb, color.a * alpha );
}

//@ps
float2 P = v.uv - float2( 0.5, 0.5 );
P.y = -P.y;
P = float2( rotation.x * P.x - rotation.y * P.y, rotation.y * P.x + rotation.x * P.y );

float antialias = antialiasing;
float point_size = SQRT_2 * size + 2.0 * ( linewidth + 1.5 * antialias );
float2 Q = P * point_size;
float dist = 0.0;
if ( type == 0.0 )       dist = disc( Q, size );
else if ( type == 1.0 )  dist = square( Q, size );
else if ( type == 2.0 )  dist = triangle2( Q, size );
else if ( type == 3.0 )  dist = diamond( Q, size );
else if ( type == 4.0 )  dist = heart( Q, size );
else if ( type == 5.0 )  dist = spade( Q, size );
else if ( type == 6.0 )  dist = club( Q, size );
else if ( type == 7.0 )  dist = chevron( Q, size );
else if ( type == 8.0 )  dist = clover( Q, size );
else if ( type == 9.0 )  dist = ring( Q, size );
else if ( type == 10.0 ) dist = tag( Q, size );
else if ( type == 11.0 ) dist = marker_cross( Q, size );
else if ( type == 12.0 ) dist = asterisk( Q, size );
else if ( type == 13.0 ) dist = infinity( Q, size );
else if ( type == 14.0 ) dist = pin( Q, size );
else if ( type == 15.0 ) dist = arrow( Q, size );
else if ( type == 16.0 ) dist = ellipse( Q, size );
else if ( type == 17.0 ) dist = ellipse_fast( Q, size );

col_out = float4( 1.0, 1.0, 1.0, 1.0 );
if ( draw_type == 0.0 )
	col_out = marker_filled( dist, linewidth, antialias, fg_color );
else if ( draw_type == 1.0 )
	col_out = marker_stroke( dist, linewidth, antialias, fg_color );
else if ( draw_type == 2.0 )
	col_out = marker_outline( dist, linewidth, antialias, fg_color, bg_color );
else if ( draw_type == 3.0 )
{
	float g = pow( abs( dist ), 1.0 / 2.2 );
	col_out = float4( g, g, g, 1.0 );
}
else if ( draw_type == 4.0 )
	col_out = dist > 0.0 ? bg_color : fg_color;
