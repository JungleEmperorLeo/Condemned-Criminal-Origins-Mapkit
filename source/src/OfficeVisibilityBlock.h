#pragma once
#include "OfficeSectorPortals.h"
#include "PortalRecord.h"
#include "SectorOutlines.h"
namespace OfficeVisibilityBlock {
// Variable portal contours and recovered all-or-zero sector outlines.
class Rectangular {
 PrefabScene::Arena memory;
 void U32(unsigned char*& p,unsigned int v){*(unsigned int*)p=v;p+=4;}
 void U16(unsigned char*& p,unsigned short v){p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);p+=2;}
 void Copy(unsigned char*& p,const void* sourceBytes,unsigned int n){const unsigned char* s=(const unsigned char*)sourceBytes;for(unsigned int i=0;i<n;i++)*p++=s[i];}
public:
 VisibilityTree::Builder tree;OfficeSectorPortals::AxisAligned portalBuilder;unsigned char* data;unsigned int size;const char* error;
 Rectangular():data(0),size(0),error(0){}
 bool Build(const StreamingSplit::Bounds* bounds,const char* const* names,unsigned int count,const OfficeSectorPortals::PreparedSector* prepared=0){
  memory.Clear();data=0;size=0;error=0;if(!names||!tree.Build(bounds,count)||!portalBuilder.Build(bounds,count,tree.saveIds,prepared))return false;
  unsigned int namePool=0,nameBytes=0;for(unsigned int i=0;i<count;i++){if(!names[i])return false;unsigned int n=lstrlenA(names[i]);if(n>4095)return false;nameBytes+=n;namePool+=(n+4)&~3u;}
  const unsigned int* p=(const unsigned int*)tree.data;unsigned int* depth=(unsigned int*)memory.Allocate(tree.nodeCount*4);int* children=(int*)memory.Allocate(tree.nodeCount*8);if(!depth||!children)return false;
  for(unsigned int i=0;i<tree.nodeCount;i++){unsigned int n=*p++;p+=n+2;children[i*2]=(int)*p++;children[i*2+1]=(int)*p++;}
  for(unsigned int i=tree.nodeCount;i-->0;){depth[i]=1;for(unsigned int k=0;k<2;k++){int child=children[i*2+k];if(child>=0){if((unsigned int)child<=i||(unsigned int)child>=tree.nodeCount)return false;if(depth[child]+1>depth[i])depth[i]=depth[child]+1;}}}
  unsigned int portalVertices=0;for(unsigned int i=0;i<portalBuilder.count;i++)portalVertices+=portalBuilder.portals[i].vertexCount;
  unsigned int outlinePlanes=0;for(unsigned int i=0;i<count;i++)outlinePlanes+=SectorOutlines::Count(prepared[i].faces,prepared[i].count);
  size=outlinePlanes*16+32+portalBuilder.count*34+portalVertices*12+count*38+nameBytes+portalBuilder.count*8+tree.size;if(size>16*1024*1024)return false;data=(unsigned char*)memory.Allocate(size);if(!data)return false;unsigned char* out=data;
  unsigned int header[]={count,portalBuilder.count,tree.nodeCount,depth[0],portalVertices,namePool,outlinePlanes,portalBuilder.count*2};Copy(out,header,32);
  for(unsigned int i=0;i<portalBuilder.count;i++){const OfficeSectorPortals::Portal& v=portalBuilder.portals[i];unsigned int bytes;if(!PortalRecord::Write(out,size-(unsigned int)(out-data),v.vertices,v.vertexCount,v.plane,bytes))return false;out+=bytes;}
  for(unsigned int id=0;id<count;id++){unsigned int source=tree.order[id],n=lstrlenA(names[source]);U16(out,(unsigned short)n);Copy(out,names[source],n);Copy(out,&bounds[source],24);unsigned int outlines=SectorOutlines::Count(prepared[source].faces,prepared[source].count);U32(out,outlines);for(unsigned int k=0;k<outlines;k++)Copy(out,&prepared[source].faces[k].plane,16);unsigned int refs=0;for(unsigned int j=0;j<portalBuilder.count;j++)if(portalBuilder.portals[j].sectorA==id||portalBuilder.portals[j].sectorB==id)refs++;U32(out,refs);for(unsigned int j=0;j<portalBuilder.count;j++)if(portalBuilder.portals[j].sectorA==id||portalBuilder.portals[j].sectorB==id)U32(out,j);U32(out,id);}
  Copy(out,tree.data,tree.size);return (unsigned int)(out-data)==size;
 }
};
}



