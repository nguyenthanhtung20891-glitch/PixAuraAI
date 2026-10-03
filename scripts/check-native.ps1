$ErrorActionPreference = 'Stop'
$workspacePath = Split-Path $PSScriptRoot -Parent
$vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswherePath)) { throw 'Install Visual Studio C++ workload (vswhere missing).' }
$installPath = & $vswherePath -latest -products '*' -property installationPath
if (-not $installPath) { throw 'Install Visual Studio desktop C++ workload.' }
$vcvarsPath = Join-Path $installPath 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $vcvarsPath)) { throw 'Install Visual Studio desktop C++ workload (vcvars missing).' }
$vcvarsAllPath = Join-Path $installPath 'VC\Auxiliary\Build\vcvarsall.bat'
if (-not (Test-Path -LiteralPath $vcvarsAllPath)) { throw 'Visual Studio C++ installation is incomplete: repair the desktop C++ workload (vcvarsall missing).' }
$buildPath = Join-Path $workspacePath 'build\msvc'
New-Item -ItemType Directory -Path $buildPath -Force | Out-Null
$commands = @(
    'cl /nologo /std:c++17 /EHsc /W4 /WX /LD /DPIXAURA_SHARED /DPIXAURA_BUILDING /I"..\..\packages\core\include" "..\..\packages\core\src\core.cpp" "..\..\packages\core\src\document.cpp" "..\..\packages\core\src\document_api.cpp" /link /OUT:pixaura_core.dll',
    'cl /nologo /std:c++17 /EHsc /W4 /WX /DPIXAURA_SHARED /I"..\..\packages\core\include" "..\..\packages\core\tests\core_test.cpp" pixaura_core.lib /Fe:core_test.exe',
    'cl /nologo /TC /W4 /WX /DPIXAURA_SHARED /I"..\..\packages\core\include" "..\..\packages\core\tests\c_consumer.c" pixaura_core.lib /Fe:c_consumer.exe',
    'core_test.exe',
    'c_consumer.exe',
    'cl /nologo /std:c++17 /EHsc /W4 /WX /DPIXAURA_SHARED /I"..\..\packages\core\include" "..\..\packages\core\tests\document_test.cpp" "..\..\packages\core\src\document.cpp" pixaura_core.lib /Fe:document_test.exe',
    'cl /nologo /TC /W4 /WX /DPIXAURA_SHARED /I"..\..\packages\core\include" "..\..\packages\core\tests\document_c_consumer.c" pixaura_core.lib /Fe:document_c_consumer.exe',
    'cl /nologo /std:c++17 /EHsc /W4 /WX /I"..\..\packages\core\include" "..\..\packages\core\tests\document_allocation_test.cpp" "..\..\packages\core\src\document.cpp" "..\..\packages\core\src\document_api.cpp" /Fe:document_allocation_test.exe',
    'document_test.exe "..\..\tests\fixtures\image-document-v1.json"',
    'document_c_consumer.exe "..\..\tests\fixtures\image-document-v1.json" document-canonical.json',
    'document_allocation_test.exe "..\..\tests\fixtures\image-document-v1.json"'
)
Push-Location $buildPath
try {
    $batchLines = @('@echo off', ('call "' + $vcvarsPath + '"'), 'if errorlevel 1 exit /b 1')
    foreach ($command in $commands) { $batchLines += @($command, 'if errorlevel 1 exit /b 1') }
    $batchLines += 'exit /b 0'
    Set-Content -LiteralPath (Join-Path $buildPath 'check.cmd') -Value $batchLines -Encoding ASCII
    & cmd.exe /d /c check.cmd
    if ($LASTEXITCODE -ne 0) { throw "Native checks failed (exit $LASTEXITCODE)." }
} finally { Pop-Location }
