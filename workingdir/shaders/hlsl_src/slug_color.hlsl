// slug_color.hlsl — COLR v0 color glyph permutation of the Slug GPU font shader.
// Identical to slug.hlsl except SLUG_COLOR is defined, which changes the PS output
// to preserve the exact palette color RGB and only modulate alpha by coverage.
#define SLUG_COLOR
#include "slug.hlsl"
