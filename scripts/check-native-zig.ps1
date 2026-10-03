param([Parameter(Mandatory = $true)][string]$ZigPath)
$ErrorActionPreference = 'Stop'
$zigCompiler = (Resolve-Path -LiteralPath $ZigPath).Path
$workspacePath = Split-Path $PSScriptRoot -Parent
$buildPath = Join-Path $workspacePath 'build\zig-host'
New-Item -ItemType Directory -Path $buildPath -Force | Out-Null
$previousLocalCache = $env:ZIG_LOCAL_CACHE_DIR
$previousGlobalCache = $env:ZIG_GLOBAL_CACHE_DIR
$env:ZIG_LOCAL_CACHE_DIR = Join-Path $buildPath 'local-cache'
$env:ZIG_GLOBAL_CACHE_DIR = Join-Path $buildPath 'global-cache'
Push-Location $buildPath
try {
    & $zigCompiler c++ -target x86_64-windows-gnu -std=c++17 -Wall -Wextra -Wpedantic -Werror -DPIXAURA_SHARED -DPIXAURA_BUILDING -I ../../packages/core/include -shared ../../packages/core/src/core.cpp ../../packages/core/src/document.cpp ../../packages/core/src/document_api.cpp -o pixaura_core.dll '-Wl,--out-implib,pixaura_core.lib'
    if ($LASTEXITCODE -ne 0) { throw 'Shared core build failed.' }
    & $zigCompiler c++ -target x86_64-windows-gnu -std=c++17 -Wall -Wextra -Wpedantic -Werror -DPIXAURA_SHARED -I ../../packages/core/include ../../packages/core/tests/core_test.cpp pixaura_core.lib -o core_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'C++ consumer build failed.' }
    & $zigCompiler cc -target x86_64-windows-gnu -std=c11 -Wall -Wextra -Wpedantic -Werror -DPIXAURA_SHARED -I ../../packages/core/include ../../packages/core/tests/c_consumer.c pixaura_core.lib -o c_consumer.exe
    if ($LASTEXITCODE -ne 0) { throw 'C consumer build failed.' }
    & .\core_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'C++ ABI test failed.' }
    & .\c_consumer.exe
    if ($LASTEXITCODE -ne 0) { throw 'C consumer test failed.' }
    & $zigCompiler c++ -target x86_64-windows-gnu -std=c++17 -Wall -Wextra -Wpedantic -Werror -DPIXAURA_SHARED -I ../../packages/core/include ../../packages/core/tests/document_test.cpp ../../packages/core/src/document.cpp pixaura_core.lib -o document_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'Document C++ consumer build failed.' }
    & $zigCompiler cc -target x86_64-windows-gnu -std=c11 -Wall -Wextra -Wpedantic -Werror -DPIXAURA_SHARED -I ../../packages/core/include ../../packages/core/tests/document_c_consumer.c pixaura_core.lib -o document_c_consumer.exe
    if ($LASTEXITCODE -ne 0) { throw 'Document C consumer build failed.' }
    & $zigCompiler c++ -target x86_64-windows-gnu -std=c++17 -Wall -Wextra -Wpedantic -Werror -I ../../packages/core/include ../../packages/core/tests/document_allocation_test.cpp ../../packages/core/src/document.cpp ../../packages/core/src/document_api.cpp -o document_allocation_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'Document allocation consumer build failed.' }
    foreach ($documentTest in @('document_test.exe', 'document_c_consumer.exe', 'document_allocation_test.exe')) {
        if ($documentTest -eq 'document_c_consumer.exe') {
            & (Join-Path $buildPath $documentTest) ../../tests/fixtures/image-document-v1.json document-canonical.json
        } else {
            & (Join-Path $buildPath $documentTest) ../../tests/fixtures/image-document-v1.json
        }
        if ($LASTEXITCODE -ne 0) { throw "$documentTest failed." }
    }
    & $zigCompiler c++ -target x86_64-windows-gnu -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsanitize=undefined -I ../../packages/core/include ../../packages/core/src/core.cpp ../../packages/core/tests/core_test.cpp -o core_ubsan.exe
    if ($LASTEXITCODE -ne 0) { throw 'Undefined-behavior trap build failed.' }
    & .\core_ubsan.exe
    if ($LASTEXITCODE -ne 0) { throw 'Undefined-behavior trap test failed.' }
} finally {
    Pop-Location
    $env:ZIG_LOCAL_CACHE_DIR = $previousLocalCache
    $env:ZIG_GLOBAL_CACHE_DIR = $previousGlobalCache
}
