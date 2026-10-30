$ErrorActionPreference = 'Stop'

# The common VST3 folder is protected. Re-launch this same installer elevated,
# wait for it to finish, and return the elevated process exit code to the BAT.
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = [Security.Principal.WindowsPrincipal]::new($identity)
$isAdmin = $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host 'Requesting administrator permission to install the VST3...' -ForegroundColor Yellow
    try {
        $arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
        $elevated = Start-Process -FilePath 'powershell.exe' -Verb RunAs -ArgumentList $arguments -Wait -PassThru
        exit $elevated.ExitCode
    }
    catch {
        Write-Error 'Administrator permission was not granted. The plugin was not installed.'
        exit 1
    }
}

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
if (-not (Test-Path $destination)) { throw 'The VST3 bundle was not found at the install destination after copying.' }
Write-Host "Installed to: $destination" -ForegroundColor Green
Write-Host 'Restart Ableton Live or rescan VST3 plug-ins.' -ForegroundColor Cyan
