<#
.SYNOPSIS
    Builds the Release version of Burn The Wo and packs it into a zip that players can download.

.DESCRIPTION
    1. Finds MSBuild (Visual Studio 2026) and builds "Burn The Wok.slnx" as Release | x86.
    2. Collects everything the game needs into dist\Burn-The-Wo\ :
         Burn The Wo.exe, SDL2.dll, glew32.dll, CARA MAIN.txt, licenses\, assets\ (if any)
    3. Zips that folder into dist\Burn-The-Wo_<date>_<commit>.zip

    See docs\07-build-dan-distribusi.md for the full guide.

.PARAMETER SkipBuild
    Do not run MSBuild; package the existing Release\ output as-is.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\package-release.ps1
#>
param(
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $root 'Burn The Wok.slnx'
$releaseDir = Join-Path $root 'Release'
$distDir = Join-Path $root 'dist'
$stageDir = Join-Path $distDir 'Burn-The-Wo'

function Fail([string]$message) {
    Write-Host ''
    Write-Host "GAGAL: $message" -ForegroundColor Red
    exit 1
}

# --- 1. Build ---------------------------------------------------------------
if (-not $SkipBuild) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        Fail 'Visual Studio tidak ditemukan (vswhere.exe tidak ada). Install Visual Studio 2026 dengan workload "Desktop development with C++".'
    }
    $msbuild = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) {
        Fail 'MSBuild tidak ditemukan. Pastikan workload "Desktop development with C++" terpasang.'
    }

    Write-Host "[1/3] Build Release | x86 ..." -ForegroundColor Cyan
    & $msbuild $solution /p:Configuration=Release /p:Platform=x86 /m /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) {
        Fail "Build gagal (exit code $LASTEXITCODE). Lihat pesan error di atas."
    }
} else {
    Write-Host "[1/3] Build dilewati (-SkipBuild)" -ForegroundColor Yellow
}

# --- 2. Collect files --------------------------------------------------------
Write-Host "[2/3] Menyusun folder dist\Burn-The-Wo ..." -ForegroundColor Cyan

$exe = Join-Path $releaseDir 'Burn The Wok.exe'
$requiredDlls = @('SDL2.dll', 'glew32.dll')
if (-not (Test-Path $exe)) {
    Fail "File $exe tidak ada. Jalankan skrip tanpa -SkipBuild."
}
foreach ($dll in $requiredDlls) {
    if (-not (Test-Path (Join-Path $releaseDir $dll))) {
        Fail "File Release\$dll tidak ada. Build ulang tanpa -SkipBuild."
    }
}

if (Test-Path $stageDir) {
    Remove-Item $stageDir -Recurse -Force
}
New-Item -ItemType Directory -Force $stageDir | Out-Null

# The project/exe is named after the repo folder ("Burn The Wok"); players see the official title.
Copy-Item $exe (Join-Path $stageDir 'Burn The Wo.exe')
foreach ($dll in $requiredDlls) {
    Copy-Item (Join-Path $releaseDir $dll) $stageDir
}
Copy-Item (Join-Path $PSScriptRoot 'package\CARA MAIN.txt') $stageDir

# Third-party licenses must travel with the binaries (GLEW's license requires it).
$licenseDir = Join-Path $stageDir 'licenses'
New-Item -ItemType Directory -Force $licenseDir | Out-Null
Copy-Item (Join-Path $root 'External\SDL\COPYING.txt') (Join-Path $licenseDir 'SDL2-LICENSE.txt')
Copy-Item (Join-Path $root 'External\GLEW\LICENSE.txt') (Join-Path $licenseDir 'GLEW-LICENSE.txt')

# Game assets (sprites, audio, fonts), once they exist.
$assetsDir = Join-Path $root 'Burn The Wok\assets'
if (Test-Path $assetsDir) {
    Copy-Item $assetsDir (Join-Path $stageDir 'assets') -Recurse
}

# --- 3. Zip ------------------------------------------------------------------
Write-Host "[3/3] Membuat zip ..." -ForegroundColor Cyan

$date = Get-Date -Format 'yyyy-MM-dd'
$commit = ''
try {
    $hash = (& git -C $root rev-parse --short HEAD 2>$null)
    if ($LASTEXITCODE -eq 0 -and $hash) {
        $commit = "_$hash"
    }
} catch {
    # git not installed: the date alone is fine.
}
$zipPath = Join-Path $distDir "Burn-The-Wo_$date$commit.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}
# Zipping the folder itself (not its contents) means players get one tidy folder after extracting.
Compress-Archive -Path $stageDir -DestinationPath $zipPath

# --- Summary -----------------------------------------------------------------
Write-Host ''
Write-Host 'Isi paket:' -ForegroundColor Green
Get-ChildItem $stageDir -Recurse -File | ForEach-Object {
    $relative = $_.FullName.Substring($stageDir.Length + 1)
    '  {0,-30} {1,10:N0} byte' -f $relative, $_.Length
}
$zipSize = (Get-Item $zipPath).Length / 1MB
Write-Host ''
Write-Host ('SELESAI: {0}  ({1:N2} MB)' -f $zipPath, $zipSize) -ForegroundColor Green
Write-Host 'Langkah berikutnya: tes zip ini (docs\07-build-dan-distribusi.md bagian 3), lalu upload.'
