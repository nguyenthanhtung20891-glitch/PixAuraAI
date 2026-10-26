param([Parameter(Mandatory = $true)][string]$ZigPath)
$ErrorActionPreference = 'Stop'
# Canonical CMake source/consumer/test authority prevents handwritten link drift.
& (Join-Path $PSScriptRoot 'check-decode-zig.ps1') -ZigPath $ZigPath
if ($LASTEXITCODE -ne 0) { throw 'Windows Zig canonical native gate failed.' }
