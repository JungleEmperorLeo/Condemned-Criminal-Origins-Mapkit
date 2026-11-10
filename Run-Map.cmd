@echo off
rem Start Condemned on a compiled map. Usage: Run-Map.cmd Worlds\MyMap   (path relative to Runtime\Game, no extension)
rem Without an argument a file picker opens.
if not exist "%~dp0game-install.txt" (echo Run Setup.cmd first. & pause & exit /b 1)
if "%~1"=="" (start "" "%~dp0Runtime\CondemnedTestLauncher.exe") else (start "" "%~dp0Runtime\CondemnedTestLauncher.exe" "%~1")
