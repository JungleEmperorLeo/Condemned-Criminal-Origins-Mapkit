# Source provenance

The algorithm files in this directory originate from the user-provided prior-generation source tree:

`E:/LithTechSC/Jupiter_Ent/Jupiter_Ent/Engine - Copy/tools/PreProcessor/Packer_PC/NvTriStrip/`

Retained algorithm files: `NvTriStrip.cpp`, `NvTriStrip.h`, `NvTriStripObjects.cpp`, `NvTriStripObjects.h`, `VertexCache.cpp`, `VertexCache.h`.

Adaptations in the rebuild:

- Replace SDK includes and STL containers with the limited `NvCompat` scalar/pointer/POD containers and small `bdefs.h`/`processing.h` shims.
- Apply FEAR's observed PT_LIST filtering of stripifier-generated degenerate faces at RVAs `0x7DB9E` / `0x7DC08`, decrementing the output index count by three. The prior-generation implementation omitted this.
- Replace the static scratch index vector with a local vector. Logical contents and ordering are unchanged; this avoids process-lifetime destructor registration in the native runtime.
- The caller selects cache size 24 and lists-only output. FEAR's wrapper falls back to filtered input whenever stripifier output has a different length.

`RenderIndexReorder.h` is the rebuild wrapper. It supports U16-range indices, removes repeated-index triangles using the original U32 comparisons before conversion, and retains a safe empty-result extension. Direct empty input is outside the original helper's supported precondition; no empty-original parity is claimed.

The native generator has no original DLL dependency. `verify-render-index-reorder.py` loads the original only in a separate comparison executable.
