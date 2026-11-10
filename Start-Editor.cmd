@echo off
if not exist "%~dp0Runtime\FEAR.proj00" (echo Run Setup.cmd first. & pause & exit /b 1)
rem refresh the editor's project path (fixes a moved folder) and report problems
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Check.ps1" -Quiet
if errorlevel 1 (echo. & echo The kit installation has problems. Running the full check: & powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Check.ps1" & pause & exit /b 1)
cd /d "%~dp0Tools"
set MAP=%~1
if "%MAP%"=="" set MAP=%~dp0Runtime\Game\Worlds\123.World00c
start "" "%~dp0Tools\WorldEdit_Condemned_v4.exe" "%MAP%"

