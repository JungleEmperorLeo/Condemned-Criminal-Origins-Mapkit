#pragma once
#include "WorldCenter.h"
#include "SourceSurfacePreparation.h"
namespace SourcePreparedSurfaceOutput {
struct Stats {unsigned int surfaces,fragments,rejected;};
inline bool Put(HANDLE file,const void* data,unsigned int bytes){DWORD done;return WriteFile(file,data,bytes,&done,0)&&done==bytes;}
// Intermediate surface stream, not World00p serialization. Provenance is retained.
inline bool Save(const PrefabScene::Model& model,const char* path,Stats& stats){
 stats.surfaces=stats.fragments=stats.rejected=0;WorldText::Vec3 offset;
 if(!WorldCenter::Compute(model,offset))return false;
 HANDLE file=CreateFileA(path,0x40000000,FILE_SHARE_READ,0,2,FILE_ATTRIBUTE_NORMAL,0);
 if(file==INVALID_HANDLE_VALUE)return false;bool ok=Put(file,&offset,12);
 for(const PrefabScene::Node* node=model.first;node&&ok;node=node->next)if(node->kind==1){
  for(unsigned int i=0;i<node->geometry.polygons&&ok;i++){
   SourceSurfacePreparation::Surface surface;if(!SourceSurfacePreparation::Build(*node,i,offset,surface)){stats.rejected++;continue;}
   stats.surfaces++;const WorldText::Polygon& polygon=node->geometry.faces[i];
   for(unsigned int ordinal=0;ordinal<SourceSurfacePreparation::FragmentCount(polygon.count)&&ok;ordinal++){
    SourceSurfacePreparation::Fragment fragment;if(!SourceSurfacePreparation::FragmentAt(polygon,ordinal,fragment)){ok=false;break;}
    unsigned int ids[3]={node->index,i,node->surfaceFlags};
    ok=Put(file,ids,12)&&Put(file,surface.bases,144)&&Put(file,&fragment.count,4);
    for(unsigned int j=0;j<fragment.count&&ok;j++){
     WorldText::Vec3 point=node->geometry.vertices[fragment.indices[j]];point.x-=offset.x;point.y-=offset.y;point.z-=offset.z;ok=Put(file,&point,12);
    }if(ok)stats.fragments++;
   }
  }
 }CloseHandle(file);return ok;
}
}
