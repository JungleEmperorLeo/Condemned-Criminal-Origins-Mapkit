#pragma once
namespace BspNodeTable {
struct Record {int polygon,negative,positive;};
static_assert(sizeof(Record)==12,"BSP node record must be 12 bytes");
// Bounded diagnostic exporter for a convex chain only. Caller supplies leaf
// labels and assumes source polygon IDs are the intended saved polygon IDs.
// Does not infer FEAR leaf semantics or handle general trees/coplanar lists.
inline bool ConvexChain(const BspTree::Branch* root,Record* output,unsigned int capacity,int positiveLeaf,int negativeLeaf,unsigned int& count){
 count=0;if(positiveLeaf>=0||negativeLeaf>=0||positiveLeaf==negativeLeaf)return false;
 for(const BspTree::Branch* b=root;b;b=b->back){
  if(b->front||!b->coplanar||b->coplanar->next||count>=capacity||b->coplanar->fragment.sourcePolygonIndex>0x7fffffff)return false;
  count++;
 }
 unsigned int index=0;for(const BspTree::Branch* b=root;b;b=b->back,index++){
  output[index].polygon=(int)b->coplanar->fragment.sourcePolygonIndex;
  output[index].negative=b->back?(int)(index+1):negativeLeaf;
  output[index].positive=positiveLeaf;
 }return true;
}
}
