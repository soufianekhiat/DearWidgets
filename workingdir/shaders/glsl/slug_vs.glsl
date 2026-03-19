// Slug GPU Font Rendering - Vertex Shader (GLSL)
// Based on the Slug Algorithm by Eric Lengyel (public domain, 2026)
// https://sluglibrary.com
//
// Cross-platform design: uses ImDrawVert vertex format (Position, UV, Color).
// Per-glyph Slug data is passed via named uniforms (slugBandLoc, slugBanding).
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

// ImGui standard projection matrix (set by ImGui OpenGL3 backend)
uniform mat4 ProjMtx;

// Inputs matching ImGui's ImDrawVert layout:
//   location 0 = Position (float2)
//   location 1 = UV       (float2, carries em-space render coordinate)
//   location 2 = Color    (ubyte4 UNORM → vec4)
layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec4 Color;

// Outputs to pixel shader
layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_renderCoord;  // em-space coordinate, interpolated

void main()
{
    v_color       = Color;
    v_renderCoord = UV;  // UV carries em-space coords set by AddImageQuad
    gl_Position   = ProjMtx * vec4(Position, 0.0, 1.0);
}
