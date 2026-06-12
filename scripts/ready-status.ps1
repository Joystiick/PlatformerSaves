# Quick status — run anytime
$gd = "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"
Write-Host "=== Platformer Saver readiness ===" -ForegroundColor Cyan

$checks = @(
    @{ Name = "Geometry Dash"; Ok = (Test-Path "$gd\GeometryDash.exe") },
    @{ Name = "Geode loader"; Ok = (Test-Path "$gd\geode") },
    @{ Name = "Geode mods folder"; Ok = (Test-Path "$gd\geode\mods") },
    @{ Name = "Geode CLI"; Ok = [bool](Get-Command geode -ErrorAction SilentlyContinue) },
    @{ Name = "Geode SDK"; Ok = [bool]$env:GEODE_SDK -or (Test-Path "$env:USERPROFILE\OneDrive\Documents\Geode") },
    @{ Name = "CMake (portable)"; Ok = (Test-Path "$env:USERPROFILE\tools\cmake-3.31.6-windows-x86_64\bin\cmake.exe") },
    @{ Name = "Ninja"; Ok = [bool](Get-Command ninja -ErrorAction SilentlyContinue) },
    @{ Name = "MSVC (cl.exe)"; Ok = [bool](Get-ChildItem "C:\Program Files*" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue | Select-Object -First 1) }
)

foreach ($c in $checks) {
    $color = if ($c.Ok) { "Green" } else { "Yellow" }
    $mark = if ($c.Ok) { "OK" } else { "MISSING" }
    Write-Host ("  [{0}] {1}" -f $mark, $c.Name) -ForegroundColor $color
}

$deps = @("sabe.persistenceapi", "geode.node-ids", "geode.custom-keybinds")
Write-Host "`nDependencies in GD:" -ForegroundColor Cyan
foreach ($d in $deps) {
    $found = Get-ChildItem "$gd\geode\mods\$d*.geode" -ErrorAction SilentlyContinue | Select-Object -First 1
    Write-Host ("  [{0}] {1}" -f $(if ($found) { "OK" } else { "MISSING" }), $d) -ForegroundColor $(if ($found) { "Green" } else { "Yellow" })
}

$geode = Get-ChildItem "c:\Users\lachi\Downloads\Platformer Saver\build" -Filter "*.geode" -ErrorAction SilentlyContinue | Select-Object -First 1
Write-Host "`nBuilt mod:" -ForegroundColor Cyan
if ($geode) {
    Write-Host "  [OK] $($geode.FullName)" -ForegroundColor Green
} else {
    Write-Host "  [MISSING] Run scripts\install-vs-admin.bat as Admin, then scripts\setup-build.ps1" -ForegroundColor Yellow
}
