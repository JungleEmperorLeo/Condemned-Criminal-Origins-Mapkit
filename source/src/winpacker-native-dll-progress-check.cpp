#include "win_min.h"
#include "IPackerImpl.h"
#include "IPackerOutput.h"
#include "WinPackerNativeOptions.h"
extern "C" {
__declspec(dllimport) void WINAPI ExitProcess(unsigned int);
__declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char*);
__declspec(dllimport) void* WINAPI GetProcAddress(HMODULE, const char*);
}
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")
#pragma comment(linker,"/alternatename:__imp__LoadLibraryA@4=__imp__LoadLibraryA")
#pragma comment(linker,"/alternatename:__imp__GetProcAddress@8=__imp__GetProcAddress")

namespace NativeDllHost {
static bool Same(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}
static bool Copy(char* out, const char* value, unsigned int capacity) {
    if (!value) return false;
    unsigned int i = 0;
    for (; i + 1 < capacity && value[i]; ++i) out[i] = value[i];
    out[i] = 0;
    return !value[i];
}
class UI : public IPackerUI {
    unsigned int storage[8][8];
    char names[8][64], helps[8][1024], values[8][MAX_PATH], filters[8][128];
    CPackerProperty* properties[8];
public:
    CPackerPropList list;
    UI() { list.properties = properties; list.count = 0; }
    ~UI() { for (unsigned int i = 0; i < list.count; ++i) properties[i]->~CPackerProperty(); }
    bool CreateProperty(const CPackerProperty& source, const char* group) {
        if (!Same(group, "Native compiler") || list.count >= 8) return false;
        unsigned int i = list.count;
        const unsigned char* raw = (const unsigned char*)&source;
        if (!Copy(names[i], *(const char* const*)(raw + 4), sizeof(names[i])) ||
            !Copy(helps[i], *(const char* const*)(raw + 8), sizeof(helps[i]))) return false;
        if (source.GetType() == 0)
            properties[i] = new(storage[i])CPackerBoolProperty(names[i], raw[16] != 0, helps[i]);
        else if (source.GetType() == 1) {
            if (!Copy(filters[i], *(const char* const*)(raw + 20), sizeof(filters[i])) ||
                !Copy(values[i], *(const char* const*)(raw + 24), sizeof(values[i]))) return false;
            properties[i] = new(storage[i])CPackerStringProperty(names[i], values[i], raw[16] != 0,
                raw[17] != 0, raw[18] != 0, filters[i], helps[i]);
        } else return false;
        ++list.count;
        return true;
    }
    bool CreateReference(const char*, const char*) { return false; }
    bool String(const char* name, const char* value) {
        for (unsigned int i = 0; i < list.count; ++i)
            if (Same(names[i], name) && properties[i]->GetType() == 1) {
                if (!Copy(values[i], value, MAX_PATH)) return false;
                *(const char**)((unsigned char*)properties[i] + 24) = values[i];
                return true;
            }
        return false;
    }
    bool Flag(const char* name, bool value) {
        for (unsigned int i = 0; i < list.count; ++i)
            if (Same(names[i], name) && properties[i]->GetType() == 0) {
                *((unsigned char*)properties[i] + 16) = value ? 1u : 0u;
                return true;
            }
        return false;
    }
};
struct Event { unsigned int type, code; float value; char text[96]; };
static_assert(sizeof(Event) == 108, "event wire size");
class Output : public IPackerOutput {
public:
    Event events[512];
    unsigned int count, overflow;
    Output() : count(0), overflow(0) {}
    bool Add(unsigned int type, unsigned int code, float value, const char* text) {
        if (count == 512) { overflow = 1; return false; }
        Event& e = events[count++];
        e.type = type; e.code = code; e.value = value;
        unsigned int i = 0;
        if (text) for (; i + 1 < sizeof(e.text) && text[i]; ++i) e.text[i] = text[i];
        for (; i < sizeof(e.text); ++i) e.text[i] = 0;
        return true;
    }
    bool CreateTask(const char* name) { return Add(1, 0, 0, name); }
    bool ActivateTask(const char* name) { return Add(2, 0, 0, name); }
    bool ActivateSubTask(const char* name) { return Add(3, 0, 0, name); }
    bool UpdateProgress(float value) { return Add(4, 0, value, 0); }
    bool CheckOutputFile(const char* name) { return Add(5, 0, 0, name); }
    bool LogMessage(EMsgType level, const char* message) { return Add(6, (unsigned int)level, 0, message); }
    bool LogMessageH(EMsgType level, const char*, const char* message) { return Add(7, (unsigned int)level, 0, message); }
};
static bool Environment(const char* name, char* value, bool optional = false) {
    DWORD bytes = GetEnvironmentVariableA(name, value, MAX_PATH);
    return bytes < MAX_PATH && (bytes || optional);
}
static bool Write(HANDLE file, const void* data, unsigned int bytes) {
    DWORD done = 0;
    return WriteFile(file, data, bytes, &done, 0) && done == bytes;
}
}

extern "C" void HarnessEntry() {
    using namespace NativeDllHost;
    char dll[MAX_PATH] = {}, source[MAX_PATH] = {}, project[MAX_PATH] = {}, destination[MAX_PATH] = {},
         resources[MAX_PATH] = {}, result[MAX_PATH] = {}, split[MAX_PATH] = {},
         postProfile[MAX_PATH] = {}, worldStem[MAX_PATH] = {};
    if (!Environment("FEAR_WINPACKER_DLL_PATH", dll) ||
        !Environment("FEAR_WINPACKER_HOST_SOURCE", source) ||
        !Environment("FEAR_WINPACKER_HOST_PROJECT", project) ||
        !Environment("FEAR_WINPACKER_HOST_OUTPUT", destination) ||
        !Environment("FEAR_WINPACKER_HOST_RESOURCES", resources, true) ||
        !Environment("FEAR_WINPACKER_HOST_RESULT", result) ||
        !Environment("FEAR_WINPACKER_HOST_SPLIT", split, true) ||
        !Environment("FEAR_WINPACKER_HOST_POST_PROFILE", postProfile, true) ||
        !Environment("FEAR_WINPACKER_HOST_WORLD_STEM", worldStem, true)) ExitProcess(1);
    HMODULE module = LoadLibraryA(dll);
    if (!module) ExitProcess(2);
    typedef IPackerImpl* (__cdecl *Associate)(const char*);
    Associate associate = (Associate)GetProcAddress(module, "AssociatePacker");
    if (!associate) ExitProcess(3);
    IPackerImpl* packer = associate(source);
    if (!packer) ExitProcess(4);
    static unsigned int uiMemory[(sizeof(UI) + 3) / 4], outputMemory[(sizeof(Output) + 3) / 4];
    UI& ui = *new(uiMemory)UI;
    Output& output = *new(outputMemory)Output;
    bool requested = packer->RequestUserOptions(&ui);
    bool configured = requested && ui.list.count == 8 && ui.String("NativeProject", project) &&
        ui.String("NativeResources", resources) && ui.String("NativeOutput", destination) &&
        ui.Flag("NativeSectionsOnly", false) && ui.Flag("CenterWorld", true) &&
        ui.Flag("SplitMeshes", Same(split, "true")) &&
        (!postProfile[0] || ui.String("NativePostProfile", postProfile)) &&
        (!worldStem[0] || ui.String("NativeWorldStem", worldStem));
    bool processed = configured && packer->Process(source, &ui.list, &output);
    unsigned int header[6] = {0x31455057u, 1, requested ? 1u : 0u, configured ? 1u : 0u,
                             processed ? 1u : 0u, output.count | (output.overflow << 31)};
    HANDLE file = CreateFileA(result, 0x40000000, FILE_SHARE_READ, 0, 2, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE || !Write(file, header, sizeof(header)) ||
        !Write(file, output.events, output.count * sizeof(Event))) ExitProcess(5);
    CloseHandle(file);
    ui.~UI();
    // The isolated host exits immediately; Windows releases the loaded module.
    ExitProcess(configured ? 0 : 6);
}

void operator delete(void*, unsigned int) {}
extern "C" int __cdecl _purecall() { return 0; }
extern "C" { int _fltused = 0; }
extern "C" void* __cdecl memset(void* target, int value, unsigned int bytes) {
    unsigned char* data = (unsigned char*)target;
    while (bytes--) *data++ = (unsigned char)value;
    return target;
}
