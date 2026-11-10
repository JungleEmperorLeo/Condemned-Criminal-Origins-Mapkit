Materials patch 1 - Materials tab, material-as-texture, Material Properties editor
===============================================================================

Extract this zip over the DEdit source root (the folder that contains "tools" and
"libs"), answering yes to overwrite. Then run build_dedit.bat as before.

New:      tools\DEdit\Materials\*.cpp/.h   (7 modules, all listed in build\dedit_sources.txt)
Changed:  streamsim.cpp/.h (both copies)   open hook so a .Mat00 opens as an in-memory texture
          ResourceMgr.cpp/.h               RESTYPE_MATERIAL; any folder holding .Mat00 files is a material folder
          MainFrm.cpp/.h, ProjectBar.cpp/.h, ProjectTabControlBar.cpp, EditHelpers.cpp/.h,
          PropPageOptionsMisc.cpp, DEdit.cpp, resource.h, dedit.rc, res\bitmap1.bmp
          build\build_dedit.bat, build\dedit_sources.txt

How to try it
  1. Open (or create) a project whose folder is the extracted game data root
     (the folder that contains Tex..., Materials..., Shaders...). 
  2. The new "Materials" tab (sphere icon) lists every folder that has .Mat00 files.
     The list shows each material's diffuse map as its picture and its shader name.
  3. Click a material = it becomes the current texture; apply it to brush polys the
     same way you apply a texture.
  4. Double click (or right click > Material Properties...) = Material Properties editor.
     OK writes the .Mat00 (the first original is kept next to it as .Mat00.bak).
  5. SurfaceFlags shows names if you point it at the game's GameServer.dll (button appears
     when SurfaceFlags is selected). Otherwise it is a number.

Not done yet in this patch: reading the compiled .fxo for descriptions / defaults / the
"?" link (the editor lists the parameters stored in the material), saving levels in
Condemned form, packing.
