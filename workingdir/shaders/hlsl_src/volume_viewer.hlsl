// dear_widgets VolumeViewer -- Texture3D raymarching shader.
//
// Modes:
//   0 = Raymarch DDA  (front-to-back compositing with grayscale transfer fn)
//   1 = MIP           (max intensity projection)
//   2 = Iso-Surface   (march to threshold, shade with gradient normal)
//
// Camera: orbital (azimuth, elevation, distance) around volume center.
// Volume is assumed to occupy [0, 1]^3 in world space. The ray is intersected
// with that AABB, then marched at `step_size` intervals.
//
// The 3D texture is bound at slot 1 via ImPlatform_SetShaderTexture("volume", 1, ...).
// texture0 (ImGui's default) is unused but declared for binding compatibility.

cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjMtx;
};

cbuffer VolumeViewerParams : register(b1)
{
    // Camera orientation as three basis vectors (right, up, forward)
    // packed as row0..row2 of the world-to-camera rotation. forward points from
    // camera toward the volume center.
    float4  camRight;      // .xyz = right basis, .w = aspect (widget_w / widget_h)
    float4  camUp;         // .xyz = up basis, .w = field-of-view tangent
    float4  camForward;    // .xyz = forward basis, .w = camera distance
    float4  camPos;        // .xyz = camera world position, .w = step_size
    float4  winPack;       // (window_min, window_max, gamma, iso_threshold)
    uint4   modePack;      // (mode, ramp, max_steps, 0)
    float4  volDims;       // (W, H, D, 0) — voxel dims
    float4  bgColor;       // background color
};

SamplerState sampler0;
Texture2D    texture0;         // unused (ImGui binds via AddImage)
SamplerState volumeSampler;
Texture3D    volume;           // bound via SetShaderTexture("volume", 1, ...)

struct VS_INPUT { float2 pos : POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };
struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };

PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT o;
    o.pos = mul(ProjMtx, float4(input.pos.xy, 0.0f, 1.0f));
    o.col = input.col;
    o.uv  = input.uv;
    return o;
}

// Ray-AABB intersection. Returns (tmin, tmax); tmin > tmax means no hit.
float2 ray_box(float3 ro, float3 rd, float3 bmin, float3 bmax)
{
    float3 inv = 1.0f / rd;
    float3 t0 = (bmin - ro) * inv;
    float3 t1 = (bmax - ro) * inv;
    float3 tmin3 = min(t0, t1);
    float3 tmax3 = max(t0, t1);
    float tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
    float tmax = min(min(tmax3.x, tmax3.y), tmax3.z);
    return float2(tmin, tmax);
}

float sample_vol(float3 uvw)
{
    // Return the .r channel as scalar.
    return volume.SampleLevel(volumeSampler, uvw, 0).r;
}

float window_map(float v, float wmin, float wmax, float gamma)
{
    float t = saturate((v - wmin) / max(wmax - wmin, 1e-6f));
    return pow(t, 1.0f / max(gamma, 1e-4f));
}

// Ramp: 0=gray, 1=viridis (inlined cubic)
float3 apply_ramp(float t, uint mode)
{
    if (mode == 1u)
    {
        t = saturate(t);
        float3 c = float3(0.267f, 0.005f, 0.329f)
                 + t * (float3( 0.150f, 1.390f, 0.800f)
                 + t * (float3( 2.400f,-0.700f,-2.200f)
                 + t * (float3(-1.900f, 0.450f, 1.500f))));
        return saturate(c);
    }
    return float3(t, t, t);
}

// Estimate gradient via central differences for iso-surface shading.
float3 gradient(float3 uvw, float3 inv_dims)
{
    float gx = sample_vol(uvw + float3(inv_dims.x, 0, 0)) - sample_vol(uvw - float3(inv_dims.x, 0, 0));
    float gy = sample_vol(uvw + float3(0, inv_dims.y, 0)) - sample_vol(uvw - float3(0, inv_dims.y, 0));
    float gz = sample_vol(uvw + float3(0, 0, inv_dims.z)) - sample_vol(uvw - float3(0, 0, inv_dims.z));
    return float3(gx, gy, gz);
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    // Build ray in world space from UV (centered in [-1,1]).
    float2 ndc = input.uv * 2.0f - 1.0f;
    ndc.x *= camRight.w;          // aspect
    // Flip Y: ImGui UV goes top->bottom (y=0 at the top), but the camera's
    // camUp axis should map to the TOP of the screen so ndc.y must grow
    // upward. Without this the volume renders vertically mirrored.
    ndc.y = -ndc.y;
    float fov_tan = camUp.w;
    float3 ro = camPos.xyz;
    float3 rd = normalize(camForward.xyz + camRight.xyz * ndc.x * fov_tan
                                         + camUp.xyz       * ndc.y * fov_tan);

    // Intersect with volume unit box [0,1]^3.
    float2 t = ray_box(ro, rd, float3(0, 0, 0), float3(1, 1, 1));
    if (t.x > t.y || t.y < 0.0f)
        return bgColor * input.col;

    float ts = max(t.x, 0.0f);
    float te = t.y;
    float step = max(camPos.w, 1e-4f);
    uint max_steps = max(modePack.z, 1u);
    uint n = min(max_steps, (uint)((te - ts) / step + 1.0f));

    uint mode = modePack.x;
    uint ramp = modePack.y;
    float wmin = winPack.x;
    float wmax = winPack.y;
    float gamma = winPack.z;
    float iso  = winPack.w;
    float3 inv_dims = 1.0f / max(volDims.xyz, float3(1, 1, 1));

    float3 final_rgb = bgColor.rgb;
    float  final_a   = 0.0f;

    if (mode == 1u)  // MIP
    {
        float m = 0.0f;
        for (uint i = 0u; i < n; ++i)
        {
            float s = ts + (float)i * step;
            float v = sample_vol(ro + rd * s);
            m = max(m, v);
        }
        float t01 = window_map(m, wmin, wmax, gamma);
        final_rgb = apply_ramp(t01, ramp);
        final_a = 1.0f;
    }
    else if (mode == 2u)  // Iso-Surface
    {
        float prev = sample_vol(ro + rd * ts);
        bool hit = false;
        float3 hit_uvw = float3(0, 0, 0);
        for (uint i = 1u; i < n; ++i)
        {
            float s = ts + (float)i * step;
            float3 uvw = ro + rd * s;
            float v = sample_vol(uvw);
            if ((prev - iso) * (v - iso) < 0.0f)
            {
                float tInt = (iso - prev) / max(v - prev, 1e-6f);
                hit_uvw = ro + rd * (s - step + tInt * step);
                hit = true;
                break;
            }
            prev = v;
        }
        if (hit)
        {
            float3 g = gradient(hit_uvw, inv_dims);
            float gl = length(g);
            float3 nrm = (gl > 1e-6f) ? (g / gl) : float3(0, 1, 0);
            // Flip so the normal faces the ray.
            if (dot(nrm, rd) > 0.0f) nrm = -nrm;
            float diff = max(dot(nrm, normalize(float3(0.4f, 0.7f, 0.5f))), 0.0f);
            float3 base = apply_ramp(window_map(sample_vol(hit_uvw), wmin, wmax, gamma), ramp);
            final_rgb = base * (0.2f + 0.8f * diff);
            final_a = 1.0f;
        }
    }
    else  // Raymarch DDA front-to-back compositing
    {
        float3 acc_rgb = float3(0, 0, 0);
        float  acc_a   = 0.0f;
        for (uint i = 0u; i < n; ++i)
        {
            float s = ts + (float)i * step;
            float v = sample_vol(ro + rd * s);
            float t01 = window_map(v, wmin, wmax, gamma);
            float3 c = apply_ramp(t01, ramp);
            float a = t01 * step * 4.0f;    // density proportional to sample
            a = saturate(a);
            acc_rgb += (1.0f - acc_a) * c * a;
            acc_a   += (1.0f - acc_a) * a;
            if (acc_a > 0.99f) break;
        }
        final_rgb = acc_rgb + bgColor.rgb * (1.0f - acc_a);
        final_a = 1.0f;
    }

    return float4(final_rgb, final_a) * input.col;
}
