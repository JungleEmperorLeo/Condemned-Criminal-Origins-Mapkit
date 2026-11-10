#pragma once
namespace PhysicsPrimitives {
inline bool OBB(float mass,float density,const WorldText::Vec3& half,float* body){
 if(!WorldText::Finite(mass)||!WorldText::Finite(density)||mass<0||density<0||!WorldText::FiniteVector(half)||half.x<0||half.y<0||half.z<0)return false;
 WorldText::Vec3 adjusted=half;if(adjusted.x<.5f)adjusted.x=.5f;if(adjusted.y<.5f)adjusted.y=.5f;if(adjusted.z<.5f)adjusted.z=.5f;
 float minimum=adjusted.x;if(adjusted.y<minimum)minimum=adjusted.y;if(adjusted.z<minimum)minimum=adjusted.z;float margin=minimum-2.5f;if(margin>3)margin=3;
 body[0]=mass;body[1]=density;body[2]=margin;body[3]=adjusted.x-margin;body[4]=adjusted.y-margin;body[5]=adjusted.z-margin;return true;
}
inline bool Sphere(float mass,float density,float radius,float* body){
 if(!WorldText::Finite(mass)||!WorldText::Finite(density)||!WorldText::Finite(radius)||mass<0||density<0||radius<=0)return false;body[0]=mass;body[1]=density;body[2]=radius;return true;
}
}
