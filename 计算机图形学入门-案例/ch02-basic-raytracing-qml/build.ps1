param(
    [string]$QtPrefix = "D:\Qt\6.9.3\mingw_64",
    [string]$CMake = "D:\Qt\Tools\CMake_64\bin\cmake.exe",
    [string]$Ninja = "D:\Qt\Tools\Ninja\ninja.exe"
)

$ErrorActionPreference = "Stop"
$projectRoot = (Resolve-Path -LiteralPath $PSScriptRoot).Path
$buildDrive = "R:"

if (Test-Path -LiteralPath ($buildDrive + "\")) {
    throw "$buildDrive is already in use. Free that drive letter before building."
}

foreach ($requiredPath in @($QtPrefix, $CMake, $Ninja)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "Required Qt build path was not found: $requiredPath"
    }
}

try {
    & subst.exe $buildDrive $projectRoot
    if ($LASTEXITCODE -ne 0) {
        throw "Unable to map $buildDrive to the project directory."
    }

    & $CMake `
        -S ($buildDrive + "\") `
        -B ($buildDrive + "\build-ascii") `
        -G Ninja `
        ("-DCMAKE_PREFIX_PATH=" + $QtPrefix) `
        ("-DCMAKE_MAKE_PROGRAM=" + $Ninja)
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed."
    }

    & $CMake --build ($buildDrive + "\build-ascii") --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed."
    }

    Write-Host "Built: $projectRoot\build-ascii\ch02_raytracing.exe"
}
finally {
    if (Test-Path -LiteralPath ($buildDrive + "\")) {
        & subst.exe $buildDrive /D | Out-Null
    }
}

