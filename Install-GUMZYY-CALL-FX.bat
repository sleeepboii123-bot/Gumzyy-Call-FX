@echo off
setlocal
title GUMZYY CALL FX Installer
echo GUMZYY CALL FX - Windows VST3 Installer
echo.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-GUMZYY-CALL-FX.ps1"
if errorlevel 1 (
  echo.
  echo Installation failed or administrator permission was not granted.
  pause
  exit /b 1
)
echo.
echo GUMZYY CALL FX was installed successfully.
echo Restart Ableton Live or rescan VST3 plug-ins.
pause
