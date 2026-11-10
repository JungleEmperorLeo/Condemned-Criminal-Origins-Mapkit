@echo off
setlocal EnableExtensions

rem ======================================================================
rem  build_dedit.bat - builds DEdit (32-bit) with Visual Studio build tools.
rem
rem  Usage:   build_dedit.bat [rebuild] [debug]
rem             rebuild  recompile every file (default: skip files that
rem                      already have an object file)
rem             debug    debug build (default: release)
rem
rem  Needs:   Visual Studio 2017 or newer (or the Build Tools) with the
rem           "Desktop development with C++" workload and the component
rem           "C++ MFC for latest v14x build tools (x86 & x64)".
rem
rem  Output:  <repo>\build\dedit\<config>\DEdit.exe
rem ======================================================================

set "REBUILD=0"
set "CONFIG=release"
for %%A in (%*) do call :parseArg %%A

for %%I in ("%~dp0..\..\..") do set "ROOT=%%~fI"
set "DEDIT=%ROOT%\tools\DEdit"
set "SRCLIST=%~dp0dedit_sources.txt"
set "OUT=%ROOT%\build\dedit\%CONFIG%"
set "OBJ=%OUT%\obj"
set "LOG=%OUT%\build.log"

if not exist "%SRCLIST%" goto :noSrcList

rem ---- find Visual Studio ------------------------------------------------
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :noVS
set "VSPATH="
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%I"
if not defined VSPATH goto :noVS

call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul
if errorlevel 1 goto :noVS

if not exist "%VCToolsInstallDir%atlmfc\include\afxwin.h" goto :noMFC

rem ---- output folders ----------------------------------------------------
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OBJ%" mkdir "%OBJ%"
if exist "%LOG%" del "%LOG%"

rem ---- compiler settings -------------------------------------------------
set "INC="
for %%D in (
  tools\DEdit
  tools\DEdit\Controls tools\DEdit\Dlgs tools\DEdit\Docs tools\DEdit\draw
  tools\DEdit\FileFormats tools\DEdit\Frames tools\DEdit\KeyAggregate
  tools\DEdit\LevelError tools\DEdit\Lightmap tools\DEdit\Misc tools\DEdit\Terrain tools\DEdit\Materials
  tools\DEdit\ModelView tools\DEdit\MRCEXT tools\DEdit\MRCEXT\VC_HEADERS
  tools\DEdit\Options tools\DEdit\Project tools\DEdit\RegionEdit
  tools\DEdit\Trackers tools\DEdit\Views
  tools\shared\world tools\shared\engine tools\shared\model
  sdk\inc sdk\inc\physics sdk\inc\sys\win
  libs\stdlith libs\lith libs\dibmgr libs\ButeMgr libs\MFCStub libs\rezmgr
  libs\ltamgr libs\genregmgr libs\RegMgr libs\CryptMgr
  tools\PreProcessor
  tools\DEdit\build\compat
  libs\zlib
) do call :addInc %%D

set "DEFS=/DWIN32 /D_WINDOWS /D_AFXDLL /D_MBCS /DDIRECTEDITOR_BUILD /DPREPROCESSOR_BUILD /DPRE_FLOAT /D_D3D9_H_ /D_d3d9TYPES_H_ /D_d3d9CAPS_H /DBDEFS_MFC /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_CRT_OBSOLETE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS"
if /i "%CONFIG%"=="debug" (
  set "OPT=/Od /Zi /MDd /D_DEBUG"
) else (
  set "OPT=/O2 /MD /D_FINAL"
)
rem The code base is VC6-era: old for-scope rules, permissive conversions.
set "CFLAGS=/nologo /c /EHsc /W1 /std:c++14 /permissive /Zc:forScope- /FIwinsock2.h /wd4996 /wd4786 /wd4056 /FS %OPT% %DEFS% %INC%"
if "%TRACE%"=="1" set "CFLAGS=%CFLAGS% /showIncludes"

rem One-time cleanup: ltaconverter.h changed, so its object must be rebuilt.
if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%OUT%\fix_ltaconverter.done" (
  if exist "%OBJ%\libs_ltamgr_ltaconverter.obj" del /f /q "%OBJ%\libs_ltamgr_ltaconverter.obj"
  echo done> "%OUT%\fix_ltaconverter.done"
)

rem Objects listed in build\dedit\<config>\stale.txt (one name per line) are
rem deleted so their sources get recompiled; the list is then removed.
if exist "%OUT%\stale.txt" (
  for /f "usebackq tokens=*" %%N in ("%OUT%\stale.txt") do if exist "%OBJ%\%%N" del /f /q "%OBJ%\%%N"
  del /f /q "%OUT%\stale.txt"
)

rem ---- drop objects whose source file has changed since they were built ----
rem ---- only objects whose source or included headers changed are rebuilt
if "%REBUILD%"=="0" powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0stale_check.ps1" -Root "%ROOT%" -SrcList "%SRCLIST%" -Obj "%OBJ%"

rem ---- compile -----------------------------------------------------------
set /a TOTAL=0
set /a SKIPPED=0
set /a FAILED=0
echo Building DEdit (%CONFIG%)...
for /f "usebackq eol=# tokens=*" %%S in ("%SRCLIST%") do call :compile "%%S"
echo.
echo Compiled: %TOTAL%   Up to date: %SKIPPED%   Failed: %FAILED%
if %FAILED% GTR 0 goto :compileFailed

rem ---- resources ---------------------------------------------------------
pushd "%DEDIT%"
rc /nologo /D_AFXDLL /I"%DEDIT%" /I"%DEDIT%\res" /fo"%OUT%\dedit.res" dedit.rc >> "%LOG%" 2>&1
set "RCERR=%ERRORLEVEL%"
popd
if not "%RCERR%"=="0" goto :rcFailed

rem ---- link --------------------------------------------------------------
if exist "%OUT%\link.rsp" del "%OUT%\link.rsp"
for /f "usebackq eol=# tokens=*" %%S in ("%SRCLIST%") do call :addObj "%%S"
set "LINKDBG="
if /i "%CONFIG%"=="debug" set "LINKDBG=/DEBUG"
link /nologo /SUBSYSTEM:WINDOWS /MACHINE:X86 /SAFESEH:NO /NODEFAULTLIB:LIBC.lib /NODEFAULTLIB:LIBCD.lib %LINKDBG% /OUT:"%OUT%\DEdit.exe" /MAP:"%OUT%\DEdit.map" /LIBPATH:"%ROOT%\tools\shared\world" /LIBPATH:"%ROOT%\sdk\lib" @"%OUT%\link.rsp" "%OUT%\dedit.res" s3tc.lib legacy_stdio_definitions.lib winmm.lib ddraw.lib dxguid.lib version.lib comctl32.lib imm32.lib ws2_32.lib TdGuard.lib shlwapi.lib ole32.lib >> "%LOG%" 2>&1
if errorlevel 1 goto :linkFailed

echo.
echo Built: %OUT%\DEdit.exe
exit /b 0


rem ======================================================================
:parseArg
if /i "%~1"=="rebuild" set "REBUILD=1"
if /i "%~1"=="debug" set "CONFIG=debug"
if /i "%~1"=="trace" set "TRACE=1"
exit /b 0

:addInc
set "INC=%INC% /I"%ROOT%\%1""
exit /b 0

:addObj
set "SRC=%~1"
set "NAME=%SRC:\=_%"
set "NAME=%NAME:.cpp=%"
set "NAME=%NAME:.c=%"
echo "%OBJ%\%NAME%.obj">> "%OUT%\link.rsp"
exit /b 0

:compile
set "SRC=%~1"
set "NAME=%SRC:\=_%"
set "NAME=%NAME:.cpp=%"
set "NAME=%NAME:.c=%"
set "OBJFILE=%OBJ%\%NAME%.obj"
if "%REBUILD%"=="0" if exist "%OBJFILE%" (
  set /a SKIPPED+=1
  exit /b 0
)
echo   %SRC%
cl %CFLAGS% /Fo"%OBJFILE%" "%ROOT%\%SRC%" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo   ** FAILED: %SRC%
  set /a FAILED+=1
) else (
  set /a TOTAL+=1
)
exit /b 0

:extractErrors
findstr /R /C:" error [CLR][A-Z]*[0-9][0-9]*" /C:"fatal error" "%LOG%" > "%OUT%\errors.txt"
powershell -NoProfile -Command "Get-Content -LiteralPath '%OUT%\errors.txt' -TotalCount 30"
exit /b 0

:noSrcList
echo ERROR: source list not found: %SRCLIST%
exit /b 1

:noVS
echo ERROR: Visual Studio with the C++ x86 tools was not found.
echo Install "Visual Studio Build Tools" and select "Desktop development with C++".
exit /b 1

:noMFC
echo ERROR: MFC is not installed for this Visual Studio.
echo In the Visual Studio Installer add the component
echo   "C++ MFC for latest v14x build tools (x86 & x64)".
exit /b 1

:compileFailed
call :extractErrors
echo.
echo Compile errors (first 30 shown) are in:
echo   %OUT%\errors.txt
echo Files that already compiled are skipped on the next run.
exit /b 1

:rcFailed
echo ERROR: resource compile failed. See %LOG%
exit /b 1

:linkFailed
call :extractErrors
echo.
echo Link failed. Errors are in %OUT%\errors.txt
echo Unresolved symbol: add the .cpp that defines it to build\dedit_sources.txt.
echo Duplicate symbol : remove one of the duplicate files from that list.
exit /b 1
