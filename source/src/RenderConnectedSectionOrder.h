#pragma once
// Material-local section order on the main world's ordered portal graph.
// Recovered from 0x4F9D0 with rank helpers 0x4D210/280/330/3C0.
namespace RenderConnectedSectionOrder {
struct Node {unsigned int sectionStart,sectionCount,neighborStart,neighborCount;};
struct Section {const char* material;};
inline unsigned char Lower(unsigned char c){return c>='A'&&c<='Z'?(unsigned char)(c+32):c;}
inline bool Equal(const char* a,const char* b){if(!a||!b)return false;while(*a&&Lower((unsigned char)*a)==Lower((unsigned char)*b)){a++;b++;}return Lower((unsigned char)*a)==Lower((unsigned char)*b);}
class Builder {
 PrefabScene::Arena memory;const Node* nodes;const unsigned int* neighbors;unsigned int size;unsigned char* pending;unsigned int* distances;unsigned int* queue;
 Builder(const Builder&);Builder& operator=(const Builder&);
 unsigned int Degree(unsigned int node)const{unsigned int n=0;for(unsigned int i=0;i<nodes[node].neighborCount;i++)if(pending[neighbors[nodes[node].neighborStart+i]])n++;return n;}
 unsigned int Seed()const{unsigned int best=0xffffffffu,score=0xffffffffu;for(unsigned int n=0;n<size;n++)if(pending[n]){unsigned int rank=Degree(n)+1;if(rank<score){score=rank;best=n;}}return best;}
 unsigned int Next(unsigned int node)const{unsigned int best=0xffffffffu,score=0xffffffffu;for(unsigned int i=0;i<nodes[node].neighborCount;i++){unsigned int n=neighbors[nodes[node].neighborStart+i];if(!pending[n])continue;unsigned int rank=Degree(n);if(rank<score){score=rank;best=n;}}return best;}
 unsigned int Fallback(unsigned int from){
  for(unsigned int n=0;n<size;n++)distances[n]=0xffffffffu;distances[from]=0;unsigned int first=0,last=0;queue[last++]=from;
  while(first<last){unsigned int n=queue[first++];for(unsigned int i=0;i<nodes[n].neighborCount;i++){unsigned int next=neighbors[nodes[n].neighborStart+i];if(distances[next]!=0xffffffffu)continue;distances[next]=distances[n]+1;queue[last++]=next;}}
  unsigned int best=0xffffffffu,score=0xffffffffu;for(unsigned int n=0;n<size;n++)if(pending[n]){unsigned int rank=(distances[n]>>1)+(Degree(n)+1)*4;if(rank<score){score=rank;best=n;}}return best;
 }
public:
 unsigned int assigned;const char* error;
 Builder():nodes(0),neighbors(0),size(0),pending(0),distances(0),queue(0),assigned(0),error(0){}
 bool Assign(const Node* input,unsigned int count,const Section* sections,unsigned int sectionCount,const unsigned int* edges,unsigned int edgeCount,const char* material,unsigned int* order){
  memory.Clear();assigned=0;error=0;nodes=input;neighbors=edges;size=count;
  if(!material||!material[0]||count>4096||sectionCount>100000||edgeCount>1000000||(!input&&count)||(!sections&&sectionCount)||(!order&&sectionCount)||(!edges&&edgeCount)){error="Invalid connected section order input";return false;}
  for(unsigned int n=0;n<count;n++)if(nodes[n].sectionStart>sectionCount||nodes[n].sectionCount>sectionCount-nodes[n].sectionStart||nodes[n].neighborStart>edgeCount||nodes[n].neighborCount>edgeCount-nodes[n].neighborStart){error="Node section/portal range invalid";return false;}
  for(unsigned int i=0;i<edgeCount;i++)if(edges[i]>=count){error="Portal neighbor outside node table";return false;}
  for(unsigned int i=0;i<sectionCount;i++){const char* name=sections[i].material;if(!name){error="Null section material";return false;}unsigned int j=0;for(;j<=4096&&name[j];j++)if((unsigned char)name[j]>=128){error="Non-ASCII material comparison unsupported";return false;}if(j>4096){error="Material comparison name bound";return false;}}
  pending=(unsigned char*)memory.Allocate(count+1);distances=(unsigned int*)memory.Allocate((count+1)*4);queue=(unsigned int*)memory.Allocate((count+1)*4);if(!pending||!distances||!queue){error="Connected section order allocation failed";return false;}
  unsigned int remaining=0;for(unsigned int n=0;n<count;n++){pending[n]=0;for(unsigned int i=0;i<nodes[n].sectionCount;i++)if(Equal(sections[nodes[n].sectionStart+i].material,material)){pending[n]=1;remaining++;break;}}
  unsigned int seed=Seed(),current=seed;while(remaining){if(current>=count||!pending[current]){error="Connected order could not find pending node";return false;}
   for(unsigned int i=0;i<nodes[current].sectionCount;i++){unsigned int section=nodes[current].sectionStart+i;if(Equal(sections[section].material,material))order[section]=assigned++;}
   // Original esp+0x20 retains the FIRST seed. Its neighbor probe never moves
   // to the current node; only distance-weighted fallback uses the last node.
   pending[current]=0;remaining--;if(!remaining)break;unsigned int next=Next(seed);current=next!=0xffffffffu?next:Fallback(current);
  }
  return true;
 }
};
}
