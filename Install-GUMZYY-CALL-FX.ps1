$ErrorActionPreference = 'Stop'
$plugin = Get-ChildItem -Path $PSScriptRoot -Directory -Filter '*.vst3' | Select-Object -First 1
if (-not $plugin) { throw 'No .vst3 bundle was found next to this installer. Extract the full ZIP first.' }

$destinationRoot = Join-Path $env:CommonProgramFiles 'VST3'
New-Item -ItemType Directory -Force -Path $destinationRoot | Out-Null
$destination = Join-Path $destinationRoot $plugin.Name

Write-Host 'Installing GUMZYY CALL FX...' -ForegroundColor Magenta
if (Test-Path $destination) {
    $backup = "$destination.backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    Move-Item -LiteralPath $destination -Destination $backup
    Write-Host "Existing copy moved to: $backup" -ForegroundColor Yellow
}
Copy-Item -LiteralPath $plugin.FullName -Destination $destination -Recurse -Force
Write-Host "Installed to: $destination" -ForegroundColor Green
