// Background effects shader.
// Historically an uber-shader branching on `mode`, which made D3DCompile spend
// ~9.5 s optimizing every effect together. It is now compiled once PER EFFECT:
// the host injects `#define BG_EFFECT <n>` and only that effect's code is
// compiled (everything else is #if'd out), so each variant compiles in ms and
// only the effects actually used are ever compiled. Runtime still selects via
// the `mode` constant (which equals BG_EFFECT for the compiled variant).

cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjMtx;
};

cbuffer PS_CONSTANT_BUFFER : register(b1)
{
	float2 texel_size;   // 1.0 / texture_dimensions
	float  mode;         // effect index
	float  param0;       // primary param
	float  param1;       // secondary param
	float  param2;       // tertiary param
	float2 win_center;   // window center in UV space
	float2 win_half_px;  // window half-size in pixels
	float  win_rounding; // corner radius in pixels (from ImGui)
	float  pad0;
	float2 mouse_uv;     // mouse position in UV space
	float  pad1;
	float  pad2;
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

#define PI 3.14159265358979323846f

#define MODE_BLUR          0
#define MODE_GLASS_REFRACT 1
#define MODE_FROSTED_GLASS 2
#define MODE_PIXELATE      3
#define MODE_CHROMATIC     4
#define MODE_LIQUID_GLASS  5
#define MODE_HEAT_HAZE     6
#define MODE_VORONOI       7
#define MODE_EDGE_GLOW     8
#define MODE_HALFTONE      9
#define MODE_MOUSE_EDGE   10
#define MODE_CRT          11
#define MODE_DOT_MATRIX   12
#define MODE_GLITCH       13
#define MODE_STAINED_GLASS 14
#define MODE_RAIN         15
#define MODE_KALEIDOSCOPE 16

// Default to plain blur if the host did not inject a specific effect.
#ifndef BG_EFFECT
#define BG_EFFECT MODE_BLUR
#endif

Texture2D    sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

// ---- Helpers (shared) ----

float4 sampleClamped(float2 uv)
{
	return sceneTexture.Sample(sceneSampler, clamp(uv, texel_size * 0.5f, 1.0f - texel_size * 0.5f));
}

// sRGB <-> linear conversion (exact IEC 61966-2-1)
float srgbToLinear(float c)
{
	return (c <= 0.04045f) ? c / 12.92f : pow((c + 0.055f) / 1.055f, 2.4f);
}
float linearToSrgb(float c)
{
	return (c <= 0.0031308f) ? c * 12.92f : 1.055f * pow(c, 1.0f / 2.4f) - 0.055f;
}
float3 srgbToLinear3(float3 c) { return float3(srgbToLinear(c.r), srgbToLinear(c.g), srgbToLinear(c.b)); }
float3 linearToSrgb3(float3 c) { return float3(linearToSrgb(c.r), linearToSrgb(c.g), linearToSrgb(c.b)); }

// Sample in linear space
float4 sampleLinear(float2 uv)
{
	float4 c = sampleClamped(uv);
	c.rgb = srgbToLinear3(c.rgb);
	return c;
}

// Parallax: offset UVs based on distance from window center
float2 applyParallax(float2 uv, float strength)
{
	float2 winSizeUV = win_half_px * texel_size * 2.0f;
	float2 from_center = (uv - win_center) / max(winSizeUV, float2(0.001f, 0.001f));
	return uv + from_center * strength * texel_size * 2.0f;
}

// ---- SDF (pixel space) ----

float sdRoundedBox(float2 p, float2 b, float r)
{
	float2 q = abs(p) - b + r;
	return length(max(q, 0.0f)) + min(max(q.x, q.y), 0.0f) - r;
}

float2 uvToPixel(float2 uv)
{
	return (uv - win_center) / texel_size;
}

float sdfWithNormal(float2 uv, float extraRadius, out float2 normal)
{
	float2 p = uvToPixel(uv);
	float2 b = win_half_px;
	float  r = win_rounding + extraRadius;
	float d = sdRoundedBox(p, b, r);
	float eps = 1.0f;
	float dx = sdRoundedBox(p + float2(eps, 0), b, r) - sdRoundedBox(p - float2(eps, 0), b, r);
	float dy = sdRoundedBox(p + float2(0, eps), b, r) - sdRoundedBox(p - float2(0, eps), b, r);
	normal = normalize(float2(dx, dy) + float2(0.0001f, 0.0f));
	return d;
}

// ---- Noise ----

float hash12(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * 0.1031f);
	p3 += dot(p3, p3.yzx + 33.33f);
	return frac((p3.x + p3.y) * p3.z);
}

#if BG_EFFECT == MODE_VORONOI || BG_EFFECT == MODE_STAINED_GLASS
float2 hash22(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * float3(0.1031f, 0.1030f, 0.0973f));
	p3 += dot(p3, p3.yzx + 33.33f);
	return frac((p3.xx + p3.yz) * p3.zy);
}
#endif

PS_INPUT main_vs(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = mul(ProjMtx, float4(input.pos.xy, 0.0f, 1.0f));
	output.col = input.col;
	output.uv  = input.uv;
	return output;
}

#if BG_EFFECT == MODE_BLUR
// ---- Mode 0: Two-pass Gaussian blur with SDF border refraction ----
// param0 = blur radius, param1 = direction (0=H, 1=V)
float4 effect_blur(float2 uv)
{
	float radius = param0;
	float2 dir = (param1 < 0.5f) ? float2(texel_size.x, 0.0f) : float2(0.0f, texel_size.y);

	float bevelPx = max(radius * 2.0f, 8.0f);
	float2 normal;
	float d = sdfWithNormal(uv, 0.0f, normal);
	float inside = saturate(-d / bevelPx);
	float edgeBend = saturate(1.0f - inside) * radius * 0.5f;
	uv += normal * edgeBend * texel_size;
	float edgeHighlight = pow(saturate(1.0f - abs(d) / (bevelPx * 0.5f)), 3.0f) * 0.15f;

	float step = radius / 6.0f;
	float w[7] = { 0.1964825f, 0.1748904f, 0.1225150f, 0.0675317f, 0.0292431f, 0.0099595f, 0.0026627f };

	// Accumulate in linear space for physically correct blending
	float3 lin = sampleLinear(uv).rgb * w[0];
	for (int i = 1; i < 7; ++i)
	{
		float2 off = dir * (float(i) * step);
		lin += sampleLinear(uv + off).rgb * w[i];
		lin += sampleLinear(uv - off).rgb * w[i];
	}
	lin += edgeHighlight;

	float4 col;
	col.rgb = linearToSrgb3(lin);
	col.a = 1.0f;
	return col;
}
#endif

#if BG_EFFECT == MODE_GLASS_REFRACT
// ---- Mode 1: Glass Refraction (Snell's law on rounded-rect SDF) ----
// param0 = bevel fraction, param1 = index of refraction
float4 effect_glass_refract(float2 uv)
{
	float bevelFrac = max(param0, 0.01f);
	float eta = 1.0f / max(param1, 1.01f);
	float bevelPx = bevelFrac * min(win_half_px.x, win_half_px.y);

	float2 normal;
	float d = sdfWithNormal(uv, 0.0f, normal);
	float curvature = saturate(1.0f + d / bevelPx);
	float2 refractOffset = normal * curvature * (1.0f - eta) * bevelPx * texel_size;
	float2 ruv = uv + refractOffset;

	float4 col = sampleClamped(ruv) * 0.5f;
	col += sampleClamped(ruv + texel_size * float2(1, 0)) * 0.125f;
	col += sampleClamped(ruv - texel_size * float2(1, 0)) * 0.125f;
	col += sampleClamped(ruv + texel_size * float2(0, 1)) * 0.125f;
	col += sampleClamped(ruv - texel_size * float2(0, 1)) * 0.125f;
	col.rgb += pow(curvature, 2.0f) * 0.3f;
	return col;
}
#endif

#if BG_EFFECT == MODE_FROSTED_GLASS
// ---- Mode 2: Frosted Glass + parallax ----
// param0 = blur radius, param1 = noise scale
float4 effect_frosted(float2 uv)
{
	uv = applyParallax(uv, param0);
	float2 noise_uv = uv / texel_size;
	float n0 = hash12(floor(noise_uv * param1)) * 2.0f - 1.0f;
	float n1 = hash12(floor(noise_uv * param1) + float2(7.13f, 3.71f)) * 2.0f - 1.0f;
	float2 jitter = float2(n0, n1) * param0 * texel_size;

	float2 off = texel_size * param0 * 0.5f;
	float2 suv = uv + jitter;
	float4 col  = sampleClamped(suv);
	col += sampleClamped(suv + float2(-off.x,  off.y));
	col += sampleClamped(suv + float2( off.x,  off.y));
	col += sampleClamped(suv + float2( off.x, -off.y));
	col += sampleClamped(suv + float2(-off.x, -off.y));
	return col * 0.2f;
}
#endif

#if BG_EFFECT == MODE_PIXELATE
// ---- Mode 3: Pixelate + parallax ----
// param0 = pixel block size in texels
float4 effect_pixelate(float2 uv)
{
	uv = applyParallax(uv, param0);
	float2 block = texel_size * max(param0, 1.0f);
	float2 snapped = floor(uv / block) * block + block * 0.5f;
	return sampleClamped(snapped);
}
#endif

#if BG_EFFECT == MODE_CHROMATIC
// ---- Analytic visible spectrum → linear sRGB ----
// Attempt at a compact description by Wyman, Sloan, Shirley
// Attempt at CIE 1931 color matching functions fit with Gaussians
// wavelength in nm (380..780), returns linear RGB
float3 wavelengthToRGB(float lambda)
{
	// CIE XYZ 1931 analytic approximation (multi-lobe Gaussian fit)
	float t1 = (lambda - 442.0f) * ((lambda < 442.0f) ? 0.0624f : 0.0374f);
	float t2 = (lambda - 599.8f) * ((lambda < 599.8f) ? 0.0264f : 0.0323f);
	float t3 = (lambda - 501.1f) * ((lambda < 501.1f) ? 0.0490f : 0.0382f);

	float x =  0.362f * exp(-0.5f * t1 * t1)
	         + 1.056f * exp(-0.5f * t2 * t2)
	         - 0.065f * exp(-0.5f * t3 * t3);

	float t4 = (lambda - 568.8f) * ((lambda < 568.8f) ? 0.0213f : 0.0247f);
	float t5 = (lambda - 530.9f) * ((lambda < 530.9f) ? 0.0613f : 0.0322f);

	float y =  0.821f * exp(-0.5f * t4 * t4)
	         + 0.286f * exp(-0.5f * t5 * t5);

	float t6 = (lambda - 437.0f) * ((lambda < 437.0f) ? 0.0845f : 0.0278f);
	float t7 = (lambda - 459.0f) * ((lambda < 459.0f) ? 0.0385f : 0.0725f);

	float z =  1.217f * exp(-0.5f * t6 * t6)
	         + 0.681f * exp(-0.5f * t7 * t7);

	// CIE XYZ → linear sRGB (D65)
	float3 rgb;
	rgb.r =  3.2406f * x - 1.5372f * y - 0.4986f * z;
	rgb.g = -0.9689f * x + 1.8758f * y + 0.0415f * z;
	rgb.b =  0.0557f * x - 0.2040f * y + 1.0570f * z;
	return max(rgb, 0.0f);
}

// ---- Mode 4: Chromatic Aberration (physical spectral dispersion) ----
// param0 = strength, param1 = sample count (4..16)
float4 effect_chromatic(float2 uv)
{
	float2 center = win_center;
	float2 toUV = uv - center;
	float2 winSizeUV = win_half_px * texel_size;
	float dist = length(toUV / max(winSizeUV, float2(0.001f, 0.001f)));
	float2 dir = toUV * dist * param0 * texel_size / max(length(toUV), 0.0001f);

	int N = clamp((int)param1, 4, 16);
	if (N < 4) N = 8;

	float3 col = float3(0, 0, 0);
	float3 totalWeight = float3(0, 0, 0);
	for (int i = 0; i < N; ++i)
	{
		float t = (float)i / (float)(N - 1); // 0..1

		// Map t to wavelength: short (violet 380nm) to long (red 780nm)
		float lambda = lerp(380.0f, 780.0f, t);

		// Dispersion: short wavelengths refract more, long wavelengths less
		float dispFactor = (550.0f * 550.0f) / (lambda * lambda) - 1.0f;
		float2 off = dir * dispFactor;

		float4 s = sampleClamped(uv + off);

		float3 w = wavelengthToRGB(lambda);

		col += s.rgb * w;
		totalWeight += w;
	}
	col /= max(totalWeight, float3(0.001f, 0.001f, 0.001f));
	return float4(col, 1.0f);
}
#endif

#if BG_EFFECT == MODE_LIQUID_GLASS
// ---- Mode 5: Liquid Glass ----
// param0 = refraction strength, param1 = bevel fraction
float4 effect_liquid_glass(float2 uv)
{
	float strength = max(param0, 0.01f);
	float bevelFrac = max(param1, 0.01f);
	float bevelPx = bevelFrac * min(win_half_px.x, win_half_px.y);

	float2 normal;
	float d = sdfWithNormal(uv, 0.0f, normal);
	float inside = saturate(-d / bevelPx);
	float curvature = 1.0f - inside;

	float2 refractOffset = normal * curvature * strength * bevelPx * texel_size;
	float2 ruv = uv + refractOffset;

	float4 col = sampleClamped(ruv);

	// Continuous chromatic dispersion at edges
	float dispersion = curvature * strength * bevelPx * 0.3f * texel_size.x;
	float2 dOff = normal * dispersion;
	col.r = sampleClamped(ruv + dOff).r;
	col.b = sampleClamped(ruv - dOff).b;

	// Caustic highlight
	float causticD = bevelPx * 0.3f;
	float caustic = pow(saturate(1.0f - abs(d + causticD) / (bevelPx * 0.15f)), 4.0f);
	col.rgb += caustic * 0.4f;

	// Fresnel rim
	col.rgb += pow(curvature, 3.0f) * 0.25f;
	col.rgb *= lerp(0.85f, 1.0f, inside);
	return col;
}
#endif

#if BG_EFFECT == MODE_HEAT_HAZE
// ---- Mode 6: Heat Haze (animated sinusoidal distortion) ----
// param0 = distortion amplitude (texels), param1 = wave frequency, param2 = time
float4 effect_heat_haze(float2 uv)
{
	float amplitude = max(param0, 0.5f);
	float freq = max(param1, 1.0f);
	float time = param2;

	float2 px = uv / texel_size;
	float wave1 = sin(px.y * freq * 0.05f + time * 2.3f) * cos(px.x * freq * 0.03f + time * 1.7f);
	float wave2 = sin(px.y * freq * 0.08f - time * 1.9f + 1.5f) * cos(px.x * freq * 0.04f - time * 2.1f);

	float2 distort;
	distort.x = (wave1 * 0.6f + wave2 * 0.4f) * amplitude * texel_size.x;
	distort.y = (wave2 * 0.6f + wave1 * 0.4f) * amplitude * texel_size.y * 0.5f;

	float4 col = sampleClamped(uv + distort);

	float shimmer = 1.0f + (wave1 * wave2) * 0.03f;
	col.rgb *= shimmer;

	return col;
}
#endif

#if BG_EFFECT == MODE_VORONOI
// ---- Mode 7: Voronoi Shatter with per-cell refraction ----
// param0 = cell count, param1 = edge width, param2 = IOR
float4 effect_voronoi(float2 uv)
{
	float cells = max(param0, 2.0f);
	float edgeW = max(param1, 0.5f);
	float ior = max(param2, 1.0f);
	float eta = 1.0f / max(ior, 1.01f);

	float2 px = uv / texel_size;
	float cellSize = min(win_half_px.x, win_half_px.y) * 2.0f / cells;
	float2 cell = floor(px / cellSize);

	float minDist = 1e10f;
	float secondDist = 1e10f;
	float2 closestCell = cell;

	for (int y = -1; y <= 1; ++y)
	for (int x = -1; x <= 1; ++x)
	{
		float2 neighbor = cell + float2(x, y);
		float2 pt = (neighbor + hash22(neighbor)) * cellSize;
		float d = length(px - pt);
		if (d < minDist) { secondDist = minDist; minDist = d; closestCell = neighbor; }
		else if (d < secondDist) { secondDist = d; }
	}

	float2 cellHash = hash22(closestCell * 7.31f + float2(3.17f, 1.93f));
	float2 cellTilt = cellHash * 2.0f - 1.0f;

	float2 refractOffset = cellTilt * (1.0f - eta) * cellSize * 0.4f * texel_size;
	float2 sampleUV = applyParallax(uv + refractOffset, (1.0f - eta) * cellSize * 0.2f);

	float4 col = sampleClamped(sampleUV);

	float dispersion = (1.0f - eta) * cellSize * 0.06f;
	float2 dOff = cellTilt * dispersion * texel_size;
	col.r = sampleClamped(sampleUV + dOff).r;
	col.b = sampleClamped(sampleUV - dOff).b;

	float edgeDist = secondDist - minDist;
	float edgeLine = 1.0f - saturate(edgeDist / edgeW);
	float edgeGlow = pow(edgeLine, 2.0f);
	col.rgb = lerp(col.rgb, float3(1, 1, 1), edgeGlow * 0.5f);

	float edgeDarken = saturate(edgeDist / (edgeW * 3.0f));
	col.rgb *= lerp(0.7f, 1.0f, edgeDarken);

	return col;
}
#endif

#if BG_EFFECT == MODE_EDGE_GLOW
// ---- Mode 8: Edge Glow / X-Ray ----
// param0 = glow intensity, param1 = edge threshold
float4 effect_edge_glow(float2 uv)
{
	float intensity = max(param0, 0.5f);
	float2 ts = texel_size;

	float tl = dot(sampleClamped(uv + float2(-ts.x, -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float tc = dot(sampleClamped(uv + float2( 0,    -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float tr = dot(sampleClamped(uv + float2( ts.x, -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float ml = dot(sampleClamped(uv + float2(-ts.x,  0   )).rgb, float3(0.299f, 0.587f, 0.114f));
	float mr = dot(sampleClamped(uv + float2( ts.x,  0   )).rgb, float3(0.299f, 0.587f, 0.114f));
	float bl = dot(sampleClamped(uv + float2(-ts.x,  ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float bc = dot(sampleClamped(uv + float2( 0,     ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float br = dot(sampleClamped(uv + float2( ts.x,  ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));

	float gx = -tl - 2.0f*ml - bl + tr + 2.0f*mr + br;
	float gy = -tl - 2.0f*tc - tr + bl + 2.0f*bc + br;
	float edge = sqrt(gx*gx + gy*gy);

	float angle = atan2(gy, gx);
	float hue = angle / (2.0f * PI) + 0.5f;
	float3 k = frac(float3(hue, hue - 1.0f/3.0f, hue + 1.0f/3.0f));
	float3 neon = saturate(abs(k * 6.0f - 3.0f) - 1.0f);

	float glow = saturate(edge * intensity);
	float3 bg = sampleClamped(uv).rgb * 0.15f;
	return float4(bg + neon * glow, 1.0f);
}
#endif

#if BG_EFFECT == MODE_HALFTONE
// ---- Mode 9: Halftone / Ben-Day Dots (CMYK-style) ----
// param0 = dot spacing in pixels, param1 = dot sharpness
float halftone_dot_ex(float2 px, float spacing, float angle, float intensity, float sharpness, out float2 cellCenterPx)
{
	float ca = cos(angle), sa = sin(angle);
	float2 rotated = float2(px.x * ca + px.y * sa, -px.x * sa + px.y * ca);
	float2 cell = floor(rotated / spacing) * spacing + spacing * 0.5f;
	cellCenterPx = float2(cell.x * ca - cell.y * sa, cell.x * sa + cell.y * ca);
	float dist = length(rotated - cell);
	float maxR = spacing * 0.5f;
	float dotR = intensity * maxR;
	float edge = sharpness / max(maxR * 0.1f, 0.5f);
	return saturate((dotR - dist) * edge);
}

float4 effect_halftone(float2 uv)
{
	float spacing = max(param0, 4.0f);
	float sharpness = max(param1, 0.5f);

	float2 px = uv / texel_size;
	float2 cc;

	// Cyan grid (15 deg)
	float4 srcC = sampleClamped(px * texel_size);
	float dc = halftone_dot_ex(px, spacing, 0.2618f, 0.0f, sharpness, cc);
	srcC = sampleClamped(cc * texel_size);
	float c = 1.0f - srcC.r;
	dc = halftone_dot_ex(px, spacing, 0.2618f, c, sharpness, cc);

	// Magenta grid (75 deg)
	float dm = halftone_dot_ex(px, spacing, 1.3090f, 0.0f, sharpness, cc);
	float4 srcM = sampleClamped(cc * texel_size);
	float m = 1.0f - srcM.g;
	dm = halftone_dot_ex(px, spacing, 1.3090f, m, sharpness, cc);

	// Yellow grid (0 deg)
	float dy = halftone_dot_ex(px, spacing, 0.0f, 0.0f, sharpness, cc);
	float4 srcY = sampleClamped(cc * texel_size);
	float y2 = 1.0f - srcY.b;
	dy = halftone_dot_ex(px, spacing, 0.0f, y2, sharpness, cc);

	// Black grid (45 deg)
	float dk = halftone_dot_ex(px, spacing, 0.7854f, 0.0f, sharpness, cc);
	float4 srcK = sampleClamped(cc * texel_size);
	float k = min(1.0f - srcK.r, min(1.0f - srcK.g, 1.0f - srcK.b)) * 0.5f;
	dk = halftone_dot_ex(px, spacing, 0.7854f, k, sharpness, cc);

	float3 col = float3(1, 1, 1);
	col -= float3(dc, 0, 0);
	col -= float3(0, dm, 0);
	col -= float3(0, 0, dy);
	col -= float3(dk, dk, dk);
	return float4(saturate(col), 1.0f);
}
#endif

#if BG_EFFECT == MODE_MOUSE_EDGE
// ---- Mode 10: Mouse-reactive Edge Highlight ----
// param0 = highlight radius (pixels), param1 = edge intensity
float4 effect_mouse_edge(float2 uv)
{
	float radius = max(param0, 20.0f);
	float intensity = max(param1, 1.0f);
	float2 ts = texel_size;

	float tl = dot(sampleClamped(uv + float2(-ts.x, -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float tc = dot(sampleClamped(uv + float2( 0,    -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float tr = dot(sampleClamped(uv + float2( ts.x, -ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float ml = dot(sampleClamped(uv + float2(-ts.x,  0   )).rgb, float3(0.299f, 0.587f, 0.114f));
	float mr = dot(sampleClamped(uv + float2( ts.x,  0   )).rgb, float3(0.299f, 0.587f, 0.114f));
	float bl = dot(sampleClamped(uv + float2(-ts.x,  ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float bc = dot(sampleClamped(uv + float2( 0,     ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));
	float br = dot(sampleClamped(uv + float2( ts.x,  ts.y)).rgb, float3(0.299f, 0.587f, 0.114f));

	float gx = -tl - 2.0f*ml - bl + tr + 2.0f*mr + br;
	float gy = -tl - 2.0f*tc - tr + bl + 2.0f*bc + br;
	float edge = sqrt(gx*gx + gy*gy);

	float2 px = uv / texel_size;
	float2 mousePx = mouse_uv / texel_size;
	float mouseDist = length(px - mousePx);

	float falloff = saturate(1.0f - mouseDist / radius);
	falloff = falloff * falloff;

	float2 dir = normalize(uv - mouse_uv + float2(0.0001f, 0.0f));
	float hue = atan2(dir.y, dir.x) / (2.0f * PI) + 0.5f;
	float3 k = frac(float3(hue, hue - 1.0f/3.0f, hue + 1.0f/3.0f));
	float3 highlight = saturate(abs(k * 6.0f - 3.0f) - 1.0f);

	float glow = saturate(edge * intensity) * falloff;

	float4 col = sampleClamped(uv);
	col.rgb += highlight * glow;
	return col;
}
#endif

#if BG_EFFECT == MODE_CRT
// ---- Mode 11: CRT Scanlines ----
// param0 = scanline thickness (pixels, 1..6), param1 = barrel distortion strength
float4 effect_crt(float2 uv)
{
	float lineThick = max(param0, 1.0f);
	float barrel = param1;

	float2 centered = (uv - win_center) / (win_half_px * texel_size);
	float r2 = dot(centered, centered);
	float2 distorted = uv + centered * r2 * barrel * win_half_px * texel_size * 0.1f;

	float2 px = distorted / texel_size;
	float subpixel = frac(px.x / 3.0f) * 3.0f;
	float3 phosphor;
	phosphor.r = saturate(1.0f - abs(subpixel - 0.5f) * 1.5f);
	phosphor.g = saturate(1.0f - abs(subpixel - 1.5f) * 1.5f);
	phosphor.b = saturate(1.0f - abs(subpixel - 2.5f) * 1.5f);
	phosphor = lerp(float3(1, 1, 1), phosphor, 0.6f);

	float3 src = sampleClamped(distorted).rgb;

	float lineSpacing = lineThick * 2.0f;
	float linePhase = frac(px.y / lineSpacing);
	float scanMask = smoothstep(0.0f, 0.15f, linePhase) * (1.0f - smoothstep(0.35f, 0.5f, linePhase));
	scanMask = lerp(0.15f, 1.0f, scanMask);

	float lum = dot(src, float3(0.299f, 0.587f, 0.114f));
	scanMask = lerp(scanMask, 1.0f, lum * 0.3f);

	float vignette = 1.0f - r2 * 0.4f;

	float3 col = src * phosphor * scanMask * vignette;
	return float4(saturate(col), 1.0f);
}
#endif

#if BG_EFFECT == MODE_DOT_MATRIX
// ---- Mode 12: Dot Matrix / LED ----
// param0 = cell size (pixels), param1 = dot roundness (0=square, 1=circle)
float4 effect_dot_matrix(float2 uv)
{
	float cellPx = max(param0, 3.0f);
	float roundness = saturate(param1);

	float2 px = uv / texel_size;
	float2 cell = floor(px / cellPx);
	float2 cellCenter = (cell + 0.5f) * cellPx;
	float2 inCell = (px - cellCenter) / (cellPx * 0.5f);

	float4 src = sampleClamped(cellCenter * texel_size);

	float distSq = length(inCell);
	float distBox = max(abs(inCell.x), abs(inCell.y));
	float dist = lerp(distBox, distSq, roundness);

	float lum = dot(src.rgb, float3(0.299f, 0.587f, 0.114f));
	float dotSize = lerp(0.5f, 0.95f, lum);
	float mask = saturate((dotSize - dist) * cellPx * 0.5f);

	float3 col = src.rgb * mask;

	float glow = saturate(1.0f - dist / dotSize) * 0.15f * lum;
	col += glow;

	return float4(col, 1.0f);
}
#endif

#if BG_EFFECT == MODE_GLITCH
// ---- Mode 13: Glitch / Datamosh ----
// param0 = intensity, param1 = block size, param2 = time
float4 effect_glitch(float2 uv)
{
	float intensity = max(param0, 0.1f);
	float blockSize = max(param1, 4.0f);
	float time = param2;

	float2 px = uv / texel_size;

	float timeSeed = floor(time * 8.0f);

	float lineBlock = floor(px.y / blockSize);
	float lineHash = hash12(float2(lineBlock, timeSeed));
	float lineShift = 0.0f;
	if (lineHash > (1.0f - intensity * 0.3f))
		lineShift = (hash12(float2(lineBlock + 0.5f, timeSeed)) * 2.0f - 1.0f) * intensity * 40.0f;

	float2 blockCoord = floor(px / (blockSize * 4.0f));
	float blockHash = hash12(blockCoord + timeSeed);
	float2 blockShift = float2(0, 0);
	if (blockHash > (1.0f - intensity * 0.1f))
	{
		blockShift.x = (hash12(blockCoord + timeSeed + 1.0f) * 2.0f - 1.0f) * intensity * 60.0f;
		blockShift.y = (hash12(blockCoord + timeSeed + 2.0f) * 2.0f - 1.0f) * intensity * 20.0f;
	}

	float2 shiftedUV = (px + float2(lineShift, 0) + blockShift) * texel_size;

	float channelShift = intensity * 3.0f * texel_size.x;
	float chHash = hash12(float2(timeSeed, 0.77f));
	float3 col;
	col.r = sampleClamped(shiftedUV + float2(channelShift * chHash, 0)).r;
	col.g = sampleClamped(shiftedUV).g;
	col.b = sampleClamped(shiftedUV - float2(channelShift * chHash, 0)).b;

	float invertHash = hash12(float2(lineBlock + 3.0f, timeSeed));
	if (invertHash > (1.0f - intensity * 0.05f))
		col = 1.0f - col;

	return float4(saturate(col), 1.0f);
}
#endif

#if BG_EFFECT == MODE_STAINED_GLASS
// ---- Mode 14: Stained Glass ----
// param0 = cell count, param1 = lead width (edge thickness)
float4 effect_stained_glass(float2 uv)
{
	float cells = max(param0, 2.0f);
	float leadW = max(param1, 1.0f);

	float2 px = uv / texel_size;
	float cellSize = min(win_half_px.x, win_half_px.y) * 2.0f / cells;
	float2 cell = floor(px / cellSize);

	float minDist = 1e10f;
	float secondDist = 1e10f;
	float2 closestPt = px;

	for (int y = -1; y <= 1; ++y)
	for (int x = -1; x <= 1; ++x)
	{
		float2 neighbor = cell + float2(x, y);
		float2 pt = (neighbor + hash22(neighbor)) * cellSize;
		float d = length(px - pt);
		if (d < minDist) { secondDist = minDist; minDist = d; closestPt = pt; }
		else if (d < secondDist) { secondDist = d; }
	}

	float2 centroidUV = applyParallax(closestPt * texel_size, cellSize * 0.1f);
	float3 col = float3(0, 0, 0);
	float2 ts2 = texel_size * 2.0f;
	col += sampleClamped(centroidUV).rgb;
	col += sampleClamped(centroidUV + float2(ts2.x, 0)).rgb;
	col += sampleClamped(centroidUV - float2(ts2.x, 0)).rgb;
	col += sampleClamped(centroidUV + float2(0, ts2.y)).rgb;
	col += sampleClamped(centroidUV - float2(0, ts2.y)).rgb;
	col *= 0.2f;

	float lum = dot(col, float3(0.299f, 0.587f, 0.114f));
	col = lerp(float3(lum, lum, lum), col, 1.5f);
	col = saturate(col);

	float edgeDist = secondDist - minDist;
	float lead = saturate(edgeDist / leadW);
	lead = lead * lead;

	float specular = pow(saturate(1.0f - minDist / (cellSize * 0.4f)), 8.0f) * 0.2f;

	col = col * lead * 0.9f + specular;
	col = lerp(float3(0.05f, 0.05f, 0.05f), col, lead);

	return float4(col, 1.0f);
}
#endif

#if BG_EFFECT == MODE_RAIN
// ---- Mode 15: Rain on Glass (BigWings / Heartfelt technique) ----
// param0 = rain amount (0..1), param1 = fog blur, param2 = time

float S(float a, float b, float t)
{
	return (a < b) ? smoothstep(a, b, t) : (1.0f - smoothstep(b, a, t));
}

float3 N13(float p)
{
	float3 p3 = frac(float3(p, p, p) * float3(0.1031f, 0.11369f, 0.13787f));
	p3 += dot(p3, p3.yzx + 19.19f);
	return frac(float3((p3.x+p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float Saw(float b, float t)
{
	return S(0.0f, b, t) * S(1.0f, b, t);
}

float2 DropLayer(float2 uv, float time)
{
	float2 UV = uv;

	uv.y += time * 0.75f;
	float2 a = float2(6.0f, 1.0f);
	float2 grid = a * 2.0f;
	float2 id = floor(uv * grid);

	float colShift = frac(sin(id.x * 12345.564f) * 7658.76f);
	uv.y += colShift;

	id = floor(uv * grid);
	float3 n = N13(id.x * 35.2f + id.y * 2376.1f);
	float2 st = frac(uv * grid) - float2(0.5f, 0.0f);

	float x = n.x - 0.5f;
	float y = UV.y * 20.0f;
	float wiggle = sin(y + sin(y));
	x += wiggle * (0.5f - abs(x)) * (n.z - 0.5f);
	x *= 0.7f;

	float ti = frac(time + n.z);
	y = (Saw(0.85f, ti) - 0.5f) * 0.9f + 0.5f;

	float2 p = float2(x, y);
	float d = length((st - p) * a.yx);

	float mainDrop = S(0.4f, 0.0f, d);

	float r = sqrt(S(1.0f, y, st.y));
	float cd = abs(st.x - x);
	float trail = S(0.23f * r, 0.15f * r * r, cd);
	float trailFront = S(-0.02f, 0.02f, st.y - y);
	trail *= trailFront * r * r;

	float trail2 = S(0.2f * r, 0.0f, cd);
	float yy = frac(UV.y * 10.0f) + (st.y - 0.5f);
	float dd = length(st - float2(x, yy));
	float droplets = S(0.3f, 0.0f, dd);
	float m = mainDrop + droplets * r * trailFront;

	return float2(m, trail);
}

float StaticDrops(float2 uv, float time)
{
	uv *= 40.0f;
	float2 id = floor(uv);
	uv = frac(uv) - 0.5f;
	float3 n = N13(id.x * 107.45f + id.y * 3543.654f);
	float2 p = (n.xy - 0.5f) * 0.7f;
	float d = length(uv - p);
	float fade = Saw(0.025f, frac(time + n.z));
	return S(0.3f, 0.0f, d) * frac(n.z * 10.0f) * fade;
}

float2 RainDrops(float2 uv, float time, float rainAmount)
{
	float t = -time;

	float l0 = smoothstep(0.0f, 0.5f, rainAmount);
	float l1 = smoothstep(0.25f, 0.75f, rainAmount);
	float l2 = smoothstep(0.0f, 0.5f, rainAmount);

	float s = StaticDrops(uv, t) * l0;
	float2 m1 = DropLayer(uv, t) * l1;
	float2 m2 = DropLayer(uv * 1.85f, t) * l2;

	float c = s + m1.x + m2.x;
	c = smoothstep(0.3f, 1.0f, c);

	return float2(c, max(m1.y * l0, m2.y * l1));
}

float4 effect_rain(float2 uv)
{
	float rainAmount = saturate(param0);
	float fogBlur = max(param1, 0.0f);
	float time = param2;

	float2 winUV = (uv - win_center) / (win_half_px * texel_size) * 0.5f + 0.5f;
	float aspect = win_half_px.x / max(win_half_px.y, 1.0f);
	winUV.x *= aspect;

	float2 c = RainDrops(winUV, time, rainAmount);

	float2 e = float2(0.002f, 0.0f);
	float cx = RainDrops(winUV + e, time, rainAmount).x;
	float cy = RainDrops(winUV + e.yx, time, rainAmount).x;
	float2 n = float2(cx - c.x, cy - c.x);

	float focus = lerp(fogBlur, 0.0f, smoothstep(0.1f, 0.2f, c.x));
	focus = lerp(focus, fogBlur * 0.5f, c.y);

	float2 refractUV = uv + n * win_half_px * texel_size * 0.5f;

	int R = clamp((int)(focus * 2.0f + 0.5f), 0, 4);
	float3 blurred;

	if (R > 0)
	{
		float3 acc = float3(0, 0, 0);
		float count = 0.0f;
		float spacing = max(focus * 0.5f, 1.0f);
		for (int ky = -R; ky <= R; ++ky)
		for (int kx = -R; kx <= R; ++kx)
		{
			float2 off = float2((float)kx, (float)ky) * spacing * texel_size;
			acc += srgbToLinear3(sampleClamped(refractUV + off).rgb);
			count += 1.0f;
		}
		blurred = linearToSrgb3(acc / count);
	}
	else
	{
		blurred = sampleClamped(refractUV).rgb;
	}

	float3 sharp = sampleClamped(refractUV).rgb;

	float sharpness = smoothstep(0.1f, 0.3f, c.x);
	float4 col;
	col.rgb = lerp(blurred, sharp, sharpness);
	col.a = 1.0f;

	return col;
}
#endif

#if BG_EFFECT == MODE_KALEIDOSCOPE
// ---- Mode 16: Kaleidoscope ----
// param0 = number of mirror segments, param1 = rotation (radians), param2 = time (auto-rotate)
float4 effect_kaleidoscope(float2 uv)
{
	float segments = max(param0, 2.0f);
	float rotation = param1 + param2 * 0.3f;

	float2 centered = (uv - win_center) / (win_half_px * texel_size);

	float ca = cos(rotation), sa = sin(rotation);
	float2 rotated = float2(centered.x * ca + centered.y * sa, -centered.x * sa + centered.y * ca);

	float angle = atan2(rotated.y, rotated.x);
	float radius = length(rotated);

	float segAngle = 2.0f * PI / segments;
	angle = abs(fmod(angle, segAngle) - segAngle * 0.5f);

	float2 mirrored = float2(cos(angle), sin(angle)) * radius;
	float2 sampleUV = mirrored * win_half_px * texel_size + win_center;

	float4 col = sampleClamped(sampleUV);

	float segEdge = abs(fmod(atan2(rotated.y, rotated.x) + PI, segAngle) - segAngle * 0.5f);
	float edgeGlow = pow(saturate(1.0f - segEdge / (segAngle * 0.05f)), 4.0f) * 0.15f;
	col.rgb += edgeGlow;

	return col;
}
#endif

float4 main_ps(PS_INPUT input) : SV_Target
{
	float2 uv = input.uv;
	float4 col;

#if   BG_EFFECT == MODE_GLASS_REFRACT
	col = effect_glass_refract(uv);
#elif BG_EFFECT == MODE_FROSTED_GLASS
	col = effect_frosted(uv);
#elif BG_EFFECT == MODE_PIXELATE
	col = effect_pixelate(uv);
#elif BG_EFFECT == MODE_CHROMATIC
	col = effect_chromatic(uv);
#elif BG_EFFECT == MODE_LIQUID_GLASS
	col = effect_liquid_glass(uv);
#elif BG_EFFECT == MODE_HEAT_HAZE
	col = effect_heat_haze(uv);
#elif BG_EFFECT == MODE_VORONOI
	col = effect_voronoi(uv);
#elif BG_EFFECT == MODE_EDGE_GLOW
	col = effect_edge_glow(uv);
#elif BG_EFFECT == MODE_HALFTONE
	col = effect_halftone(uv);
#elif BG_EFFECT == MODE_MOUSE_EDGE
	col = effect_mouse_edge(uv);
#elif BG_EFFECT == MODE_CRT
	col = effect_crt(uv);
#elif BG_EFFECT == MODE_DOT_MATRIX
	col = effect_dot_matrix(uv);
#elif BG_EFFECT == MODE_GLITCH
	col = effect_glitch(uv);
#elif BG_EFFECT == MODE_STAINED_GLASS
	col = effect_stained_glass(uv);
#elif BG_EFFECT == MODE_RAIN
	col = effect_rain(uv);
#elif BG_EFFECT == MODE_KALEIDOSCOPE
	col = effect_kaleidoscope(uv);
#else
	col = effect_blur(uv);
#endif

	col.a = 1.0f;
	return col * input.col;
}
