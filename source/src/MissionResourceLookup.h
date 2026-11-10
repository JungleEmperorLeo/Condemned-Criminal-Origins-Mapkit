#pragma once
// CMissionDB::IsMissionLevel semantics: ordered Missions -> Levels, first match.
class MissionResourceLookup {
 PrefabScene::Arena memory;
public:
 HRECORD mission;HRECORD level;unsigned int missionOrderIndex,levelIndex;const char* error;
 MissionResourceLookup():mission(0),level(0),missionOrderIndex(0),levelIndex(0),error(0){}
 bool Find(SoundDatabaseResources::Reader& database,const char* worldStem,const char* orderName="MissionOrder"){
  memory.Clear();mission=level=0;missionOrderIndex=levelIndex=0;error=0;if(!worldStem||!orderName){error="Missing mission lookup input";return false;}
  unsigned int length=lstrlenA(worldStem);if(length>4096){error="Mission world path too long";return false;}char* packed=(char*)memory.Allocate(length+12);if(!packed)return false;lstrcpyA(packed,worldStem);lstrcpyA(packed+length,".World00p");
  HRECORD order=database.Record("Missions/MissionOrder",orderName);if(!order){error="Missing mission order";return false;}unsigned int count=database.ValueCount(order,"Missions");if(count>4096)return false;
  for(unsigned int i=0;i<count;i++){HRECORD candidate=database.Link(order,"Missions",i);unsigned int levels=database.ValueCount(candidate,"Levels");if(levels>4096)return false;
   for(unsigned int j=0;j<levels;j++){HRECORD candidateLevel=database.Link(candidate,"Levels",j);const char* world=database.String(candidateLevel,"World");
    // SDK copies the stored World into a MAX_PATH buffer before comparison.
    char truncated[MAX_PATH];unsigned int n=lstrlenA(world);if(n>=MAX_PATH)n=MAX_PATH-1;for(unsigned int k=0;k<n;k++)truncated[k]=world[k];truncated[n]=0;
    if(FearBrushFlags::Equal(packed,truncated)){mission=candidate;level=candidateLevel;missionOrderIndex=i;levelIndex=j;return true;}
   }
  }return true;
 }
};
