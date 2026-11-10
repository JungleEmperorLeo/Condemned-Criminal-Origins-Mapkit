#pragma once
#include "SectorPreparation.h"
namespace ExtrusionPrism {
// Source vertices are projected along the aperture normal onto each cap plane.
// Float stores mirror projection blocks at RVA 0x31400 and 0x314D7.
struct Volume {SectorOrientation::Face* faces;unsigned int count;};
inline bool Project(const WorldText::Vec3& point,const WorldText::Vec3& direction,const ExtrusionVolume::Plane& cap,bool positive,WorldText::Vec3& output){
 float facing=(float)(double(cap.z)*direction.z+double(cap.y)*direction.y+double(cap.x)*direction.x);if(!WorldText::Finite(facing)||(facing<0?-facing:facing)<0.01f)return false;
 double dot=positive?(double(cap.z)*point.z+double(cap.y)*point.y+double(cap.x)*point.x):(double(cap.x)*point.x+double(cap.z)*point.z+double(cap.y)*point.y);float distance=(float)(double(cap.d)-dot),reciprocal=(float)(1.0/double(facing));
 for(unsigned int a=0;a<3;a++){float product=(float)(double((&direction.x)[a])*distance);float scaled=(float)(double(reciprocal)*product);(&output.x)[a]=(float)(double(scaled)+(&point.x)[a]);}return WorldText::FiniteVector(output);
}
inline bool Build(PrefabScene::Arena& memory,const WorldText::Vec3* points,unsigned int count,const ExtrusionVolume::Plane& aperture,const ExtrusionVolume::Ends& ends,bool full,Volume& volume){
 // A semi extrusion (!full) keeps its own polygon as the bottom: no negative end is needed.
 volume.faces=0;volume.count=0;if(!points||count<3||count>128||!ends.hasPositive||ends.positiveDistance<=0||(full&&(!ends.hasNegative||ends.negativeDistance>=0)))return false;
 WorldText::Vec3 normal={aperture.x,aperture.y,aperture.z};float length=ExtrusionVolume::Length(normal.x,normal.y,normal.z);if(length<0.9999f||length>1.0001f)return false;
 WorldText::Vec3* bottom=(WorldText::Vec3*)memory.Allocate(count*12);WorldText::Vec3* top=(WorldText::Vec3*)memory.Allocate(count*12);SectorOrientation::Face* faces=(SectorOrientation::Face*)memory.Allocate((count+2)*sizeof(SectorOrientation::Face));if(!bottom||!top||!faces)return false;
 for(unsigned int i=0;i<count;i++){if(!Project(points[i],normal,ends.positive,true,top[i]))return false;if(full){if(!Project(points[i],normal,ends.negative,false,bottom[i]))return false;}else bottom[i]=points[i];}
 faces[0].points=bottom;faces[0].count=count;faces[1].points=(WorldText::Vec3*)memory.Allocate(count*12);faces[1].count=count;if(!faces[1].points)return false;for(unsigned int i=0;i<count;i++)faces[1].points[i]=top[count-1-i];
 for(unsigned int i=0;i<count;i++){SectorOrientation::Face& face=faces[i+2];face.count=4;face.points=(WorldText::Vec3*)memory.Allocate(48);if(!face.points)return false;unsigned int next=(i+1)%count;face.points[0]=bottom[i];face.points[1]=top[i];face.points[2]=top[next];face.points[3]=bottom[next];}
 bool flipped;SectorPreparation::Failure failure;if(!SectorPreparation::Prepare(faces,count+2,flipped,failure)||flipped)return false;volume.faces=faces;volume.count=count+2;return true;
}
}
