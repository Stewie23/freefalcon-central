param(
    [switch] $IncludeVisualStudio2013,
    [switch] $RepairCurrentBuildTools
)

$ErrorActionPreference = "Stop"

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator
)

if (-not $isAdmin) {
    throw "Run this script from an elevated PowerShell window because Chocolatey installs machine-wide packages."
}

$packages = @(
    "windows-sdk-7.1",
    "directx-sdk",
    "wixtoolset"
)

if ($RepairCurrentBuildTools) {
    $packages += @(
        "visualstudio2026-workload-vctools",
        "windows-sdk-10.0"
    )
}

if ($IncludeVisualStudio2013) {
    $packages += @(
        "VisualStudio2013Professional",
        "visualstudio2013-update"
    )
}

choco install -y $packages

Write-Host ""
Write-Host "After install, set DX81_SDK to the DirectX 8.1 SDK path expected by the project."
Write-Host "The original FreeFalcon guide expects DirectX SDK 8.1 at C:\DXSDK\."
Write-Host "Chocolatey's directx-sdk package is newer than 8.1, so the original SDK may still be required if d3dx.lib is missing."
