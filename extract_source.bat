@echo off
rem Unpacks jupiter-source.zip (Jupiter source + terrain/lighting changes) into this folder.
if not exist "%~dp0jupiter-source.zip" (
  echo ERROR: jupiter-source.zip not found next to this file.
  exit /b 1
)
echo Unpacking source...
powershell -NoProfile -Command "Expand-Archive -LiteralPath '%~dp0jupiter-source.zip' -DestinationPath '%~dp0.' -Force"
if errorlevel 1 (
  echo ERROR: unpack failed.
  exit /b 1
)
echo Done.
exit /b 0
