#pragma once
namespace ResourceOrder {
struct Node {Node* next;Node* previous;const char* path;unsigned int hash,value;};
class List {
 PrefabScene::Arena* arena;Node* tail;unsigned int active,mask;
 unsigned int Bucket(unsigned int hash){unsigned int b=hash&mask;return b<active?b:b-(mask>>1)-1;}
 char Upper(char c){return c>='a'&&c<='z'?c-('a'-'A'):c;}
 bool Equal(const char* a,const char* b){while(*a&&Upper(*a)==Upper(*b)){a++;b++;}return Upper(*a)==Upper(*b);}
 void Split(){
  if(mask<active)mask=mask*2+1;unsigned int split=active-(mask>>1)-1;Node* p=head;
  for(unsigned int i=0;i<count;i++){Node* next=p->next;
   if(Bucket(p->hash)==split&&(p->hash&mask)!=split&&p!=tail){
    if(p->previous)p->previous->next=p->next;else head=p->next;p->next->previous=p->previous;
    p->previous=tail;p->next=0;tail->next=p;tail=p;
   }p=next;
  }active++;
 }
public:
 Node* head;unsigned int count;
 void Init(PrefabScene::Arena* memory){arena=memory;head=tail=0;count=0;active=mask=1;}
 bool Insert(const char* path,unsigned int value,bool* inserted){
  if(!path||!inserted||count>=100000)return false;
  // Original grows before lookup, so even duplicate attempts may split a bucket.
  if(active<=count/4)Split();unsigned int hash=0;for(unsigned int i=0;path[i];i++){if(i>4096||(unsigned char)path[i]>=128)return false;hash=hash*29+(unsigned int)((int)Upper(path[i])-'A');}
  unsigned int bucket=Bucket(hash);Node* before=head;
  while(before&&Bucket(before->hash)<bucket)before=before->next;
  for(Node* p=before;p&&Bucket(p->hash)==bucket;p=p->next)if(Equal(p->path,path)){*inserted=false;return true;}
  Node* n=(Node*)arena->Allocate(sizeof(Node));if(!n)return false;n->path=path;n->hash=hash;n->value=value;n->next=before;n->previous=before?before->previous:tail;
  if(n->previous)n->previous->next=n;else head=n;if(before)before->previous=n;else tail=n;count++;*inserted=true;return true;
 }
};
// Recovered linear bucket splitting and insertion for ASCII resource paths.
}
