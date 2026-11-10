#pragma once
#include "NativePreparedRenderScene.h"
#include "SourcePhysicalBspContours.h"
#include "SourceKeyRotation.h"
#include "SourceRuntimeObjectOrder.h"
#include "NativeBlindDataRegistry.h"
#include "ScatterNoise.h"
#include "LTRotationOps.h"
// ScatterVolume BlindData (type 0x73f53a84, version 1) recovered from retail
// WorldPacker ConvertScatter (VA 0x10011BE0): volumes 0x10011610, particles
// 0x100104A0, kill 0x1000EF30, outside 0x10010380, subvolumes 0x100112A0/
// 0x1000FE20, writer 0x1000F920. Every explicit (float) is an original fstp
// dword; double expressions are the x87 53-bit register values. The polygon
// set is the MAIN physical BSP contour list. scatter-research/SCATTER_RE_NOTES.md.
namespace NativeScatter {
static const unsigned int TypeId=0x73f53a84u,Missing=0xffffffffu;
struct Settings {
 unsigned int version,maximumBytes,maximumParticles,maximumCells,requiredFlags,excludedFlags;bool centered;
 Settings():version(1),maximumBytes(16u*1024u*1024u),maximumParticles(4000000),maximumCells(1000000),requiredFlags(0),excludedFlags(0x180),centered(true){}
};
struct Particle {WorldText::Vec3 position;float scale;unsigned char bytes[4];};
struct SubVolume {WorldText::Vec3 center,half;unsigned int first,count;};
struct Polygon {const WorldText::Vec3* points;unsigned int count;const char* material;WorldText::Vec3 low,high;};
namespace detail {
inline float F(double v){return (float)v;}
inline float Abs(float v){unsigned int bits;memcpy(&bits,&v,4);bits&=0x7fffffffu;memcpy(&v,&bits,4);return v;}
inline int Trunc(double v){return ScatterNoise::Trunc(v);}
inline int Lower(int c){return c>='A'&&c<='Z'?c+32:c;}
inline bool SameNoCase(const char* a,const char* b){for(;;a++,b++){int x=Lower((unsigned char)*a),y=Lower((unsigned char)*b);if(x!=y)return false;if(!x)return true;}}
inline WorldText::Vec3 Rotate(const float* q,const WorldText::Vec3& v){return LTRotationOps::Rotate(q,v);}
// Rotation matrix columns (VA 0x1000EDE0 + GetVectors 0x1000F190).
inline bool Axes(const float* q,WorldText::Vec3& right,WorldText::Vec3& up,WorldText::Vec3& forward){
 const float x=q[0],y=q[1],z=q[2],w=q[3];const double norm=(((double)x*x+(double)y*y)+(double)z*z)+(double)w*w;if(!(norm>0))return false;
 const float s=F(2.0/norm),xs=F((double)s*x),ys=F((double)s*y),zs=F((double)s*z);
 const float wx=F((double)xs*w),wy=F((double)ys*w),wz=F((double)zs*w),xx=F((double)xs*x),xy=F((double)ys*x),xz=F((double)zs*x),yy=F((double)ys*y),yz=F((double)zs*y),zz=F((double)zs*z);
 right.x=F(1.0-((double)zz+yy));up.x=F((double)xy-wz);forward.x=F((double)xz+wy);
 right.y=F((double)xy+wz);up.y=F(1.0-((double)zz+xx));forward.y=F((double)yz-wx);
 right.z=F((double)xz-wy);up.z=F((double)yz+wx);forward.z=F(1.0-((double)yy+xx));return true;
}
inline float Wave(const WorldText::Vec3& p,float size,const ScatterNoise::Noise& noise,unsigned char& out){
 const float inverse=F(1.0/(double)size);
 const float qx=F((double)inverse*p.x),qy=F((double)inverse*p.y),qz=F((double)inverse*p.z);
 float v=F((noise.Value(qx,qy,qz)+1.0)*0.5);v=F((double)v-(double).3f);v=F((double)v*2.5);
 if(v<0)v=0;else if(v>1)v=1;out=(unsigned char)Trunc((double)v*255.0);return v;
}
}
class Builder {
 PrefabScene::Arena memory;Builder(const Builder&);Builder& operator=(const Builder&);
 bool Fail(const char* message,unsigned int node=Missing){char saved[512];lstrcpynA(saved,message?message:"Scatter source compilation failed",512);Clear();lstrcpyA(diagnostic,saved);error=diagnostic;errorNode=node;return false;}
 static const PrefabScene::Property* Property(const PrefabScene::Node& n,const char* name,const char* type){for(const PrefabScene::Property* p=n.properties;p;p=p->next)if(GameClassMetadata::Exact(p->name,name))return GameClassMetadata::Exact(p->type,type)&&p->data?p:0;return 0;}
 static bool Real(const PrefabScene::Property* p,float& value){if(!p)return false;WorldText::Document doc;return doc.Parse(p->data,lstrlenA(p->data))&&doc.Elements(0)==1&&doc.Number(doc.First(0),value)&&WorldText::Finite(value);}
 static bool Vector(const PrefabScene::Property* p,WorldText::Vec3& value){if(!p)return false;WorldText::Document doc;if(!doc.Parse(p->data,lstrlenA(p->data)))return false;unsigned int form=doc.First(0),inner=doc.Next(doc.First(form));return doc.tokens[inner].list&&doc.Vector(inner,value)&&WorldText::FiniteVector(value);}
 // Growable particle store; the arena only grows, as the original vector does.
 Particle* particles;unsigned int particleCount,particleCapacity;
 bool Push(const Particle& p,unsigned int limit){
  if(particleCount==particleCapacity){if(particleCapacity>=limit)return false;unsigned int wanted=particleCapacity?particleCapacity*2:4096;if(wanted>limit)wanted=limit;Particle* grown=(Particle*)scratch.Allocate(wanted*sizeof(Particle));if(!grown)return false;if(particleCount)memcpy(grown,particles,particleCount*sizeof(Particle));particles=grown;particleCapacity=wanted;}
  particles[particleCount++]=p;return true;
 }
 PrefabScene::Arena scratch;
 struct VolumeInput {
  WorldText::Vec3 dims,clumpOffset;float density,plantDepth,clumpiness,clumpSize,clumpCutoff,waveSize,waveDirSize,scaleRange,placementCutoff;unsigned int images;const char* material;
  float inverse[4];WorldText::Vec3 translation;
 };
 // ScatterParticlesOnPoly (VA 0x100104A0).
 bool Scatter(const Polygon& polygon,const VolumeInput& v,ScatterNoise::Random& random,const ScatterNoise::Noise& noise,const Settings& settings){
  if(v.material[0]&&polygon.material&&!detail::SameNoCase(polygon.material,v.material))return true;
  using detail::F;const WorldText::Vec3& v0=polygon.points[0];
  for(unsigned int k=0;k+2<polygon.count;k++){
   const WorldText::Vec3& v1=polygon.points[k+1];const WorldText::Vec3& v2=polygon.points[k+2];
   const float e2z=F((double)v2.z-v0.z),e2y=F((double)v2.y-v0.y),e2x=F((double)v2.x-v0.x),e1z=F((double)v1.z-v0.z),e1y=F((double)v1.y-v0.y),e1x=F((double)v1.x-v0.x);
   const float cz=F((double)e1y*e2x-(double)e1x*e2y),cy=F((double)e1x*e2z-(double)e1z*e2x),cx=F((double)e1z*e2y-(double)e1y*e2z);
   const float sum=F(((double)cx*cx+(double)cy*cy)+(double)cz*cz);if(!(sum>=0))return Fail("Invalid Scatter polygon area");
   const double area=(double)F(ScatterNoise::X87Sqrt((double)sum))*0.5;const double full=((double)v.density*area)*(1.0/65536.0);
   if(!(full<2147483647.0))return Fail("Scatter particle count overflow");
   int n=detail::Trunc(full);const double chance=random.Unit();if(full-(double)n>chance)n++;
   for(int i=0;i<n;i++){
    float b=F(random.Unit()),c=F(random.Unit());if((double)c+(double)b>1.0){b=F(1.0-(double)b);c=F(1.0-(double)c);}
    const float a=F((1.0-(double)b)-(double)c);
    WorldText::Vec3 p;
    p.z=F((double)F((double)v2.z*c)+(double)F((double)F((double)v0.z*a)+(double)F((double)v1.z*b)));
    p.y=F((double)F((double)v2.y*c)+(double)F((double)F((double)v0.y*a)+(double)F((double)v1.y*b)));
    p.x=F((double)F((double)v2.x*c)+(double)F((double)F((double)v0.x*a)+(double)F((double)v1.x*b)));
    const float alpha=F(((double)a+c)+b);
    // KillParticle (VA 0x1000EF30).
    if(alpha<v.placementCutoff)continue;
    {const float ox=F((double)p.x+v.clumpOffset.x),oy=F((double)p.y+v.clumpOffset.y),oz=F((double)p.z+v.clumpOffset.z),inverse=F(1.0/(double)v.clumpSize);
     const float clump=F((noise.Value(F((double)ox*inverse),F((double)oy*inverse),F((double)oz*inverse))+1.0)*0.5);
     const float weight=F((1.0-(double)clump)*(1.0-(double)v.clumpiness)+(double)clump);if(weight<v.clumpCutoff)continue;
     const float keep=F((double)weight*alpha);if((double)keep<random.Unit())continue;}
    Particle out;const WorldText::Vec3 r=detail::Rotate(v.inverse,p);
    out.position.z=F((double)r.z+v.translation.z);out.position.y=F((double)r.y+v.translation.y);out.position.x=F((double)v.translation.x+r.x);
    const float unit=F(random.Unit());out.scale=F(((double)unit+unit-1.0)*v.scaleRange+1.0);
    detail::Wave(p,v.waveDirSize,noise,out.bytes[0]);detail::Wave(p,v.waveSize,noise,out.bytes[1]);
    const double d=random.Unit();int o=detail::Trunc(d*255.0);out.bytes[2]=(unsigned char)(o<0?0:(o>=255?255:o));
    random.Next();int m=detail::Trunc((double)v.images*d);const int last=(int)v.images-1;out.bytes[3]=(unsigned char)(m<0?0:(m>=last?last:m));
    if(!Push(out,settings.maximumParticles))return Fail("Scatter particle limit exceeded");
   }
  }
  return true;
 }
public:
 NativeBlindDataRegistry::RecordInput* records;NativeBlindDataRegistry::BindingInput* bindings;
 unsigned int* missingNodes;unsigned int count,bindingCount,missingCount,volumeCount,totalParticles,totalBytes,errorNode;const char* error;char diagnostic[512];
 Builder():particles(0),particleCount(0),particleCapacity(0),records(0),bindings(0),missingNodes(0),count(0),bindingCount(0),missingCount(0),volumeCount(0),totalParticles(0),totalBytes(0),errorNode(Missing),error(0){diagnostic[0]=0;}
 void Clear(){memory.Clear();scratch.Clear();particles=0;particleCount=particleCapacity=0;records=0;bindings=0;missingNodes=0;count=bindingCount=missingCount=volumeCount=totalParticles=totalBytes=0;errorNode=Missing;error=0;diagnostic[0]=0;}
 static bool IsVolume(const PrefabScene::Node& n){if(n.kind!=2)return false;const char* a=n.className;const char* b="ScatterVolume";for(;*a==*b;a++,b++)if(!*a)return true;return false;}
 bool Build(const NativePreparedRenderScene::Context& scene,const Settings& settings=Settings()){
  Clear();if(!scene.Ready()||settings.version!=1||settings.maximumBytes<8||settings.maximumBytes>16u*1024u*1024u||!settings.maximumParticles||settings.maximumParticles>4000000||!settings.maximumCells||settings.maximumCells>1000000)return Fail("Invalid Scatter source scene/settings");
  const PrefabScene::Model& model=scene.Model();
  SourceRuntimeObjectOrder::Builder order;if(!order.Build(scene))return Fail(order.error,order.errorNode);
  const PrefabScene::Node** nodes=(const PrefabScene::Node**)memory.Allocate((model.count?model.count:1)*sizeof(void*));if(!nodes)return Fail("Scatter node index allocation failed");
  unsigned int seen=0;for(const PrefabScene::Node* n=model.first;n;n=n->next){if(seen>=model.count||n->index!=seen)return Fail("Invalid Scatter source node order");nodes[seen++]=n;}if(seen!=model.count)return Fail("Incomplete Scatter source index");
  for(unsigned int i=0;i<order.count;i++){if(order.rows[i].sourceNode>=seen)return Fail("Invalid Scatter runtime object");if(IsVolume(*nodes[order.rows[i].sourceNode]))volumeCount++;}
  if(!volumeCount)return true;
  records=(NativeBlindDataRegistry::RecordInput*)memory.Allocate(volumeCount*sizeof(*records));bindings=(NativeBlindDataRegistry::BindingInput*)memory.Allocate(volumeCount*sizeof(*bindings));missingNodes=(unsigned int*)memory.Allocate(volumeCount*4);if(!records||!bindings||!missingNodes)return Fail("Scatter record allocation failed");
  // The physical BSP world polygon list (m_Polies), in contour order.
  const WorldText::Vec3 zero={0,0,0};const WorldText::Vec3& offset=settings.centered?scene.Offset():zero;
  // Held in the builder arena: the contour builder is too large for a CRT-free stack frame.
  PrefabScene::Arena contourMemory;void* contoursPlace=contourMemory.Allocate(sizeof(SourcePhysicalBspContours::Main));if(!contoursPlace)return Fail("Scatter contour allocation failed");
  struct Holder{SourcePhysicalBspContours::Main* p;~Holder(){p->~Main();}} holder={new(contoursPlace)SourcePhysicalBspContours::Main};SourcePhysicalBspContours::Main& contours=*holder.p;
  if(!contours.Build(model,scene.Partition(),offset,false))return Fail(contours.error);
  Polygon* polygons=(Polygon*)memory.Allocate(contours.count*sizeof(Polygon));if(!polygons)return Fail("Scatter polygon allocation failed");unsigned int polygonCount=0;
  for(unsigned int i=0;i<contours.count;i++){const SourcePhysicalBspContours::Contour& c=contours.contours[i];if(c.sourceNode>=seen||c.sourcePolygon>=nodes[c.sourceNode]->geometry.polygons||c.polygon.count<3)return Fail("Invalid Scatter polygon provenance");
   // m_Polies holds no 0x180 surfaces (absent from the physical BSP reservation); controls-f3e9faf6/anymaterial.
   if((c.polygon.flags&settings.requiredFlags)!=settings.requiredFlags||(c.polygon.flags&settings.excludedFlags))continue;
   Polygon& p=polygons[polygonCount++];p.points=c.polygon.points;p.count=c.polygon.count;p.material=nodes[c.sourceNode]->geometry.faces[c.sourcePolygon].material;p.low=p.high=p.points[0];
   for(unsigned int j=1;j<p.count;j++)for(unsigned int k=0;k<3;k++){const float x=(&p.points[j].x)[k];if(x<(&p.low.x)[k])(&p.low.x)[k]=x;if(x>(&p.high.x)[k])(&p.high.x)[k]=x;}}
  ScatterNoise::Noise noise;ScatterNoise::Random random;random.Seed(1337);
  for(unsigned int row=0;row<order.count;row++){
   const PrefabScene::Node& node=*nodes[order.rows[row].sourceNode];if(!IsVolume(node))continue;
   if(!Volume(node,polygons,polygonCount,scene.Offset(),random,noise,settings))return false;
  }
  return true;
 }
private:
 bool Volume(const PrefabScene::Node& node,const Polygon* polygons,unsigned int polygonCount,const WorldText::Vec3& worldOffset,ScatterNoise::Random& random,const ScatterNoise::Noise& noise,const Settings& settings){
  using detail::F;VolumeInput v;
  const PrefabScene::Property* material=Property(node,"PlacementMaterial","string");
  if(!Vector(Property(node,"Dims","vector"),v.dims)||!Property(node,"BlindDataIndex","real")||!Real(Property(node,"Density","real"),v.density)||!Real(Property(node,"Clumpiness","real"),v.clumpiness)||!Real(Property(node,"ClumpSize","real"),v.clumpSize)||!Real(Property(node,"WaveSize","real"),v.waveSize)||!Real(Property(node,"ScaleRange","real"),v.scaleRange)||!material)
   return Fail("Found bad ScatterVolume object",node.index);
  if(!Real(Property(node,"PlantDepth","real"),v.plantDepth))v.plantDepth=0;
  if(!Real(Property(node,"ClumpCutoff","real"),v.clumpCutoff))v.clumpCutoff=0;
  if(!Vector(Property(node,"ClumpOffset","vector"),v.clumpOffset))v.clumpOffset.x=v.clumpOffset.y=v.clumpOffset.z=0;
  if(!Real(Property(node,"WaveDirSize","real"),v.waveDirSize))v.waveDirSize=v.waveSize;
  if(!Real(Property(node,"PlacementCutoff","real"),v.placementCutoff))v.placementCutoff=0;
  float images;v.images=1;if(Real(Property(node,"NumImages","real"),images)){int n=detail::Trunc((double)images+.5);v.images=n<1?1u:(n>=255?255u:(unsigned int)n);}
  v.material=material->data;
  float rotation[4];if(!SourceKeyRotation::Quaternion(node,rotation))return Fail("Invalid ScatterVolume rotation",node.index);
  WorldText::Vec3 position=node.position;if(settings.centered){position.x-=worldOffset.x;position.y-=worldOffset.y;position.z-=worldOffset.z;}
  if(!WorldText::FiniteVector(position))return Fail("Invalid ScatterVolume position",node.index);
  // LTRigidTransform inverse (VA 0x1000F290): conjugate rotation, negated rotated position.
  v.inverse[0]=-rotation[0];v.inverse[1]=-rotation[1];v.inverse[2]=-rotation[2];v.inverse[3]=rotation[3];
  {const WorldText::Vec3 t=detail::Rotate(v.inverse,position);v.translation.x=-t.x;v.translation.y=-t.y;v.translation.z=-t.z;}
  WorldText::Vec3 right,up,forward;if(!detail::Axes(rotation,right,up,forward))return Fail("Invalid ScatterVolume rotation",node.index);
  WorldText::Vec3 extent,low,high;
  for(unsigned int k=0;k<3;k++){const float a=F((double)detail::Abs((&forward.x)[k])*v.dims.z),b=F((double)detail::Abs((&up.x)[k])*v.dims.y+(double)a);(&extent.x)[k]=F((double)detail::Abs((&right.x)[k])*v.dims.x+(double)b);
   (&low.x)[k]=F((double)(&position.x)[k]-(double)(&extent.x)[k]);(&high.x)[k]=F((double)(&extent.x)[k]+(double)(&position.x)[k]);}
  // Collection (VA 0x10010070) precedes the Density and PlacementMaterial early returns.
  unsigned int selected=0;for(unsigned int i=0;i<polygonCount;i++){const Polygon& p=polygons[i];if(low.x<=p.high.x&&p.low.x<=high.x&&low.y<=p.high.y&&p.low.y<=high.y&&low.z<=p.high.z&&p.low.z<=high.z)selected++;}
  if(!selected||!(v.density>0)){missingNodes[missingCount++]=node.index;return true;}
  particleCount=0;
  for(unsigned int i=0;i<polygonCount;i++){const Polygon& p=polygons[i];if(low.x<=p.high.x&&p.low.x<=high.x&&low.y<=p.high.y&&p.low.y<=high.y&&low.z<=p.high.z&&p.low.z<=high.z&&!Scatter(p,v,random,noise,settings))return false;}
  // RemoveOutsideParticles (VA 0x10010380): inclusive local box.
  unsigned int kept=0;for(unsigned int i=0;i<particleCount;i++){const WorldText::Vec3& q=particles[i].position;if(-v.dims.x<=q.x&&-v.dims.y<=q.y&&-v.dims.z<=q.z&&q.x<=v.dims.x&&q.y<=v.dims.y&&q.z<=v.dims.z)particles[kept++]=particles[i];}particleCount=kept;
  if(!particleCount){missingNodes[missingCount++]=node.index;return true;}
  // CreateSubVolumes (VA 0x100112A0): 512-unit XZ cells, Z outer; FillSubVolume (VA 0x1000FE20).
  WorldText::Vec3 lo=particles[0].position,hi=lo;
  for(unsigned int i=0;i<particleCount;i++)for(unsigned int k=0;k<3;k++){const float x=(&particles[i].position.x)[k];if(x<(&lo.x)[k])(&lo.x)[k]=x;else if(x>(&hi.x)[k])(&hi.x)[k]=x;}
  lo.x=F((double)lo.x-(double).1f);lo.z=F((double)lo.z-(double).1f);hi.x=F((double)hi.x+(double).1f);hi.z=F((double)hi.z+(double).1f);
  const float depth=F((double)hi.z-lo.z),width=F((double)hi.x-lo.x);
  // Cell counts are 1-ftol(extent/-512.0f).
  const double qx=(double)width/-512.0,qz=(double)depth/-512.0;if(!(qx<=0&&qz<=0&&qx>-(double)settings.maximumCells&&qz>-(double)settings.maximumCells))return Fail("Scatter subvolume grid limit",node.index);
  const unsigned int nx=1u+(unsigned int)(-detail::Trunc(qx)),nz=1u+(unsigned int)(-detail::Trunc(qz));if((unsigned long long)nx*nz>settings.maximumCells)return Fail("Scatter subvolume grid limit",node.index);
  unsigned int* indices=(unsigned int*)scratch.Allocate(particleCount*4);SubVolume* subs=(SubVolume*)scratch.Allocate(nx*nz*sizeof(SubVolume));if(!indices||!subs)return Fail("Scatter subvolume allocation failed",node.index);
  unsigned int subCount=0,used=0;float remainingZ=depth,startZ=lo.z;
  for(unsigned int row=0;row<nz;row++){
   float sizeZ=remainingZ;if(!(remainingZ<512.f)){sizeZ=512.f;remainingZ=F((double)remainingZ-512.0);}
   const float halfZ=F((double)sizeZ*.5),centerZ=F((double)halfZ+startZ);float remainingX=width,startX=lo.x;
   for(unsigned int column=0;column<nx;column++){
    float sizeX=remainingX;if(!(remainingX<512.f)){sizeX=512.f;remainingX=F((double)remainingX-512.0);}
    const float halfX=F((double)sizeX*.5),centerX=F((double)halfX+startX);
    const float minZ=F((double)centerZ-halfZ),minX=F((double)centerX-halfX),maxZ=F((double)halfZ+centerZ),maxX=F((double)halfX+centerX);
    SubVolume& s=subs[subCount];s.first=used;s.count=0;WorldText::Vec3 a={},b={};
    for(unsigned int i=0;i<particleCount;i++){const WorldText::Vec3& q=particles[i].position;
     if(q.x<minX||q.z<minZ||!(q.x<maxX)||!(q.z<maxZ))continue;
     if(!s.count){a=b=q;}
     for(unsigned int k=0;k<3;k++){const float x=(&q.x)[k];if(x<(&a.x)[k])(&a.x)[k]=x;else if(x>(&b.x)[k])(&b.x)[k]=x;}
     indices[used++]=i;s.count++;}
    if(s.count){
     const float hz=F((double)F((double)b.z-a.z)*.5),hy=F((double)F((double)b.y-a.y)*.5),hx=F((double)F((double)b.x-a.x)*.5);
     s.center.z=F((double)hz+a.z);s.center.y=F((double)hy+a.y);s.center.x=F((double)hx+a.x);s.half.x=hx;s.half.y=hy;s.half.z=hz;subCount++;}
    startX=F((double)startX+512.0);
   }
   startZ=F((double)startZ+512.0);
  }
  // AddScatterBlindData (VA 0x1000F920).
  unsigned long long bytes=8ull+28ull*subCount+20ull*used;if(bytes>settings.maximumBytes-totalBytes)return Fail("Scatter payload limit",node.index);
  unsigned char* data=(unsigned char*)memory.Allocate((unsigned int)bytes);if(!data)return Fail("Scatter payload allocation failed",node.index);
  unsigned int at=0;const unsigned int one=1;memcpy(data,&one,4);memcpy(data+4,&subCount,4);at=8;
  for(unsigned int s=0;s<subCount;s++){const SubVolume& sub=subs[s];const float head[6]={sub.center.x,F((double)sub.center.y-v.plantDepth),sub.center.z,sub.half.x,sub.half.y,sub.half.z};memcpy(data+at,head,24);memcpy(data+at+24,&sub.count,4);at+=28;
   for(unsigned int j=0;j<sub.count;j++){const Particle& q=particles[indices[sub.first+j]];const float values[4]={q.position.x,F((double)q.position.y-v.plantDepth),q.position.z,q.scale};memcpy(data+at,values,16);memcpy(data+at+16,q.bytes,4);at+=20;}}
  if(at!=bytes)return Fail("Scatter payload cursor mismatch",node.index);
  records[count].typeId=TypeId;records[count].data=data;records[count].bytes=at;bindings[bindingCount].sourceNode=node.index;bindings[bindingCount].property="BlindDataIndex";bindings[bindingCount].ordinal=count;bindings[bindingCount].typeId=TypeId;
  count++;bindingCount++;totalParticles+=used;totalBytes+=at;return true;
 }
};
}
