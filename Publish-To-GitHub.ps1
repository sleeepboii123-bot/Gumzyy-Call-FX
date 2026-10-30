$ErrorActionPreference = 'Stop'
$repo = 'sleeepboii123-bot/Gumzyy-Call-FX'
$source = $PSScriptRoot
$work = Join-Path $env:TEMP 'GUMZYY-CALL-FX-GITHUB-PUBLISH'

function Refresh-Path {
    $machine = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    $user = [Environment]::GetEnvironmentVariable('Path', 'User')
    $env:Path = $machine + ';' + $user
    $extra = @(
        (Join-Path $env:ProgramFiles 'Git\cmd'),
        (Join-Path $env:ProgramFiles 'GitHub CLI'),
        (Join-Path $env:LOCALAPPDATA 'Programs\GitHub CLI'),
        (Join-Path $env:LOCALAPPDATA 'Programs\Git\cmd')
    )
    foreach ($path in $extra) { if ((Test-Path $path) -and ($env:Path -notlike "*$path*")) { $env:Path += ';' + $path } }
}
function Require-Command([string]$name) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) { throw "$name was not found. Install Git for Windows and GitHub CLI, then run this helper again." }
}

Write-Host 'GUMZYY CALL FX - Publish project to GitHub' -ForegroundColor Magenta
Write-Host "Target repository: $repo" -ForegroundColor Cyan
Write-Host 'This updates only the existing Gumzyy-Call-FX repository; OFF-SESSION-TUNE is untouched.'
Refresh-Path
Require-Command 'git'
Require-Command 'gh'

& gh auth status --hostname github.com *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host 'A browser window will open so you can authorize GitHub CLI.' -ForegroundColor Yellow
    & gh auth login --hostname github.com --git-protocol https --web
    if ($LASTEXITCODE -ne 0) { throw 'GitHub authentication did not complete.' }
}
& gh auth setup-git
if ($LASTEXITCODE -ne 0) { throw 'Could not configure GitHub authentication.' }

# A missing repository is an expected first-run condition. In Windows PowerShell,
# native stderr can become a terminating PowerShell error when ErrorActionPreference
# is Stop, even though we only need gh's exit code here. Temporarily use Continue.
$previousPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = 'Continue'
    & gh repo view $repo *> $null
    $repoViewExitCode = $LASTEXITCODE
}
finally {
    $ErrorActionPreference = $previousPreference
}

if ($repoViewExitCode -ne 0) {
    Write-Host 'Repository does not exist yet (or is not accessible). Creating it now...' -ForegroundColor Yellow
    $previousPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        & gh repo create $repo --public --description 'GUMZYY CALL FX - NYXV x OFF SESSION low-latency ad-lib effects VST3'
        $repoCreateExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousPreference
    }
    if ($repoCreateExitCode -ne 0) {
        throw 'Could not create the new GitHub repository. Check that the authenticated account can create repositories and that this repository name is available.'
    }
}
else {
    Write-Host 'GitHub repository already exists. Reusing it...' -ForegroundColor Green
}

if (Test-Path $work) { Remove-Item -Recurse -Force $work }
Write-Host 'Cloning repository...' -ForegroundColor Yellow
& gh repo clone $repo $work
if ($LASTEXITCODE -ne 0) { throw 'Could not clone the new repository.' }

Write-Host 'Copying source files...' -ForegroundColor Yellow
Get-ChildItem -LiteralPath $source -Force | Where-Object { $_.Name -notin @('.git', 'build', 'release') } | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $work -Recurse -Force
}
# Remove the previous hyphenated resource filename so the repository contains only
# the new C++-identifier-safe Assets/phone_banner.png asset.
$obsoleteBanner = Join-Path $work 'Assets\phone-banner.png'
if (Test-Path $obsoleteBanner) { Remove-Item -LiteralPath $obsoleteBanner -Force }

Push-Location $work
try {
    $userJson = gh api user
    if ($LASTEXITCODE -ne 0) { throw 'Could not read authenticated GitHub account.' }
    $user = $userJson | ConvertFrom-Json
    & git config user.name $user.login
    & git config user.email ("{0}+{1}@users.noreply.github.com" -f $user.id, $user.login)
    & git add -A
    if ($LASTEXITCODE -ne 0) { throw 'Git could not stage project files.' }
    & git diff --cached --quiet
    if ($LASTEXITCODE -eq 0) {
        Write-Host 'Repository already contains the exact project files.' -ForegroundColor Green
    } else {
        & git commit -m 'Fix GUMZYY CALL FX v0.3.2 button LookAndFeel linker symbols'
        if ($LASTEXITCODE -ne 0) { throw 'Git commit failed.' }
        & git push origin HEAD:main
        if ($LASTEXITCODE -ne 0) { throw 'Git push failed.' }
        Write-Host 'Project files pushed successfully.' -ForegroundColor Green
    }
}
finally { Pop-Location }

Write-Host ''
Write-Host 'NEXT: open Actions and inspect the Windows build.' -ForegroundColor Cyan
Write-Host 'https://github.com/sleeepboii123-bot/Gumzyy-Call-FX/actions'
Read-Host 'Press Enter to close'
