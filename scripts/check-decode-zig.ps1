param([Parameter(Mandatory = $true)][string]$ZigPath)
$ErrorActionPreference = 'Stop'
$workspacePath = Split-Path $PSScriptRoot -Parent
$buildPath = Join-Path $workspacePath 'build\decode-zig'
New-Item -ItemType Directory -Force $buildPath | Out-Null
$zigCompiler = (Resolve-Path -LiteralPath $ZigPath).Path
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$cmakeCompiler = if ($cmakeCommand) { $cmakeCommand.Source } else { 'I:\AndroidStudioSDKdata\cmake\3.22.1\bin\cmake.exe' }
if (-not (Test-Path -LiteralPath $cmakeCompiler)) { throw 'CMake is required for the codec/native gate.' }
$toolDirectory = Split-Path $cmakeCompiler -Parent
$ninjaCompiler = Join-Path $toolDirectory 'ninja.exe'
$ctestCompiler = Join-Path $toolDirectory 'ctest.exe'
$utf8 = New-Object System.Text.UTF8Encoding($false)
foreach ($entry in @(@('cc', 'cc'), @('cxx', 'c++'), @('ar', 'ar'), @('ranlib', 'ranlib'))) {
    $target = if ($entry[0] -eq 'cc' -or $entry[0] -eq 'cxx') { ' -target x86_64-windows-gnu' } else { '' }
    [System.IO.File]::WriteAllText((Join-Path $buildPath ($entry[0] + '.cmd')), "@echo off`r`n`"$zigCompiler`" $($entry[1])$target %*`r`n", $utf8)
}
$prefix = $buildPath.Replace('\', '/')
$toolchain = @"
set(CMAKE_C_COMPILER "$prefix/cc.cmd")
set(CMAKE_CXX_COMPILER "$prefix/cxx.cmd")
set(CMAKE_AR "$prefix/ar.cmd")
set(CMAKE_RANLIB "$prefix/ranlib.cmd")
set(CMAKE_C_ARCHIVE_CREATE "$prefix/ar.cmd qc <TARGET> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "$prefix/ranlib.cmd <TARGET>")
set(CMAKE_CXX_ARCHIVE_CREATE "$prefix/ar.cmd qc <TARGET> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_FINISH "$prefix/ranlib.cmd <TARGET>")
"@
[System.IO.File]::WriteAllText((Join-Path $buildPath 'zig.cmake'), $toolchain + "`n", $utf8)
$previousLocalCache = $env:ZIG_LOCAL_CACHE_DIR
$previousGlobalCache = $env:ZIG_GLOBAL_CACHE_DIR
$previousTemp = $env:TEMP
$previousTmp = $env:TMP
try {
    $env:TEMP = Join-Path $buildPath 'tmp'
    $env:TMP = $env:TEMP
    New-Item -ItemType Directory -Path $env:TEMP -Force | Out-Null
    $env:ZIG_LOCAL_CACHE_DIR = Join-Path $buildPath 'local-cache'
    $env:ZIG_GLOBAL_CACHE_DIR = Join-Path $buildPath 'global-cache'
    $binaryPath = Join-Path $buildPath 'native'
    & $cmakeCompiler -S $workspacePath -B $binaryPath -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninjaCompiler" "-DCMAKE_TOOLCHAIN_FILE=$prefix/zig.cmake" -DCMAKE_BUILD_TYPE=Debug
    if ($LASTEXITCODE -ne 0) { throw 'Decode configure failed.' }
    & $cmakeCompiler --build $binaryPath -j 4
    if ($LASTEXITCODE -ne 0) { throw 'Decode native build failed.' }
    & $ctestCompiler --test-dir $binaryPath --output-on-failure --verbose
    if ($LASTEXITCODE -ne 0) { throw 'Decode/native tests failed.' }
} finally {
    $env:ZIG_LOCAL_CACHE_DIR = $previousLocalCache
    $env:ZIG_GLOBAL_CACHE_DIR = $previousGlobalCache
    $env:TEMP = $previousTemp
    $env:TMP = $previousTmp
}
