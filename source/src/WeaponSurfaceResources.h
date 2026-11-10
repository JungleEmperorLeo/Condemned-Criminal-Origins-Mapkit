#pragma once
// SDK g_SurfaceFXTypes is append-only during gathering; duplicates retain order.
class WeaponSurfaceResources {
 PrefabScene::Arena memory;unsigned int count;
public:
 struct Entry{const char* name;Entry* next;};Entry* first;Entry* last;
 WeaponSurfaceResources():count(0),first(0),last(0){}
 bool Add(const char* name){if(!name)return true;unsigned int n=lstrlenA(name);if(n>4096||count>=4096)return false;Entry* entry=(Entry*)memory.Allocate(sizeof(Entry));char* copy=(char*)memory.Allocate(n+1);if(!entry||!copy)return false;lstrcpyA(copy,name);entry->name=copy;entry->next=0;if(last)last->next=entry;else first=entry;last=entry;count++;return true;}
};
