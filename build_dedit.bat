@echo off
rem Builds DEdit. Output: build\dedit\release\DEdit.exe  (options: rebuild, debug)
rem On first run the source is unpacked from jupiter-source.zip.
if not exist "%~dp0tools\DEdit\build\build_dedit.bat" call "%~dp0extract_source.bat"
if errorlevel 1 exit /b 1
call "%~dp0tools\DEdit\build\build_dedit.bat" %*
exit /b %ERRORLEVEL%
