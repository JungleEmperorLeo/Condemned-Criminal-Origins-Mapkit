#pragma once
namespace RenderFrameLimits {
// A bounded source endpoint budget shared by frame preparation and its sort
// and optional position prepass. One million endpoints reserve 404 MB for
// input, sort records, working vertices and frames, plus 12 MB of snap points.
// Counts/byte spans remain representable in the 32-bit harness and arenas.
static const unsigned int MaximumEndpoints=1000000u;
static_assert(MaximumEndpoints<=0xffffffffu/208u,"source endpoint input byte bound");
static_assert(MaximumEndpoints<=0x7fffffffu/72u,"frame sort signed pointer-distance bound");
static_assert(MaximumEndpoints<=0x7fffffffu/(208u+72u+84u+40u+12u),"linear frame workspace byte bound");
}
