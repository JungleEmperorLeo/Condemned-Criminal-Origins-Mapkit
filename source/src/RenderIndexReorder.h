#pragma once
#include "legacy-nvtristrip/NvCompat.h"
#include "legacy-nvtristrip/NvTriStrip.h"

// Static source implementation of the optional RVA 0x49740 pass, verified
// independently against the original and complete captured node index streams.
// The older Jupiter NvTriStrip source is retained as a separately attributable
// dependency. It is not an original-DLL call or a source-map-specific lookup.
// Supported section indices are uint16; runtime mesh declaration/splitting is
// separate. Default original strip settings are cache24 and lists-only=true.
namespace RenderIndexReorder {
struct Triangle { unsigned int indices[3]; };
inline bool Section(const Triangle* triangles,unsigned int count,NvCompat::Vector<Triangle>& output) {
    if ((!triangles && count)||count>100000) return false;
    NvCompat::Vector<unsigned short> input;
    input.reserve(count*3);
    for(unsigned int i=0;i<count;++i) {
        const unsigned int* v=triangles[i].indices;
        // The original compares uint32 values before its ushort conversion.
        if(v[0]==v[1] || v[0]==v[2] || v[1]==v[2]) continue;
        for(unsigned int j=0;j<3;++j) {
            if(v[j]>65535) return false; // source section index range contract
            input.push_back((unsigned short)v[j]);
        }
    }
    output.clear();
    if(input.empty()) return true;
    SetCacheSize(24);
    SetListsOnly(true);
    PrimitiveGroup* groups=0;
    unsigned short nGroups=0;
    GenerateStrips(input.data(),(unsigned int)input.size(),&groups,&nGroups);
    bool valid=nGroups==1 && groups && groups[0].type==PT_LIST && groups[0].numIndices%3==0;
    if(valid) {
        const unsigned short* indices=groups[0].indices;
        unsigned int countIndices=groups[0].numIndices;
        // Original falls back to the filtered pre-stripifier indices whenever
        // the generated list has a different length.
        if(countIndices!=input.size()) {
            indices=input.data();countIndices=(unsigned int)input.size();
        }
        output.resize(countIndices/3);
        for(unsigned int i=0;i<countIndices;++i) output[i/3].indices[i%3]=indices[i];
    }
    delete[] groups;
    return valid;
}
}
