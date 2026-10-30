@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-GUMZYY-CALL-FX.ps1"
if errorlevel 1 (
  echo.
  echo Installation failed. Please send a screenshot of this window.
  pause
  exit /b 1
)
echo.
echo GUMZYY CALL FX was installed. Restart Ableton Live or rescan VST3 plug-ins.
pause
