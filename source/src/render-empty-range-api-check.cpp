#include "WorldPacker.cpp"
#include "RenderPackedRanges.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
// Exercise actual NULL optional pointers through the public range API rather
// than checker-owned dummy allocations. No original module is loaded.
extern "C" void HarnessEntry(){
 RenderPackedRanges::Builder builder;
 if(!builder.Build(0,0,false)||builder.count||builder.materialCount||builder.declarationCount||builder.vertexBytes||builder.indexCount||builder.error)ExitProcess(1);
 if(!builder.Build(0,0,true)||builder.count||builder.error)ExitProcess(2);
 if(builder.Build(0,1,false)||!builder.error)ExitProcess(3);
 const RenderVertexPacking::Element decl[]={{0,0,2,0,0,0},{255,0,17,0,0,0}};
 RenderPackedRanges::Input input={"Empty",0,0,decl,2,0,0,0,0,0};
 if(!builder.Build(&input,1,false)||builder.count!=1||builder.materialCount!=1||builder.declarationCount!=1||builder.vertexBytes||builder.indexCount||builder.ranges[0].vertexCount||builder.ranges[0].triangles||builder.error)ExitProcess(4);
 RenderAttributes::Vertex vertex={};input.vertices=&vertex;input.vertexCount=1;
 if(!builder.Build(&input,1,false)||builder.vertexBytes!=12||builder.indexCount||builder.error)ExitProcess(5);
 input.triangles=1;if(builder.Build(&input,1,false)||!builder.error)ExitProcess(6);
 input.triangles=0;input.vertices=0;if(builder.Build(&input,1,false)||!builder.error)ExitProcess(7);
 ExitProcess(0);
}
