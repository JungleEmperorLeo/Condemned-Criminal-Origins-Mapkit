#include "win_min.h"
#include "WinPackerNativeProgress.h"
extern "C" __declspec(dllimport) void WINAPI ExitProcess(unsigned int);
#pragma comment(linker,"/alternatename:__imp__ExitProcess@4=__imp__ExitProcess")
#pragma comment(linker,"/alternatename:__imp__CreateFileA@28=__imp__CreateFileA")
#pragma comment(linker,"/alternatename:__imp__WriteFile@20=__imp__WriteFile")
#pragma comment(linker,"/alternatename:__imp__CloseHandle@4=__imp__CloseHandle")

namespace ProgressHost {
static unsigned int checks[64], used;
#define CHECK(name, condition) checks[used++] = (condition) ? 1u : 0u
static bool Same(const char* a, const char* b) {
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}
class Host : public IPackerOutput {
public:
    unsigned int creates, activates, subtasks, updates, unexpected, eventCount;
    unsigned int events[64];
    float values[32];
    float displayed;
    const char* mainName;
    const char* label;
    bool callbackResult;
    Host() : creates(0), activates(0), subtasks(0), updates(0), unexpected(0), eventCount(0), displayed(0),
             mainName(0), label(0), callbackResult(true) {}
    void Event(unsigned int type) { if (eventCount < 64) events[eventCount++] = type; else ++unexpected; }
    bool CreateTask(const char* name) { Event(1); ++creates; mainName = name; return callbackResult; }
    bool ActivateTask(const char* name) {
        Event(2);
        ++activates;
        if (!mainName || !Same(mainName, name)) ++unexpected;
        return callbackResult;
    }
    bool ActivateSubTask(const char* name) { Event(3); ++subtasks; label = name; displayed = 0; return callbackResult; }
    bool UpdateProgress(float value) {
        Event(4);
        displayed = value;
        if (updates < 32) values[updates] = value;
        else ++unexpected;
        ++updates;
        return callbackResult;
    }
    bool CheckOutputFile(const char*) { ++unexpected; return false; }
    bool LogMessage(EMsgType, const char*) { ++unexpected; return false; }
    bool LogMessageH(EMsgType, const char*, const char*) { ++unexpected; return false; }
    bool Monotonic(bool completed) const {
        for (unsigned int i = 0; i < updates && i < 32; ++i)
            if (values[i] < 0 || values[i] > (completed ? 100.0f : 99.0f) ||
                (i && values[i] < values[i - 1])) return false;
        return true;
    }
};
static bool Write(HANDLE file, const void* data, unsigned int bytes) {
    DWORD done = 0;
    return WriteFile(file, data, bytes, &done, 0) && done == bytes;
}
}

extern "C" void HarnessEntry() {
    using namespace ProgressHost;
    using WinPackerNativeProgress::Progress;
    Progress progress;
    Host host;
    CHECK("unstarted state rejects stage records", !progress.OnLine(&host, "STAGE\trender") && !progress.Active() && host.updates == 0);
    progress.Finish(&host, true);
    CHECK("finish before begin emits no completion", progress.Percent() == 0 && host.updates == 0);
    progress.Begin(&host);
    CHECK("begin creates and activates exactly one main task", host.creates == 1 && host.activates == 1 && Same(host.mainName, "Native source compilation"));
    CHECK("begin explicitly initializes progress to zero", host.updates == 1 && host.values[0] == 0 && progress.Percent() == 0 && progress.Active());
    CHECK("begin callbacks create then activate then initialize progress", host.eventCount == 3 && host.events[0] == 1 && host.events[1] == 2 && host.events[2] == 4);
    progress.Begin(&host);
    CHECK("duplicate active begin does not reset or create a task", host.creates == 1 && host.updates == 1);
    const char* stages[8] = {"STAGE\tsource-capabilities", "STAGE\tworld-structure", "STAGE\tobjects-and-blind-data", "STAGE\trender", "STAGE\tphysics", "STAGE\tstreaming-and-resources", "STAGE\tassembly", "STAGE\tpublishing"};
    const unsigned int expected[8] = {0, 12, 32, 42, 65, 82, 94, 98};
    const char* labels[8] = {"Reading source and capabilities", "Building world geometry", "Compiling objects and keyframes", "Compiling render geometry", "Compiling physics", "Collecting resources", "Assembling world data", "Writing output files"};
    bool exact = true;
    for (unsigned int i = 0; i < 8; ++i)
        if (!progress.OnLine(&host, stages[i]) || progress.Percent() != expected[i] ||
            host.values[i + 1] != (float)expected[i] || !Same(host.label, labels[i])) exact = false;
    CHECK("all stage boundaries use their exact weights and human labels", exact);
    CHECK("stage changes preserve a single main task", host.creates == 1 && host.activates == 1 && host.subtasks == 8 && host.updates == 9 && !host.unexpected);
    bool order = host.eventCount == 19;
    for (unsigned int i = 0; i < 8; ++i)
        if (host.events[3 + 2 * i] != 3 || host.events[4 + 2 * i] != 4) order = false;
    CHECK("every subtask reset is immediately followed by global progress", order && host.displayed == 98);
    CHECK("stage progress is monotonic and below completion", host.Monotonic(false) && progress.Percent() == 98);
    unsigned int before = host.updates;
    CHECK("duplicate stage rejects without callback mutation", !progress.OnLine(&host, "STAGE\tpublishing") && host.updates == before && host.subtasks == 8);
    CHECK("backwards stage rejects without progress regression", !progress.OnLine(&host, "STAGE\trender") && host.updates == before && progress.Percent() == 98);
    CHECK("unknown stage does not manufacture progress", !progress.OnLine(&host, "STAGE\tdone-100") && host.updates == before);
    CHECK("null and empty input are ignored", !progress.OnLine(&host, 0) && !progress.OnLine(&host, "") && host.updates == before);
    CHECK("prefix and separator must match exactly", !progress.OnLine(&host, "stage\tassembly") && !progress.OnLine(&host, " STAGE\tassembly") && !progress.OnLine(&host, "STAGE assembly") && !progress.OnLine(&host, "STAGE\t") && host.updates == before);
    CHECK("extra payload cannot be interpreted as percentage", !progress.OnLine(&host, "STAGE\tpublishing\t100") && !progress.OnLine(&host, "STAGE\tpublishing ") && host.updates == before);
    char longLine[128];
    for (unsigned int i = 0; i < 128; ++i) longLine[i] = 'x';
    CHECK("bounded parser rejects a nonterminated oversized line", !progress.OnLine(&host, longLine) && host.updates == before);
    progress.Finish(&host, false);
    CHECK("failed finish preserves reached progress", !progress.Active() && progress.Percent() == 98 && host.updates == before);
    progress.Finish(&host, true);
    CHECK("a failed request cannot later become completed", progress.Percent() == 98 && host.updates == before);
    CHECK("finished request ignores late stage messages", !progress.OnLine(&host, "STAGE\tpublishing") && host.updates == before);
    Host second;
    progress.Begin(&second);
    CHECK("state can begin a new request after failure", progress.Active() && progress.Percent() == 0 && second.creates == 1 && second.updates == 1);
    CHECK("terminal CRLF is accepted", progress.OnLine(&second, "STAGE\tsource-capabilities\r\n") && second.subtasks == 1);
    CHECK("terminal LF is accepted", progress.OnLine(&second, "STAGE\tworld-structure\n") && progress.Percent() == 12);
    CHECK("terminal CR is accepted", progress.OnLine(&second, "STAGE\tobjects-and-blind-data\r") && progress.Percent() == 32);
    CHECK("embedded line breaks and repeated terminators reject", !progress.OnLine(&second, "STAGE\tren\nder") && !progress.OnLine(&second, "STAGE\trender\n\n") && !progress.OnLine(&second, "STAGE\trender\r\r"));
    CHECK("optional intermediate stages can be skipped", progress.OnLine(&second, "STAGE\tassembly") && progress.Percent() == 94);
    before = second.updates;
    progress.Finish(&second, true);
    CHECK("only successful finish emits completion", !progress.Active() && progress.Percent() == 100 && second.updates == before + 1 && second.values[before] == 100 && second.Monotonic(true));
    progress.Finish(&second, true);
    CHECK("repeated successful finish is idempotent", second.updates == before + 1);
    Host unavailable;
    unavailable.callbackResult = false;
    progress.Begin(&unavailable);
    CHECK("frontend callback failure does not corrupt recognized stage state", progress.OnLine(&unavailable, "STAGE\tphysics") && progress.Percent() == 65);
    progress.Finish(&unavailable, false);
    Progress quiet;
    quiet.Begin(0);
    CHECK("null frontend still tracks known stages", quiet.OnLine(0, "STAGE\trender") && quiet.Percent() == 42);
    quiet.Finish(0, true);
    CHECK("null frontend supports owned completion state", quiet.Percent() == 100 && !quiet.Active());
    char path[MAX_PATH];
    DWORD length = GetEnvironmentVariableA("FEAR_WINPACKER_PROGRESS_GUARDS_OUTPUT", path, MAX_PATH);
    if (!length || length >= MAX_PATH) ExitProcess(1);
    HANDLE file = CreateFileA(path, 0x40000000, FILE_SHARE_READ, 0, 2, FILE_ATTRIBUTE_NORMAL, 0);
    unsigned int header[2] = {0x31525057u, used};
    if (file == INVALID_HANDLE_VALUE || !Write(file, header, 8) || !Write(file, checks, used * 4)) ExitProcess(2);
    CloseHandle(file);
    for (unsigned int i = 0; i < used; ++i) if (!checks[i]) ExitProcess(3);
    ExitProcess(0);
}

void operator delete(void*, unsigned int) {}
extern "C" int __cdecl _purecall() { return 0; }
extern "C" { int _fltused = 0; }
extern "C" void* __cdecl memset(void* target, int value, unsigned int bytes) {
    unsigned char* data = (unsigned char*)target;
    while (bytes--) *data++ = (unsigned char)value;
    return target;
}
