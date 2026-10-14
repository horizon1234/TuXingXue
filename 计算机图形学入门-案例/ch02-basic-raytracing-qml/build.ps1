<#
.SYNOPSIS
Builds the Chapter 2 Qt/QML ray-tracing demonstration.

.DESCRIPTION
Qt 6.9 moc cannot write to build paths containing non-ASCII characters in some
Windows environments. This script temporarily maps the project root to the ASCII
drive R:, configures and builds through that alias, and always removes the mapping
in the finally block. Generated files remain in the physical build-ascii directory.

.PARAMETER QtPrefix
Root of the Qt kit containing libraries and CMake package files.

.PARAMETER CMake
Path to the CMake executable supplied by Qt or another compatible installation.

.PARAMETER Ninja
Path to the Ninja executable.
#>
param(
    [string]$QtPrefix = "D:\Qt\6.9.3\mingw_64",
    [string]$CMake = "D:\Qt\Tools\CMake_64\bin\cmake.exe",
    [string]$Ninja = "D:\Qt\Tools\Ninja\ninja.exe"
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$buildDrive = "R:"

# Never overwrite an existing mapping or physical drive.
if (Test-Path -LiteralPath ($buildDrive + "\")) {
    throw "$buildDrive is already in use. Free that drive letter before building."
}

# Validate every prerequisite before changing the process environment.
foreach ($requiredPath in @($QtPrefix, $CMake, $Ninja)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required Qt build path was not found: $requiredPath"
    }
}

try {
    # subst creates a path alias only; it does not copy or move project files.
    & subst.exe $buildDrive $projectRoot
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to map $buildDrive to the project directory."
    }

    # Configure through the ASCII alias. CMAKE_PREFIX_PATH selects the Qt kit,
    # while CMAKE_MAKE_PROGRAM binds the intended Ninja executable explicitly.
    & $CMake `
        -S ($buildDrive + "\") `
        -B ($buildDrive + "\build-ascii") `
        -G Ninja `
        ("-DCMAKE_PREFIX_PATH=" + $QtPrefix) `
        ("-DCMAKE_MAKE_PROGRAM=" + $Ninja)
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed."
    }

    # Let CMake/Ninja schedule compilation across the available CPU cores.
    & $CMake --build ($buildDrive + "\build-ascii") --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed."
    }

    Write-Host "Built: $projectRoot\build-ascii\ch02_raytracing.exe"
}
finally {
    # Do not leave the temporary mapping behind when configuration or build fails.
    if (Test-Path -LiteralPath ($buildDrive + "\")) {
        & subst.exe $buildDrive /D | Out-Null
    }
}

