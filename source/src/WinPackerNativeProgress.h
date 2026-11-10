#pragma once
#include "IPackerOutput.h"

namespace WinPackerNativeProgress {

// Progress marks reached worker boundaries. It does not estimate elapsed work.
class Progress {
    int stage;
    unsigned int percent;
    bool active;

    struct Stage {
        const char* name;
        const char* label;
        unsigned int percent;
    };

    static const Stage* Stages() {
        static const Stage stages[] = {
            {"source-capabilities", "Reading source and capabilities", 0},
            {"world-structure", "Building world geometry", 12},
            {"objects-and-blind-data", "Compiling objects and keyframes", 32},
            {"render", "Compiling render geometry", 42},
            {"physics", "Compiling physics", 65},
            {"streaming-and-resources", "Collecting resources", 82},
            {"assembly", "Assembling world data", 94},
            {"publishing", "Writing output files", 98}
        };
        return stages;
    }

    static bool Equal(const char* text, unsigned int length, const char* name) {
        unsigned int i = 0;
        for (; i < length; ++i) if (!name[i] || text[i] != name[i]) return false;
        return name[i] == 0;
    }

public:
    Progress() : stage(-1), percent(0), active(false) {}

    void Begin(IPackerOutput* output) {
        if (active) return;
        stage = -1;
        percent = 0;
        active = true;
        if (output) {
            output->CreateTask("Native source compilation");
            output->ActivateTask("Native source compilation");
            output->UpdateProgress(0);
        }
    }

    // Returns whether this was a recognized, increasing stage boundary.
    // Malformed, unknown, duplicate and backwards records leave state untouched.
    bool OnLine(IPackerOutput* output, const char* line) {
        if (!active || !line) return false;
        unsigned int length = 0;
        for (; length < 96 && line[length]; ++length) {}
        if (length == 96) return false;
        if (length && line[length - 1] == '\n') --length;
        if (length && line[length - 1] == '\r') --length;
        if (length < 7 || line[0] != 'S' || line[1] != 'T' ||
            line[2] != 'A' || line[3] != 'G' || line[4] != 'E' ||
            line[5] != '\t') return false;
        const Stage* stages = Stages();
        for (unsigned int i = 0; i < 8; ++i) {
            if (!Equal(line + 6, length - 6, stages[i].name)) continue;
            if ((int)i <= stage) return false;
            stage = (int)i;
            percent = stages[i].percent;
            if (output) {
                output->ActivateSubTask(stages[i].label);
                output->UpdateProgress((float)percent);
            }
            return true;
        }
        return false;
    }

    void Finish(IPackerOutput* output, bool success) {
        if (!active) return;
        active = false;
        if (success) {
            percent = 100;
            if (output) output->UpdateProgress(100);
        }
    }

    unsigned int Percent() const { return percent; }
    bool Active() const { return active; }
};

}
