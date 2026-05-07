param(
    [ValidateSet("Debug", "Release")]
    [string] $Configuration = "Debug",

    [ValidateSet("Win32")]
    [string] $Platform = "Win32",

    [string] $PlatformToolset = "v120"
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$solution = Join-Path $repoRoot "src\FreeFalcon.sln"

if (-not (Test-Path $solution)) {
    throw "Could not find solution at $solution"
}

if (-not $env:DX81_SDK -and (Test-Path "C:\DXSDK\include") -and (Test-Path "C:\DXSDK\lib")) {
    $env:DX81_SDK = "C:\DXSDK\"
}

if (-not $env:DX81_SDK) {
    throw "DX81_SDK is not set. Install the DirectX 8.1 SDK and set DX81_SDK to its install path, for example C:\DXSDK\."
}

$dxInclude = Join-Path $env:DX81_SDK "include"
$dxLib = Join-Path $env:DX81_SDK "lib"
if (-not (Test-Path $dxInclude) -or -not (Test-Path $dxLib)) {
    throw "DX81_SDK is set to '$env:DX81_SDK', but include/lib folders were not found."
}

$programFilesX86 = ${env:ProgramFiles(x86)}
$vswhere = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
$vsDevCmd = $null

if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) {
        $vsPath = & $vswhere -latest -products * -property installationPath
    }
    if ($vsPath) {
        $candidates = @(
            (Join-Path $vsPath "Common7\Tools\VsDevCmd.bat"),
            (Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat")
        )
        foreach ($candidate in $candidates) {
            if (Test-Path $candidate) {
                $vsDevCmd = $candidate
                break
            }
        }
    }
}

$vs2013VcVars = Join-Path $programFilesX86 "Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
if (-not $vsDevCmd -and (Test-Path $vs2013VcVars)) {
    $vsDevCmd = $vs2013VcVars
}

$knownVcVars = @(
    (Join-Path $programFilesX86 "Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"),
    (Join-Path $programFilesX86 "Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat")
)
foreach ($candidate in $knownVcVars) {
    if (-not $vsDevCmd -and (Test-Path $candidate)) {
        $vsDevCmd = $candidate
    }
}

if (-not $vsDevCmd) {
    throw "No Visual Studio developer environment was found. Install Visual Studio 2013 or Visual Studio Build Tools with C++ support."
}

$msbuildArgs = @(
    "`"$solution`"",
    "/m",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:PlatformToolset=$PlatformToolset",
    "/v:m"
) -join " "

Write-Host "Building $Configuration|$Platform with PlatformToolset=$PlatformToolset"
if ($vsDevCmd.EndsWith("vcvarsall.bat")) {
    & cmd.exe /c "call `"$vsDevCmd`" x86 && msbuild $msbuildArgs"
} else {
    & cmd.exe /c "call `"$vsDevCmd`" -arch=x86 && msbuild $msbuildArgs"
}
exit $LASTEXITCODE
