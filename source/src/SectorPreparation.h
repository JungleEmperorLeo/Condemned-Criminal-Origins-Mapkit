#pragma once
#include "PortalContour.h"
#include "SectorClosure.h"
namespace SectorPreparation {
enum Failure {None,InvalidInput,InvalidContour,InvalidOrientation,OpenOrAmbiguousEdges};
// Recovered geometry portion of RVA 0x2E8E0. Coordinates must already be transformed.
// Ownership, names, bounds, scene traversal and source transforms belong to the caller.
inline bool Prepare(SectorOrientation::Face* faces,unsigned int count,bool& flipped,Failure& failure){
 flipped=false;failure=None;if(!faces||count<4||count>4096){failure=InvalidInput;return false;}
 for(unsigned int i=0;i<count;i++)if(!PortalContour::Clean(faces[i].points,faces[i].count,faces[i].plane)){failure=InvalidContour;return false;}
 if(!SectorOrientation::Check(faces,count,0.1f)){SectorOrientation::Flip(faces,count);flipped=true;if(!SectorOrientation::Check(faces,count,0.1f)){failure=InvalidOrientation;return false;}}
 if(!SectorClosure::Check(faces,count,0.1f)){failure=OpenOrAmbiguousEdges;return false;}return true;
}
}
