DEdit for Jupiter: Terrain tools + Lighting Preview
===================================================

Ported from the LithTech 1.0 DEdit source onto jsj2008/lithtech (commit 0eab182).
This folder is the Jupiter source (the parts DEdit needs: tools\DEdit, tools\shared,
tools\PreProcessor, sdk and the libs it uses) with the changes already applied.
The source ships as jupiter-source.zip; build_dedit.bat (or extract_source.bat) unpacks it
here on first run, overwriting the earlier loose copies of the changed files.
ltj-terrain-lightpreview.patch is the same change as a patch against the original
repo (git apply, from the repo root).

BUILD
-----
Needs Visual Studio 2017+ or Build Tools with "Desktop development with C++" and the
component "C++ MFC for latest v14x build tools (x86 & x64)".

    build_dedit.bat            -> build\dedit\release\DEdit.exe
    build_dedit.bat rebuild    recompile everything
    build_dedit.bat debug      debug build

Jupiter has no DEdit project, so the script compiles the files listed in
tools\DEdit\build\dedit_sources.txt with cl and links them (32-bit). Files that already
compiled are skipped next run. Errors are collected in build\dedit\<config>\errors.txt
(full log: build.log). If the link reports an unresolved symbol, add the .cpp that
defines it to dedit_sources.txt.
Build-only additions: tools\DEdit\Misc\DEditStubs.cpp, tools\DEdit\build\compat\*.h,
a DECIMAL->LTA_DECIMAL rename in libs\ltamgr\ltaconverter.*, and a DX6 define in
draw\draw_d3d.h (d3d9.h otherwise empties d3d.h).

NEW FILES
---------
tools\DEdit\Terrain\Terrain.cpp/.h          Terrain tools
tools\DEdit\Lightmap\LightPreview.cpp/.h    Lighting preview

CHANGED FILES (small hooks)
---------------------------
Views\RegionView.cpp/.h   menu commands, mouse hooks for painting, light toggle
draw\draw_d3d.cpp/.h      modulates a poly's lightmap over its texture
draw\DrawBase.cpp         brush reticle for the terrain tool
Docs\RegionDoc.cpp        drops the preview when its level closes
dedit.rc, resource.h      Terrain Tools / Convert dialogs, menu items, Ctrl+L, F9

USING IT
--------
Terrain
  1. Select one brush; Brush > Convert to Terrain...; pick a grid (2x2, 4x4, 8x8).
     The brush's footprint becomes a "TerrainNNNN" group of triangular prism
     brushes (its top is the starting height, its bottom the flat base).
  2. Brush > Terrain Tools (F9). Modes: Raise, Lower, Smooth, Flatten, and Paint
     layers 1-4. Left-drag on the terrain. Fill in the four layer textures
     (the "Current" buttons use the texture selected in the texture tree) and
     press Apply Layers & Rebake.
  3. Painted layers are baked into Textures\Terrainmesh\NNNN.dtx (32-bit DTX,
     256x256, with mipmaps). The layer weights are kept in NNNN.trn next to it.
     Keep both files with the level. Terrain made by the LithTech 1.0 editor
     (.trn files) loads fine.
  While the tool window is open, left clicks don't select (I/O/L/Space still
  work for navigation).

Lighting preview
  View > Lighting Preview (Ctrl+L). It recalculates every time you turn it on
  (it doesn't track edits made while it is on). Lights come from Light and
  DirLight objects, using their LightRadius, LightColor/InnerColor, OuterColor,
  AttCoefs/AttExps, BrightScale, FOV and ClipLight, plus the world's AmbientLight.
  Brushes show it when Lighting is Lightmap, Gouraud or ShadowMesh and ReceiveLight
  is on; Flat brushes, sky and non-rendered types keep the normal editor shading.
  It uses the same formulas as the editor's built-in lightmap sample generator,
  so it approximates the Processor rather than matching it exactly.

DIFFERENCES FROM THE LITHTECH 1.0 VERSION
-----------------------------------------
* Textures are read and written with Jupiter's dtx_* / FormatMgr code (any DTX
  format DEdit can display works as a layer; the baked texture is 32-bit).
  The LT1 palette quantizer and Blood 2 surface-type default are gone.
* The preview uses Jupiter's light properties and brush Type/Lighting properties,
  and a plain MODULATE blend (LT1 lightmaps were over-bright, 128 = full).
* While the preview is on, a lit poly's detail texture is not shown (the second
  texture stage is used for the lightmap).

NOT PORTED (not asked for): LT1 .ed/.dat import and export, the LT1 texture
properties dialog, and the Hollow fix / Carve command from the LT1 source.

UPDATES
-------
* Lighting preview now follows the Processor (LightMapMaker.cpp): "Attenuation"
  (Quartic default / Linear / D3D), LightColor or InnerColor x |BrightScale|, Lambert,
  DirLight cone, AmbientLight. No sun light or brush ambient yet.
* Terrain: lighting samples are kept inside each triangle, and terrain top faces use
  smoothed normals. Convert to Terrain asks whether to build one-sided triangles
  (single upward face per brush, no sides/bottom) or closed prisms. Terrain made as
  either kind can be sculpted/painted.
* Classic (Windows 95/98 style) look: visual styles are switched off for DEdit.
  Create a file called modern_ui.txt next to DEdit.exe to turn it back on.
* DEdit also loads object modules whose DLLs are in the game folder (LT1Compat loader).


CREDITS
-------
Ihor - World processor for Condemned
