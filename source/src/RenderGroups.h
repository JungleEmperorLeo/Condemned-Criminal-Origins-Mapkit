#pragma once
// Group already-generated face vertices by explicit world/render tree and exact material.
// Full attributes are preserved; smoothing and world-model resolution belong to callers.
namespace RenderGroups {
struct FaceInput {const RenderAttributes::Vertex* vertices;unsigned int count;const unsigned short* indices;unsigned int triangles;const char* material;unsigned int tree;};
class Groups {
 bool Fail(const char* message){memory.Clear();surfaces=0;count=0;error=message;return false;}
public:
 PrefabScene::Arena memory;RenderBlock::SurfaceInput* surfaces;unsigned int count;const char* error;
 Groups():surfaces(0),count(0),error(0){}
 bool Build(const FaceInput* faces,unsigned int faceCount){
  memory.Clear();surfaces=0;count=0;error=0;if(!faces||!faceCount||faceCount>4096)return Fail("Unsupported render face count");
  surfaces=(RenderBlock::SurfaceInput*)memory.Allocate(faceCount*sizeof(RenderBlock::SurfaceInput));unsigned int* owners=(unsigned int*)memory.Allocate(faceCount*4);unsigned int* capacities=(unsigned int*)memory.Allocate(faceCount*4);unsigned int* triangleCap=(unsigned int*)memory.Allocate(faceCount*4);
  if(!surfaces||!owners||!capacities||!triangleCap)return Fail("Render group allocation failed");for(unsigned int i=0;i<faceCount;i++){capacities[i]=triangleCap[i]=0;}
  for(unsigned int i=0;i<faceCount;i++){
   const FaceInput& f=faces[i];if(!f.vertices||!f.indices||!f.count||f.count>65535||!f.triangles||f.triangles>1000000||!f.material||!f.material[0])return Fail("Invalid render face input");
   unsigned int length=0;while(f.material[length]&&length<=4096)length++;if(length>4096)return Fail("Material name too long");
   unsigned int group=0;for(;group<count;group++)if(surfaces[group].tree==f.tree&&RenderBlock::SameMaterial(surfaces[group].material,f.material))break;
   if(group==count){char* name=(char*)memory.Allocate(length+1);if(!name)return Fail("Render material allocation failed");for(unsigned int k=0;k<=length;k++)name[k]=f.material[k];surfaces[group].material=name;surfaces[group].tree=f.tree;count++;}
   if(f.count>65535-capacities[group]||f.triangles>1000000-triangleCap[group])return Fail("Render group capacity exceeded");capacities[group]+=f.count;triangleCap[group]+=f.triangles;owners[i]=group;
   for(unsigned int j=0;j<f.triangles*3;j++)if(f.indices[j]>=f.count)return Fail("Render face index out of range");
  }
  unsigned int totalBytes=0;for(unsigned int g=0;g<count;g++){unsigned int bytes=capacities[g]*84+triangleCap[g]*6;if(bytes>32*1024*1024-totalBytes)return Fail("Render groups too large");totalBytes+=bytes;surfaces[g].vertices=memory.Allocate(capacities[g]*84);surfaces[g].indices=(unsigned short*)memory.Allocate(triangleCap[g]*6);surfaces[g].count=surfaces[g].triangles=0;if(!surfaces[g].vertices||!surfaces[g].indices)return Fail("Render group buffer allocation failed");}
  for(unsigned int i=0;i<faceCount;i++){
   const FaceInput& f=faces[i];RenderBlock::SurfaceInput& s=surfaces[owners[i]];RenderAttributes::Vertex* vs=(RenderAttributes::Vertex*)s.vertices;unsigned short* ids=(unsigned short*)s.indices;unsigned short* remap=(unsigned short*)memory.Allocate(f.count*2);if(!remap)return Fail("Render remap allocation failed");
   for(unsigned int v=0;v<f.count;v++){
    const unsigned char* source=(const unsigned char*)&f.vertices[v];unsigned int found=0;
    for(;found<s.count;found++){const unsigned char* target=(const unsigned char*)&vs[found];unsigned int k=0;while(k<84&&source[k]==target[k])k++;if(k==84)break;}
    if(found==s.count)vs[s.count++]=f.vertices[v];remap[v]=(unsigned short)found;
   }
   for(unsigned int t=0;t<f.triangles;t++){for(unsigned int k=0;k<3;k++)ids[s.triangles*3+k]=remap[f.indices[t*3+k]];s.triangles++;}
  }
  return true;
 }
};
}
