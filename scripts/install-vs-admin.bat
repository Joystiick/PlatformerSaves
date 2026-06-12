@echo off
REM Run as Administrator: right-click -> Run as administrator
echo Installing Visual Studio 2022 Build Tools (C++ workload)...
echo This takes several minutes. Do not close this window.

set INSTALLER=%TEMP%\vs_BuildTools.exe
if not exist "%INSTALLER%" (
    powershell -NoProfile -Command "Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vs_buildtools.exe' -OutFile '%INSTALLER%'"
)

"%INSTALLER%" install ^
  --add Microsoft.VisualStudio.Workload.VCTools ^
  --includeRecommended ^
  --passive ^
  --wait ^
  --norestart

if exist "%ProgramFiles(x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    echo.
    echo [OK] Build Tools installed. Now run:
    echo   powershell -File scripts\setup-build.ps1
) else (
    echo.
    echo [!!] Install may still be running or failed. Open "Visual Studio Installer" from Start Menu to check.
)

pause
