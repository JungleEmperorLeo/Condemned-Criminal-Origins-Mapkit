#pragma once
#include "RenderFrameLimits.h"
namespace RenderIndexedLimits {
// A node can retain as many corners as the shared frame endpoint stage.
// This allocation budget does not widen any section's 16-bit index format.
static const unsigned int MaximumCorners=RenderFrameLimits::MaximumEndpoints;
static const unsigned int MaximumTriangles=MaximumCorners/3u;
static_assert(MaximumCorners<=0xffffffffu/88u,"indexed corner input byte bound");
static_assert(MaximumTriangles<=0xffffffffu/260u,"indexed triangle input byte bound");
static_assert(MaximumTriangles*(260u+40u+4u+4u+4u+12u)+MaximumCorners*(88u+84u+4u+4u)<512u*1024u*1024u,"linear indexed node workspace byte bound");
}
