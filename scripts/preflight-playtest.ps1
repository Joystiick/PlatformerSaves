# Preflight checks before manual play-testing
$ErrorActionPreference = "Continue"

Write-Host "=== Platformer Saver play-test preflight ===" -ForegroundColor Cyan

$gdPath = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
$geodeInGd = Test-Path "$gdPath\geode"
$gdExe = Test-Path "$gdPath\GeometryDash.exe"

Write-Host "Geometry Dash: $(if ($gdExe) { 'OK' } else { 'MISSING' })"
Write-Host "Geode loader in GD: $(if ($geodeInGd) { 'OK' } else { 'MISSING - install Geode in GD first' })"

if (Get-Command geode -ErrorAction SilentlyContinue) {
    Write-Host "`nGeode CLI profiles:"
    geode profile list
    Write-Host "`nSDK version:"
    geode sdk version
} else {
    Write-Host "Geode CLI: NOT ON PATH (restart terminal after winget install)"
}

$geodePackage = Get-ChildItem "c:\Users\lachi\Downloads\Platformer Saver\build" -Filter "*.geode" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($geodePackage) {
    Write-Host "`nBuilt mod: $($geodePackage.FullName)" -ForegroundColor Green
} else {
    Write-Host "`nBuilt mod: NOT FOUND - run 'geode build' first (requires VS 2022 C++ Build Tools)" -ForegroundColor Yellow
}

Write-Host "`nNext: install deps in-game, then follow TESTING.md"
Write-Host "Launch GD: geode profile run"
