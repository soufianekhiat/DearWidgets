// slug_debug.hlsl — Debug variant: visualize xcov/ycov/coverage as RGB
// R = |xcov| (horizontal winding), G = |ycov| (vertical winding), B = coverage
// Yellow = both non-zero (correct interior), Red-only = X winding leak, Green-only = Y winding leak
#define SLUG_DEBUG
#include "slug.hlsl"
