@echo off
rem Compile a Condemned map with the Condemned WorldPacker (writes X.World00p, X.WorldClient00p, X.WorldServer00p beside the source).
rem Usage: Build-Map.cmd "<path>\MyMap.World00c"   (or drag the file onto this script). The map must be inside Runtime\Game\Worlds.
if not exist "%~dp0Runtime\FEAR.proj00" (echo Run Setup.cmd first. & pause & exit /b 1)
if "%~1"=="" (echo Drag a .World00c or .World00a file onto Build-Map.cmd, or pass its path. & pause & exit /b 1)
cd /d "%~dp0Tools"
"%~dp0Tools\winpacker.exe" -file "%~f1" -ProjectPath "%~dp0Runtime\Game" -PackerDir "%~dp0Tools\packCDM" -SkipPropDlg -NoPauseWhenDone
echo.
echo Output files beside "%~f1" (World00p / WorldClient00p / WorldServer00p). The exit code 200 of winpacker is normal.
pause
