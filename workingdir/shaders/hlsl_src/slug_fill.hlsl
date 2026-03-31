// slug_fill.hlsl — GPU gradient fill permutation of the Slug GPU font shader.
// Uses standard SlugVertex (80 bytes) + pixel constant buffer for gradient params.
// The pixel shader computes gradient color from screen-space position and text bbox.
#define SLUG_FILL
#include "slug.hlsl"
