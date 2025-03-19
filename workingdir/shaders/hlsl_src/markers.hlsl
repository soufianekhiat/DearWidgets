cbuffer PS_CONSTANT_BUFFER
{
	float4	fg_color;
	float4	bg_color;

	float2	rotation;
	float	linewidth;
	float	size;

	float	type;
	float	antialiasing;
	float	draw_type;
	float	pad0;
};

cbuffer vertexBuffer
{
	float4x4 ProjectionMatrix;
};

struct VS_INPUT
{
	float2 pos : POSITION;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};

#define PI 3.14159265358979323846264f
#define SQRT_2 1.4142135623730951f

// --- disc
float disc( float2 P, float size )
{
	return length( P ) - size * 0.5f;
}

// --- square
float square( float2 P, float size )
{
	return max( abs( P.x ), abs( P.y ) ) - size / ( 2.0f * SQRT_2 );
}

// --- triangle2
float triangle2( float2 P, float size )
{
	float x = SQRT_2 / 2.0f * ( P.x - P.y );
	float y = SQRT_2 / 2.0f * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / ( 2.0f * SQRT_2 );
	float r2 = P.y;
	return max( r1, r2 );
}

// --- diamond
float diamond( float2 P, float size )
{
	float x = SQRT_2 / 2.0f * ( P.x - P.y );
	float y = SQRT_2 / 2.0f * ( P.x + P.y );
	return max( abs( x ), abs( y ) ) - size / ( 2.0f * SQRT_2 );
}

// --- heart
float heart( float2 P, float size )
{
	float x = SQRT_2 / 2.0f * ( P.x - P.y );
	float y = SQRT_2 / 2.0f * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / 3.5f;
	float r2 = length( P - SQRT_2 / 2.0f * float2( +1.0f, -1.0f ) * size / 3.5f ) - size / 3.5f;
	float r3 = length( P - SQRT_2 / 2.0f * float2( -1.0f, -1.0f ) * size / 3.5f ) - size / 3.5f;
	return min( min( r1, r2 ), r3 );
}

// --- spade
float spade( float2 P, float size )
{
	// Reversed heart (diamond + 2 circles)
	float s = size * 0.85f / 3.5f;
	float x = SQRT_2 / 2.0f * ( P.x + P.y ) + 0.4f * s;
	float y = SQRT_2 / 2.0f * ( P.x - P.y ) - 0.4f * s;
	float r1 = max( abs( x ), abs( y ) ) - s;
	float r2 = length( P - SQRT_2 / 2.0f * float2( +1.0f, +0.2f ) * s ) - s;
	float r3 = length( P - SQRT_2 / 2.0f * float2( -1.0f, +0.2f ) * s ) - s;
	float r4 = min( min( r1, r2 ), r3 );

	// Root (2 circles and 2 planes)
	float2 c1 = float2( +0.65f, 0.125f );
	float2 c2 = float2( -0.65f, 0.125f );
	float r5 = length( P - c1 * size ) - size / 1.6f;
	float r6 = length( P - c2 * size ) - size / 1.6f;
	float r7 = P.y - 0.5f * size;
	float r8 = 0.1f * size - P.y;
	float r9 = max( -min( r5, r6 ), max( r7, r8 ) );

	return min( r4, r9 );
}

// --- club
float club( float2 P, float size )
{
	// clover (3 discs)
	float t1 = -PI / 2.0f;
	float2  c1 = 0.225f * float2( cos( t1 ), sin( t1 ) );
	float t2 = t1 + 2.0f * PI / 3.0;
	float2  c2 = 0.225f * float2( cos( t2 ), sin( t2 ) );
	float t3 = t2 + 2.0f * PI / 3.0;
	float2  c3 = 0.225f * float2( cos( t3 ), sin( t3 ) );
	float r1 = length( P - c1 * size ) - size / 4.25f;
	float r2 = length( P - c2 * size ) - size / 4.25f;
	float r3 = length( P - c3 * size ) - size / 4.25f;
	float r4 = min( min( r1, r2 ), r3 );

	// Root (2 circles and 2 planes)
	float2 c4 = float2( +0.65f, 0.125f );
	float2 c5 = float2( -0.65f, 0.125f );
	float r5 = length( P - c4 * size ) - size / 1.6f;
	float r6 = length( P - c5 * size ) - size / 1.6f;
	float r7 = P.y - 0.5f * size;
	float r8 = 0.2f * size - P.y;
	float r9 = max( -min( r5, r6 ), max( r7, r8 ) );

	return min( r4, r9 );
}

// --- chevron
float chevron( float2 P, float size )
{
	float x = 1.0f / SQRT_2 * ( P.x - P.y );
	float y = 1.0f / SQRT_2 * ( P.x + P.y );
	float r1 = max( abs( x ), abs( y ) ) - size / 3.0f;
	float r2 = max( abs( x - size / 3 ), abs( y - size / 3.0f ) ) - size / 3.0f;
	return max( r1, -r2 );
}

// --- clover
float clover( float2 P, float size )
{
	float t1 = -PI / 2.0f; 
	float2  c1 = 0.25f * float2( cos( t1 ), sin( t1 ) );
	float t2 = t1 + 2.0f * PI / 3.0f;
	float2  c2 = 0.25f * float2( cos( t2 ), sin( t2 ) );
	float t3 = t2 + 2.0f * PI / 3.0f;
	float2  c3 = 0.25f * float2( cos( t3 ), sin( t3 ) );

	float r1 = length( P - c1 * size ) - size / 3.5f;
	float r2 = length( P - c2 * size ) - size / 3.5f;
	float r3 = length( P - c3 * size ) - size / 3.5f;
	return min( min( r1, r2 ), r3 );
}

// --- ring
float ring( float2 P, float size )
{
	float r1 = length( P ) - size / 2.0f;
	float r2 = length( P ) - size / 4.0f;
	return max( r1, -r2 );
}

// --- tag
float tag( float2 P, float size )
{
	float r1 = max( abs( P.x ) - size / 2.0f, abs( P.y ) - size / 6.0f );
	float r2 = abs( P.x - size / 1.5f ) + abs( P.y ) - size;
	return max( r1, .75f * r2 );
}

// --- cross
float cross( float2 P, float size )
{
	float x = SQRT_2 / 2.0f * ( P.x - P.y );
	float y = SQRT_2 / 2.0f * ( P.x + P.y );
	float r1 = max( abs( x - size / 3.0f ), abs( x + size / 3.0f ) );
	float r2 = max( abs( y - size / 3.0f ), abs( y + size / 3.0f ) );
	float r3 = max( abs( x ), abs( y ) );
	float r = max( min( r1, r2 ), r3 );
	r -= size / 2.0f;
	return r;
}

// --- asterisk
float asterisk( float2 P, float size )
{
	float x = SQRT_2 / 2.0f * ( P.x - P.y );
	float y = SQRT_2 / 2.0f * ( P.x + P.y );
	float r1 = max( abs( x ) - size / 2.0f, abs( y ) - size / 10.0f );
	float r2 = max( abs( y ) - size / 2.0f, abs( x ) - size / 10.0f );
	float r3 = max( abs( P.x ) - size / 2.0f, abs( P.y ) - size / 10.0f );
	float r4 = max( abs( P.y ) - size / 2.0f, abs( P.x ) - size / 10.0f );
	return min( min( r1, r2 ), min( r3, r4 ) );
}

// --- infinity
float infinity( float2 P, float size )
{
	float2 c1 = float2( +0.2125f, 0.00f );
	float2 c2 = float2( -0.2125f, 0.00f );
	float r1 = length( P - c1 * size ) - size / 3.5f;
	float r2 = length( P - c1 * size ) - size / 7.5f;
	float r3 = length( P - c2 * size ) - size / 3.5f;
	float r4 = length( P - c2 * size ) - size / 7.5f;
	return min( max( r1, -r2 ), max( r3, -r4 ) );
}

// --- pin
float pin( float2 P, float size )
{
	float2 c1 = float2( 0.0f, -0.15f ) * size;
	float r1 = length( P - c1 ) - size / 2.675f;
	float2 c2 = float2( +1.49f, -0.80f ) * size;
	float r2 = length( P - c2 ) - 2.0f * size;
	float2 c3 = float2( -1.49f, -0.80f ) * size;
	float r3 = length( P - c3 ) - 2.0f * size;
	float r4 = length( P - c1 ) - size / 5.0f;
	return max( min( r1, max( max( r2, r3 ), -P.y ) ), -r4 );
}

// --- arrow
float arrow( float2 P, float size )
{
	float r1 = abs( P.x ) + abs( P.y ) - size / 2.0f;
	float r2 = max( abs( P.x + size / 2.0f ), abs( P.y ) ) - size / 2.0f;
	float r3 = max( abs( P.x - size / 6.0f ) - size / 4.0f, abs( P.y ) - size / 4.0f );
	return min( r3, max( .75f * r1, r2 ) );
}

// --- ellipse
// Created by Inigo Quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
float ellipse( float2 P, float size )
{
	float2 ab = float2( size / 3.0f, size / 2.0f );
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

	float c = ( m2 + n2 - 1.0f ) / 3.0f;
	float c3 = c * c * c;

	float q = c3 + m2 * n2 * 2.0f;
	float d = c3 + m2 * n2;
	float g = m + m * n2;

	float co;

	if ( d < 0.0f )
	{
		float p = acos( q / c3 ) / 3.0f;
		float s = cos( p );
		float t = sin( p ) * sqrt( 3.0f );
		float rx = sqrt( -c * ( s + t + 2.0f ) + m2 );
		float ry = sqrt( -c * ( s - t + 2.0f ) + m2 );
		co = ( ry + sign( l ) * rx + abs( g ) / ( rx * ry ) - m ) / 2.0f;
	}
	else
	{
		float h = 2.0f * m * n * sqrt( d );
		float s = sign( q + h ) * pow( abs( q + h ), 1.0f / 3.0f );
		float u = sign( q - h ) * pow( abs( q - h ), 1.0f / 3.0f );
		float rx = -s - u - c * 4.0f + 2.0f * m2;
		float ry = ( s - u ) * sqrt( 3.0f );
		float rm = sqrt( rx * rx + ry * ry );
		float p = ry / sqrt( rm - rx );
		co = ( p + 2.0f * g / rm - m ) / 2.0f;
	}

	float si = sqrt( 1.0f - co * co );
	float2 closestPoint = float2( ab.x * co, ab.y * si );
	return length( closestPoint - p ) * sign( p.y - closestPoint.y );
}

float ellipse_fast( float2 P, float size )
{
	float a = 1.0f;
	float b = 3.0f;
	float r = 0.9f;
	float f = length( P*float2(a,b) );
	f = length( P*float2(a,b) );
	f = f*(f-r)/length( P*float2(a*a,b*b) );
	return f;
}

float4 stroke(float distance, float linewidth, float antialias, float4 stroke)
{
	float4 frag_color;
	float t = linewidth / 2.0f - antialias;
	float signed_distance = distance;
	float border_distance = abs( signed_distance ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );

	if ( border_distance < 0.0f )
		frag_color = stroke;
	else
		frag_color = float4( stroke.rgb, stroke.a * alpha );

	return frag_color;
}

float4 filled(float distance, float linewidth, float antialias, float4 fill)
{
	float4 frag_color;
	float t = linewidth / 2.0f - antialias;
	float signed_distance = distance;
	float border_distance = abs( signed_distance ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );

	if ( border_distance < 0.0f )
		frag_color = fill;
	else if ( signed_distance < 0.0f )
		frag_color = fill;
	else
		frag_color = float4( fill.rgb, alpha * fill.a );

	return frag_color;
}

float4 outline( float distance, float linewidth, float antialias, float4 stroke, float4 fill )
{
	float4 frag_color;
	float t = linewidth / 2.0f - antialias;
	float signed_distance = distance;
	float border_distance = abs( signed_distance ) - t;
	float alpha = border_distance / antialias;
	alpha = exp( -alpha * alpha );

	if ( border_distance < 0.0f )
		frag_color = stroke;
	else if ( signed_distance < 0.0f )
		frag_color = lerp( fill, stroke, sqrt( alpha ) );
	else
		frag_color = float4( stroke.rgb, stroke.a * alpha );

	return frag_color;
}

SamplerState sampler0;
Texture2D texture0;

PS_INPUT main_vs(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
	output.uv  = input.uv;
	return output;
}

float4 main_ps(PS_INPUT input) : SV_Target
{
	float2 uv = input.uv.xy;
	float4 col_in = input.col;
	float4 col_out = float4(1.0f, 1.0f, 1.0f, 1.0f);
		
	float2 P = uv - 0.5f;
	P.y = -P.y;
	P = float2( rotation.x * P.x - rotation.y * P.y,
	rotation.y * P.x + rotation.x * P.y );

	float antialias = antialiasing;
	float point_size = SQRT_2 * size + 2.0f * ( linewidth + 1.5f * antialias );
	float distance;
	if ( type == 0.0f ) // Disc
		distance = disc( P * point_size, size );
	else if ( type == 1.0f ) // Square
		distance = square( P * point_size, size );
	else if (type ==  2.0f) // Triangle
		distance = triangle2( P * point_size, size );
	else if (type ==  3.0f) // Diamond
		distance = diamond( P * point_size, size );
	else if (type ==  4.0f) // Heart
		distance = heart( P * point_size, size );
	else if (type ==  5.0f) // Spade
		distance = spade( P * point_size, size );
	else if (type ==  6.0f) // Club
		distance = club( P * point_size, size );
	else if (type ==  7.0f) // Chevron
		distance = chevron( P * point_size, size );
	else if (type ==  8.0f) // Clover
		distance = clover( P * point_size, size );
	else if (type ==  9.0f) // Ring
		distance = ring( P * point_size, size );
	else if (type == 10.0f) // Tag
		distance = tag( P * point_size, size );
	else if (type == 11.0f) // Cross
		distance = cross( P * point_size, size );
	else if (type == 12.0f) // Asterisk
		distance = asterisk( P * point_size, size );
	else if (type == 13.0f) // Infinity
		distance = infinity( P * point_size, size );
	else if (type == 14.0f) // Pin
		distance = pin( P * point_size, size );
	else if (type == 15.0f) // Arrow
		distance = arrow( P * point_size, size );
	else if (type == 16.0f) // Ellipse
		distance = ellipse( P * point_size, size );
	else if (type == 17.0f) // EllipseApprox
		distance = ellipse_fast( P * point_size, size );

	if ( draw_type == 0.0f )
		col_out = filled( distance, linewidth, antialias, fg_color ); 
	else if ( draw_type == 1.0f )
		col_out = stroke(distance, linewidth, antialias, fg_color);
	else if ( draw_type == 2.0f )
		col_out = outline( distance, linewidth, antialias, fg_color, bg_color );
	else if ( draw_type == 3.0f )
		col_out = float4( pow( abs( distance ), 1.0f / 2.2f ).xxx, 1.0f );
	else if ( draw_type == 4.0f )
		col_out = lerp(fg_color, bg_color, distance > 0.0f ? 1.0f.xxxx : 0.0f.xxxx);
		
	return col_out;
}

