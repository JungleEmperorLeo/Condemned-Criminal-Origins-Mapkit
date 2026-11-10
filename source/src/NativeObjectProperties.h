#pragma once
// CStringProp::SaveData (original RVA 5DCD0) gates the common filename
// converter (54670) on PF_FILENAME, loaded from the source LTA flag.
namespace NativeObjectProperties {
static const unsigned int FilenameFlag=0x4000u;
static const char* RuntimeExtension(const char* extension){
 const char* source[]={"World00a","World00c","Model00a","Model00c","Fx00a","Fx00c","TxAnm00a","TxAnm00c","Texture00a","Texture00c","Gamdb00s","Strdb00s"};
 const char* target[]={"World00p","World00p","Model00p","Model00p","Fx00p","Fx00p","TxAnm00","TxAnm00","dds","dds","Gamdb00p","Strdb00p"};
 for(unsigned int i=0;i<12;i++)if(FearBrushFlags::Equal(extension,source[i]))return target[i];return 0;
}
static bool PrepareFilename(PrefabScene::Property& property,PrefabScene::Arena& memory){
 if(!GameClassMetadata::Exact(property.type,"string")||!(property.flags&FilenameFlag))return true;
 if(!property.data)return false;unsigned int length=lstrlenA(property.data);
 // The original uses a MAX_PATH temporary. Reject its truncation boundary
 // explicitly instead of silently changing a resource identity.
 if(length>=MAX_PATH)return false;
 const char* extension=0;for(unsigned int i=length;i;i--){char c=property.data[i-1];if(c=='/'||c=='\\')break;if(c=='.'){extension=property.data+i;break;}}
 const char* runtime=extension?RuntimeExtension(extension):0;if(!runtime)return true;
 unsigned int prefix=(unsigned int)(extension-property.data),bytes=lstrlenA(runtime);if(prefix+bytes>=MAX_PATH)return false;
 char* converted=(char*)memory.Allocate(prefix+bytes+1);if(!converted)return false;memcpy(converted,property.data,prefix);lstrcpyA(converted+prefix,runtime);property.data=converted;return true;
}
}
