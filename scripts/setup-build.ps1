# Platformer Saver — one-shot build environment setup (Windows)
# Run in PowerShell. VS Build Tools step may need admin / UAC approval.

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$GdPath = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
$PortableCmake = "$env:USERPROFILE\tools\cmake-3.31.6-windows-x86_64\bin"

Write-Host "=== Platformer Saver build setup ===" -ForegroundColor Cyan

function Ensure-Command($name, $wingetId, $postInstall) {
    if (Get-Command $name -ErrorAction SilentlyContinue) {
        Write-Host "[OK] $name" -ForegroundColor Green
        return
    }
    Write-Host "[..] Installing $name via winget..." -ForegroundColor Yellow
    winget install $wingetId --accept-package-agreements --accept-source-agreements
    if ($postInstall) { & $postInstall }
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        Write-Host "[!!] Restart PowerShell after install, then re-run this script." -ForegroundColor Red
    }
}

Ensure-Command "geode" "GeodeSDK.GeodeCLI" { $null }
Ensure-Command "ninja" "Ninja-build.Ninja" { $null }

if (-not (Test-Path $PortableCmake\cmake.exe)) {
    Write-Host "[..] Downloading portable CMake 3.31.6..." -ForegroundColor Yellow
    $zip = "$env:TEMP\cmake-portable.zip"
    $dest = "$env:USERPROFILE\tools"
    New-Item -ItemType Directory -Force -Path $dest | Out-Null
    Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-windows-x86_64.zip" -OutFile $zip
    Expand-Archive -Path $zip -DestinationPath $dest -Force
}

$vcvars = @(
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $vcvars) {
    Write-Host "[!!] MSVC not found. Launching VS Build Tools installer (approve UAC if prompted)..." -ForegroundColor Yellow
    $installer = "$env:TEMP\vs_BuildTools.exe"
    if (-not (Test-Path $installer)) {
        Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_buildtools.exe" -OutFile $installer
    }
    Start-Process -FilePath $installer -ArgumentList @(
        "install",
        "--add", "Microsoft.VisualStudio.Workload.VCTools",
        "--includeRecommended",
        "--passive",
        "--wait",
        "--norestart"
    ) -Wait -Verb RunAs
    Write-Host "Re-run this script after the VS installer finishes." -ForegroundColor Yellow
    exit 1
}

Write-Host "[OK] MSVC: $vcvars" -ForegroundColor Green

if (-not $env:GEODE_SDK) {
    Write-Host "[..] Installing Geode SDK..." -ForegroundColor Yellow
    $sdkPath = "$env:USERPROFILE\Documents\Geode"
    if (-not (Test-Path $sdkPath)) {
        echo y | geode sdk install $sdkPath
    }
    geode sdk install-binaries
    $env:GEODE_SDK = $sdkPath
}

if (-not (geode profile list 2>&1 | Select-String "default")) {
    Write-Host "[..] Adding Geode profile..." -ForegroundColor Yellow
    geode profile add --name default $GdPath
}

$env:Path = "$PortableCmake;$env:Path"

Write-Host "[..] Building mod..." -ForegroundColor Yellow
Push-Location $ProjectRoot
cmd /c "`"$vcvars`" && set PATH=$PortableCmake;%PATH% && geode build --ninja"
$built = Get-ChildItem "$ProjectRoot\build" -Filter "*.geode" -ErrorAction SilentlyContinue | Select-Object -First 1
Pop-Location

if ($built) {
    Write-Host "[OK] Built: $($built.FullName)" -ForegroundColor Green
    Write-Host "Launch GD: geode profile run" -ForegroundColor Cyan
} else {
    Write-Host "[!!] Build failed. Check output above." -ForegroundColor Red
    exit 1
}
