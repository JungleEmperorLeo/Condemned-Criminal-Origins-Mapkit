#pragma once
#include "ExtrusionSequence.h"
#include "OfficeVisibilityBlock.h"
#include "WorldCenter.h"
#include "StreamingMetadata.h"
namespace VisibilityCompiler {
// Packer-log notes of the original sector builder, in processing order (the
// caller formats them; severity: kinds 7 and 8 are errors, the rest warnings):
//  0 polygon not usable (sector, centered midpoint, 0x1002EBAE), skipped;
//  1 concave sector (0x1002EA35) and 2 open sector (0x1002EC4E), skipped;
//  3 extrusion brush without exactly one polygon (0x100309F3), 4 invalid
//  extrusion polygon (0x10030B08), 5 extrusion that hits no sector on a side
//  (0x10031CE3), 6 sector intersecting an extrusion (first sector, second
//  extrusion) - each extrusion is ignored;
//  7 two intersecting sectors (0x1002DCA5), 8 their pair count (0x1002DCED):
//  fatal, the caller adds "Error building visibility".
enum NoteKind {UnusablePolygon,ConcaveSector,OpenSector,ExtrusionBrush,ExtrusionPolygon,ExtrusionMissed,ExtrusionIntersects,SectorsIntersect,IntersectingSets,InvalidPortal};
struct Note {unsigned int kind,number;const char* first;const char* second;WorldText::Vec3 midpoint;};
// Owns all prepared geometry and serialized bytes. No DLL calls, process exits,
// environment variables or file operations; the caller controls input/output.
class Builder {
 PrefabScene::Arena memory;
 OfficeVisibilityBlock::Rectangular block;
 bool Fail(unsigned int code,const char* name=0){sourceModel=0;errorCode=code;errorSector=name;return false;}
 Builder(const Builder&);Builder& operator=(const Builder&);
public:
 unsigned char* data;unsigned int size,errorCode,sectorCount,acceptedExtrusions,skippedExtrusions;
 // Packer log: for each skipped extrusion, the intersected sector and the extrusion.
 const char** skippedSector;const char** skippedExtrusion;
 const char* errorSector;WorldText::Vec3 offset;
 // Identity only: the successful caller-owned model must remain alive and
 // unchanged while another source stage consumes this prepared visibility.
 const PrefabScene::Model* sourceModel;
 const StreamingMetadata::External* streamingSectors;
 // Construction-order source node of each sector: Sector brushes, then accepted extrusions.
 const unsigned int* sectorSources;
 Note* notes;unsigned int noteCount;bool intersecting; // intersecting: fatal sector intersections (code 16)
 Builder():data(0),size(0),errorCode(0),sectorCount(0),acceptedExtrusions(0),skippedExtrusions(0),skippedSector(0),skippedExtrusion(0),errorSector(0),sourceModel(0),streamingSectors(0),sectorSources(0),notes(0),noteCount(0),intersecting(false),noteCapacity(0){offset.x=offset.y=offset.z=0;}
private:
 unsigned int noteCapacity;
 void Add(unsigned int kind,const char* first,const char* second=0,const WorldText::Vec3* midpoint=0,unsigned int number=0){if(!notes||noteCount>=noteCapacity)return;Note& n=notes[noteCount++];n.kind=kind;n.number=number;n.first=first;n.second=second;n.midpoint.x=n.midpoint.y=n.midpoint.z=0;if(midpoint)n.midpoint=*midpoint;}
public:
 bool Build(const PrefabScene::Model& model,const WorldText::Vec3* suppliedOffset=0){
  data=0;streamingSectors=0;sectorSources=0;sourceModel=0;size=errorCode=sectorCount=acceptedExtrusions=skippedExtrusions=0;skippedSector=skippedExtrusion=0;errorSector=0;notes=0;noteCount=noteCapacity=0;intersecting=false;memory.Clear();
  if(suppliedOffset){if(!WorldText::FiniteVector(*suppliedOffset))return Fail(2);offset=*suppliedOffset;}else if(!WorldCenter::Compute(model,offset))return Fail(41);
  unsigned int candidateCount=0,apertureCount=0,sectorBrushes=0;
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1){if(FearBrushFlags::Equal(n->type,"Sector")){candidateCount+=n->geometry.polygons;sectorBrushes++;}if(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))apertureCount++;}
  if(candidateCount>8192||apertureCount>100000)return Fail(5);
  noteCapacity=candidateCount+sectorBrushes+apertureCount+65536;notes=(Note*)memory.Allocate(noteCapacity*sizeof(Note));
  ExtrusionVolume::Candidate* candidates=(ExtrusionVolume::Candidate*)memory.Allocate((candidateCount+apertureCount*130+1)*sizeof(ExtrusionVolume::Candidate));
  const char** names=(const char**)memory.Allocate((sectorBrushes+apertureCount+1)*sizeof(char*));
  ExtrusionSequence::Existing* existing=(ExtrusionSequence::Existing*)memory.Allocate((sectorBrushes+apertureCount+1)*sizeof(ExtrusionSequence::Existing));
  unsigned int* sources=(unsigned int*)memory.Allocate((sectorBrushes+apertureCount+1)*sizeof(unsigned int));
  skippedSector=(const char**)memory.Allocate((apertureCount+1)*sizeof(char*));skippedExtrusion=(const char**)memory.Allocate((apertureCount+1)*sizeof(char*));
  if(!notes||!candidates||!names||!existing||!sources||!skippedSector||!skippedExtrusion)return Fail(6);unsigned int index=0,existingCount=0;
  // Sector brushes (0x1002E9CC): each unusable polygon is skipped, then a sector
  // that is concave (after one flip) or open (0.1) is skipped.
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&FearBrushFlags::Equal(n->type,"Sector")){
   const unsigned int first=index;
   for(unsigned int j=0;j<n->geometry.polygons;j++){
    const WorldText::Polygon& poly=n->geometry.faces[j];if(poly.count<3||poly.count>128)return Fail(7,n->name);
    WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(poly.count*12);if(!points)return Fail(6,n->name);
    for(unsigned int k=0;k<poly.count;k++){if(poly.indices[k]>=n->geometry.points)return Fail(8,n->name);const WorldText::Vec3& p=n->geometry.vertices[poly.indices[k]];points[k].x=p.x-offset.x;points[k].y=p.y-offset.y;points[k].z=p.z-offset.z;}
    volatile float mx=0,my=0,mz=0;for(unsigned int k=0;k<poly.count;k++){mx=mx+points[k].x;my=my+points[k].y;mz=mz+points[k].z;}WorldText::Vec3 mid={mx/(float)poly.count,my/(float)poly.count,mz/(float)poly.count};
    ExtrusionVolume::Candidate& c=candidates[index];c.points=points;c.count=poly.count;if(!PortalContour::Clean(points,c.count,c.plane)){Add(UnusablePolygon,n->name,0,&mid);continue;}index++;
   }
   const unsigned int faceCount=index-first;
   SectorOrientation::Face* faces=(SectorOrientation::Face*)memory.Allocate((faceCount+1)*sizeof(SectorOrientation::Face));if(!faces)return Fail(6,n->name);
   for(unsigned int j=0;j<faceCount;j++){const ExtrusionVolume::Candidate& c=candidates[first+j];faces[j].points=(WorldText::Vec3*)c.points;faces[j].count=c.count;faces[j].plane=c.plane;}
   // A skipped sector is destroyed: its polygons are no extrusion hit candidates
   // (the extrusion pass walks the sector list, 0x10030E1B).
   bool flipped;SectorPreparation::Failure failure;if(!SectorPreparation::Prepare(faces,faceCount,flipped,failure)){Add(failure==SectorPreparation::InvalidOrientation?ConcaveSector:OpenSector,n->name);index=first;continue;}
   for(unsigned int j=0;j<faceCount;j++){candidates[first+j].count=faces[j].count;candidates[first+j].plane=faces[j].plane;}
   names[existingCount]=n->name;sources[existingCount]=n->index;if(!ExtrusionSequence::Store(memory,candidates+first,faceCount,existing[existingCount++]))return Fail(13,n->name);
  }
  candidateCount=index;
  for(const PrefabScene::Node* n=model.first;n;n=n->next)if(n->kind==1&&(FearBrushFlags::Equal(n->type,"ExtrudeSector")||FearBrushFlags::Equal(n->type,"SemiExtrudeSector"))){
   if(n->geometry.polygons!=1){Add(ExtrusionBrush,n->name);continue;}const WorldText::Polygon& poly=n->geometry.faces[0];if(poly.count<3||poly.count>128)return Fail(7,n->name);
   WorldText::Vec3* points=(WorldText::Vec3*)memory.Allocate(poly.count*12);if(!points)return Fail(6,n->name);
   for(unsigned int k=0;k<poly.count;k++){if(poly.indices[k]>=n->geometry.points)return Fail(8,n->name);const WorldText::Vec3& p=n->geometry.vertices[poly.indices[k]];points[k].x=p.x-offset.x;points[k].y=p.y-offset.y;points[k].z=p.z-offset.z;}
   unsigned int count=poly.count;ExtrusionVolume::Plane plane;if(!PortalContour::Clean(points,count,plane)){Add(ExtrusionPolygon,n->name);continue;}WorldText::Vec3 normal={plane.x,plane.y,plane.z};bool skip;unsigned int hit=0;
   if(!ExtrusionSequence::ShouldSkip(points,count,normal,existing,existingCount,skip,&hit))return Fail(14,n->name);if(skip){skippedSector[skippedExtrusions]=names[hit];skippedExtrusion[skippedExtrusions]=n->name;skippedExtrusions++;Add(ExtrusionIntersects,names[hit],n->name);continue;}
   // 0x10031103: a SemiExtrudeSector ends at its own polygon (negative distance 0),
   // so only the positive side must hit a sector; an ExtrudeSector needs both.
   const bool full=FearBrushFlags::Equal(n->type,"ExtrudeSector");
   ExtrusionVolume::Ends ends={};if(!ExtrusionVolume::SelectEnds(points,count,normal,candidates,candidateCount,ends)||!ends.hasPositive||(full&&!ends.hasNegative)){Add(ExtrusionMissed,n->name);continue;}
   ExtrusionPrism::Volume volume;if(!ExtrusionPrism::Build(memory,points,count,plane,ends,full,volume))return Fail(12,n->name);
   unsigned int firstNewFace=candidateCount;for(unsigned int j=0;j<volume.count;j++){const SectorOrientation::Face& face=volume.faces[j];ExtrusionVolume::Candidate& c=candidates[candidateCount++];c.points=face.points;c.count=face.count;c.plane=face.plane;}
   names[existingCount]=n->name;sources[existingCount]=n->index;if(!ExtrusionSequence::Store(memory,candidates+firstNewFace,volume.count,existing[existingCount++]))return Fail(13,n->name);acceptedExtrusions++;
  }
  // 0x1002DB30: every intersecting pair (i<j, -0.1) is an error; any stops visibility.
  unsigned int pairs=0;for(unsigned int i=0;i<existingCount;i++)for(unsigned int j=i+1;j<existingCount;j++){bool hit;if(!ConvexIntersection::Test(existing[i].convex,existing[j].convex,-0.1f,hit))return Fail(15);if(hit){Add(SectorsIntersect,names[i],names[j]);pairs++;}}
  if(pairs){Add(IntersectingSets,0,0,0,pairs);intersecting=true;return Fail(16);}
  if(!existingCount){
   // No sector survives: the original writes a 32-byte zero visibility header.
   data=(unsigned char*)memory.Allocate(32);if(!data)return Fail(6);for(unsigned int i=0;i<32;i++)data[i]=0;size=32;sectorCount=0;streamingSectors=0;sectorSources=sources;sourceModel=&model;return true;
  }
  StreamingSplit::Bounds* bounds=(StreamingSplit::Bounds*)memory.Allocate(existingCount*sizeof(StreamingSplit::Bounds));OfficeSectorPortals::PreparedSector* prepared=(OfficeSectorPortals::PreparedSector*)memory.Allocate(existingCount*sizeof(OfficeSectorPortals::PreparedSector));if(!bounds||!prepared)return Fail(6);
  for(unsigned int i=0;i<existingCount;i++){
   const ConvexIntersection::Volume& v=existing[i].convex;prepared[i].count=v.count;prepared[i].faces=(SectorOrientation::Face*)memory.Allocate(v.count*sizeof(SectorOrientation::Face));if(!prepared[i].faces)return Fail(6);
   for(unsigned int a=0;a<3;a++){bounds[i].low[a]=(&v.low.x)[a];bounds[i].high[a]=(&v.high.x)[a];}
   for(unsigned int j=0;j<v.count;j++){SectorOrientation::Face& face=prepared[i].faces[j];face.count=v.faces[j].count;face.points=(WorldText::Vec3*)v.faces[j].points;face.plane=v.faces[j].plane;}
  }
  if(!block.Build(bounds,names,existingCount,prepared))return Fail(30);
  // 9: a generated portal fails the polygon check (0x1003053B, owner sector, other sector).
  for(unsigned int k=0;k<block.portalBuilder.invalidCount;k++)Add(InvalidPortal,names[block.portalBuilder.invalidOwner[k]],names[block.portalBuilder.invalidOther[k]]);
  StreamingMetadata::External* external=(StreamingMetadata::External*)memory.Allocate(existingCount*sizeof(StreamingMetadata::External));if(!external)return Fail(6);
  // Preserve construction order for first-owner assignment, but reference the
  // final serialized ID after visibility-tree reordering.
  for(unsigned int i=0;i<existingCount;i++){external[i].volume=existing[i].convex;external[i].index=block.tree.saveIds[i];}
  streamingSectors=external;sectorSources=sources;data=block.data;size=block.size;sectorCount=existingCount;sourceModel=&model;return true;
 }
};
}
