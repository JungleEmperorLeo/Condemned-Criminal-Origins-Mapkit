#pragma once
#include "PhysicsExplicitShapes.h"
namespace NativePhysicsProperties {
enum Mode {None=0,Sphere=1,OBB=2,Hull=3,Mesh=4};
inline bool Number(const PrefabScene::Node& node,const char* name,float fallback,float& value){value=fallback;const PrefabScene::Property* property=PhysicsExplicitShapes::Property(node,name);if(!property||!FearBrushFlags::Equal(property->type,"real"))return true;return PhysicsExplicitShapes::Number(node,name,value);}
inline bool Vector(const PrefabScene::Node& node,const char* name,const WorldText::Vec3& fallback,WorldText::Vec3& value){value=fallback;const PrefabScene::Property* property=PhysicsExplicitShapes::Property(node,name);if(!property||!FearBrushFlags::Equal(property->type,"vector"))return true;return PhysicsExplicitShapes::Vector(node,name,value);}
inline Mode Automatic(const PrefabScene::Node& node){const PrefabScene::Property* property=PhysicsExplicitShapes::Property(node,"PhysicsShape");if(!property||!property->string)return None;if(FearBrushFlags::Equal(property->data,"Mesh"))return Mesh;if(FearBrushFlags::Equal(property->data,"Hull"))return Hull;if(FearBrushFlags::Equal(property->data,"Sphere"))return Sphere;if(FearBrushFlags::Equal(property->data,"OBB"))return OBB;return None;}
inline bool MassDensity(const PrefabScene::Node& node,bool automatic,float& mass,float& density){if(!Number(node,"MassKg",1.f,mass)||!Number(node,"DensityG",1.1f,density))return false;if(automatic){if(mass<.1f)mass=.1f;if(density<.01f)density=.01f;}return true;}
}
