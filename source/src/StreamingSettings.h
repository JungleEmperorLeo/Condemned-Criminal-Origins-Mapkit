#pragma once
namespace StreamingSettings {
// Region names/budgets and StreamingInfo budgets of the original stage
// (StreamingStructure.h): ignored and removed regions are absent, budgets of a
// rejected info stay as far as they were read, the info position defaults to the
// centered origin.
class Settings {
 PrefabScene::Arena memory;
public:
 const char** names;unsigned int* budgets;unsigned int count,globalBudget,streamingBudget;const PrefabScene::Node* info;
 Settings():names(0),budgets(0),count(0),globalBudget(0),streamingBudget(0),info(0){}
 bool Read(const PrefabScene::Model& source){
  memory.Clear();count=globalBudget=streamingBudget=0;info=0;names=(const char**)memory.Allocate((source.count+1)*sizeof(char*));budgets=(unsigned int*)memory.Allocate((source.count+1)*4);if(!names||!budgets)return false;
  WorldText::Vec3 zero={0,0,0};StreamingStructure::Builder structure;if(!structure.Build(source,zero)||structure.fatal)return false;
  for(unsigned int r=0;r<structure.regionCount;r++)if(!structure.regions[r].removed){names[count]=structure.regions[r].node->name;budgets[count]=structure.regions[r].budget;count++;}
  globalBudget=structure.globalBudget;streamingBudget=structure.streamingBudget;info=structure.info;return true;
 }
 // 0x10034480 over the regions' own brushes: the first volume holding the info
 // position (the centered origin without a valid info).
 int Select(const StreamingConvex::Block& convex)const{
  WorldText::Vec3 position={0,0,0};if(info){position=info->position;position.x-=convex.offset.x;position.y-=convex.offset.y;position.z-=convex.offset.z;}
  const unsigned int* cursor=(const unsigned int*)convex.data;
  for(unsigned int i=0;i<convex.volumeCount;i++){
   cursor++;cursor++;unsigned int faces=*cursor++;const ExtrusionVolume::Plane* planes=(const ExtrusionVolume::Plane*)cursor;cursor+=4*faces;bool inside=convex.volumes[i].linked<0&&!convex.volumes[i].moved;
   for(unsigned int j=0;j<faces;j++)if(ExtrusionVolume::Distance(planes[j],position)<0)inside=false;
   for(unsigned int j=0;j<faces;j++){unsigned int points=*cursor++;cursor+=points*3;}
   if(inside)return (int)i;
  }return -1;
 }
};
}
