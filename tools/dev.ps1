<#
.SYNOPSIS
    Developer shortcuts for Burn The Wo (build / run / package), similar to "npm run <script>".

.DESCRIPTION
    Usually called through run.bat in the repo root:
        .\run              build Debug, then play
        .\run release      build Release, then play
        .\run build        build Debug only
        .\run play         play the last Debug build without building
        .\run package      build Release + make the download zip (tools\package-release.ps1)
        .\run clean        delete Debug\ and Release\ build output
        .\run help         show this list
#>
param(
    [Parameter(Position = 0)]
    [string]$Command = 'dev'
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $root 'Burn The Wok.slnx'
# Same working directory Visual Studio uses for F5, so relative asset paths behave identically.
$gameProjectDir = Join-Path $root 'Burn The Wok'

function Fail([string]$message) {
    Write-Host ''
    Write-Host "GAGAL: $message" -ForegroundColor Red
    exit 1
}

function Find-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        Fail 'Visual Studio tidak ditemukan (vswhere.exe tidak ada). Install Visual Studio 2026 dengan workload "Desktop development with C++".'
    }
    $msbuild = & $vswhere -latest -prerelease -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) {
        Fail 'MSBuild tidak ditemukan. Pastikan workload "Desktop development with C++" terpasang.'
    }
    return $msbuild
}

function Invoke-Build([string]$config) {
    $msbuild = Find-MSBuild
    Write-Host "Build $config | x86 ..." -ForegroundColor Cyan
    & $msbuild $solution /p:Configuration=$config /p:Platform=x86 /m /nologo /v:minimal
    if ($LASTEXITCODE -ne 0) {
        Fail "Build gagal (exit code $LASTEXITCODE). Lihat pesan error di atas."
    }
}

function Start-Game([string]$config) {
    $exe = Join-Path $root "$config\Burn The Wok.exe"
    if (-not (Test-Path $exe)) {
        Fail "$config\Burn The Wok.exe belum ada. Jalankan '.\run' (atau '.\run build') dulu."
    }
    Write-Host "Menjalankan $config\Burn The Wok.exe ... (tutup jendela gim untuk kembali ke terminal)" -ForegroundColor Green
    $process = Start-Process -FilePath $exe -WorkingDirectory $gameProjectDir -PassThru -Wait
    Write-Host "Gim ditutup (exit code $($process.ExitCode))."
}

function Show-Help {
    Write-Host ''
    Write-Host 'Pemakaian: .\run [perintah]' -ForegroundColor Cyan
    Write-Host ''
    Write-Host '  (kosong)   build Debug lalu main'
    Write-Host '  release    build Release lalu main (lebih cepat, tanpa info debug)'
    Write-Host '  build      build Debug saja'
    Write-Host '  play       main pakai build Debug terakhir (tanpa build)'
    Write-Host '  package    build Release + buat zip untuk dikirim (folder dist\)'
    Write-Host '  clean      hapus folder hasil build (Debug\, Release\)'
    Write-Host '  help       tampilkan daftar ini'
    Write-Host ''
}

switch ($Command.ToLowerInvariant()) {
    'dev'     { Invoke-Build 'Debug';   Start-Game 'Debug' }
    'release' { Invoke-Build 'Release'; Start-Game 'Release' }
    'build'   { Invoke-Build 'Debug' }
    'play'    { Start-Game 'Debug' }
    'package' { & (Join-Path $PSScriptRoot 'package-release.ps1'); exit $LASTEXITCODE }
    'clean' {
        $outputDirs = @('Debug', 'Release', 'Burn The Wok\Debug', 'Burn The Wok\Release', 'Engine\Debug', 'Engine\Release')
        foreach ($dir in $outputDirs) {
            $path = Join-Path $root $dir
            if (Test-Path $path) {
                Remove-Item $path -Recurse -Force
                Write-Host "Dihapus: $dir"
            }
        }
        Write-Host 'Bersih.' -ForegroundColor Green
    }
    { $_ -in @('help', '-h', '--help', '/?') } { Show-Help }
    default {
        Write-Host "Perintah tidak dikenal: $Command" -ForegroundColor Red
        Show-Help
        exit 1
    }
}
