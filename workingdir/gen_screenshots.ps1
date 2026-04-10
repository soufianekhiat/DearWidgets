# gen_screenshots.ps1 — Generate DearWidgets demo screenshots
# Usage: .\gen_screenshots.ps1 [-OutDir <path>] [-Width <int>] [-NoHeaders]

param(
    [string]$OutDir,
    [int]$Width = 0,
    [switch]$NoHeaders
)

$ErrorActionPreference = 'Stop'

# Default output directory
if (-not $OutDir) {
    $OutDir = Join-Path $PSScriptRoot '..\docs\screenshots'
}
$OutDir = [System.IO.Path]::GetFullPath($OutDir)

Write-Host "Generating screenshots into: $OutDir"
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
}

# Build arguments
$exe = Join-Path $PSScriptRoot 'dearwidgetsdemo.exe'
if (-not (Test-Path $exe)) {
    Write-Error "Demo executable not found: $exe"
    exit 1
}

$args = @('--screenshot', $OutDir)
if ($NoHeaders) { $args += '--no-headers' }
if ($Width -gt 0) { $args += '--width'; $args += "$Width" }

Write-Host "Running: $exe $($args -join ' ')"
& $exe @args
$exitCode = $LASTEXITCODE

if ($exitCode -ne 0) {
    Write-Error "Demo exited with code $exitCode"
    exit $exitCode
}

Write-Host "`nScreenshots written:"
Get-ChildItem -Path $OutDir -Filter '*.png' | ForEach-Object { Write-Host "  $($_.Name)" }
