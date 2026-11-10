#pragma once
namespace RenderPackedLimits {
// Aggregate allocation budgets are independent of the on-disk local 16-bit
// section index contract. They bound native working slices across all sections.
static const unsigned int MaximumVertices=1000000u;
static const unsigned int MaximumTriangles=1000000u;
static const unsigned int MaximumIndices=MaximumTriangles*3u;
static const unsigned int MaximumVertexBytes=64u*1024u*1024u;
static_assert(MaximumTriangles<=0xffffffffu/12u,"packed triangle input byte bound");
static_assert(MaximumVertices<=0xffffffffu/84u,"packed working vertex byte bound");
static_assert(MaximumIndices<=0xffffffffu/2u,"packed index output byte bound");
static_assert(MaximumVertices*84u+MaximumTriangles*12u<128u*1024u*1024u,"linear packed input workspace bound");
// Reject even an invalid caller accumulation before unsigned subtraction.
inline bool Fits(unsigned int value,unsigned int accumulated,unsigned int maximum){return accumulated<=maximum&&value<=maximum-accumulated;}
}
