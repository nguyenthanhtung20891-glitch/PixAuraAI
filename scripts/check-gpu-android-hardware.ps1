param([string]$SdkRoot = $env:ANDROID_HOME)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$workspacePath = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $workspacePath
if (-not $SdkRoot) { throw 'Set ANDROID_HOME or supply -SdkRoot for the installed SDK.' }
$adbPath = Join-Path $SdkRoot 'platform-tools\adb.exe'
if (-not (Test-Path -LiteralPath $adbPath)) { throw 'Installed adb.exe is required.' }
# A single previously authorized connected device; no serial recorded.
$state = & $adbPath get-state
if ($LASTEXITCODE -ne 0 -or "$state".Trim() -ne 'device') { throw 'One authorized physical Android device must be connected.' }
$qemu = & $adbPath shell getprop ro.kernel.qemu
if ($LASTEXITCODE -ne 0 -or "$qemu".Trim() -eq '1') { throw 'UNSUPPORTED: emulator cannot certify physical hardware.' }
$runPath = Join-Path $workspacePath ('build\gpu-android-hardware\run-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $runPath | Out-Null
$previousTemp = $env:TEMP
$previousTmp = $env:TMP
$previousSdk = $env:ANDROID_HOME
$previousAndroidUser = $env:ANDROID_USER_HOME
try {
    $env:TEMP = $runPath
    $env:TMP = $runPath
    $env:ANDROID_HOME = $SdkRoot
    $env:ANDROID_USER_HOME = Join-Path $workspacePath 'build\android-user'
    & git rev-parse HEAD | Set-Content -LiteralPath (Join-Path $runPath 'commit.txt')
    & "$workspacePath\platforms\android\gradlew.bat" -p "$workspacePath\platforms\android" --no-daemon `
        --gradle-user-home "$workspacePath\build\gradle-home" --project-cache-dir "$workspacePath\build\gpu-android-hardware\gradle-cache" `
        --dependency-verification strict :app:connectedDebugAndroidTest `
        '-Pandroid.testInstrumentationRunnerArguments.class=ai.pixaura.app.GpuHardwareTest' `
        '-Pandroid.testInstrumentationRunnerArguments.pixauraHardware=true' 2>&1 | Tee-Object -FilePath (Join-Path $runPath 'gradle.log')
    $testExit = $LASTEXITCODE
    $json = & $adbPath shell run-as ai.pixaura.app cat files/gpu-hardware.json
    if ($LASTEXITCODE -ne 0) { throw 'FAIL: hardware evidence readback failed.' }
    $json | Set-Content -LiteralPath (Join-Path $runPath 'evidence.json') -Encoding UTF8
    $evidence = ($json -join "`n") | ConvertFrom-Json
    Write-Output "Vulkan hardware validation: $($evidence.status) $($evidence.parity_passed)/$($evidence.test_count) parity cases; GPU=$($evidence.gpu)"
    Write-Output "Evidence: $runPath"
    if ($testExit -ne 0 -or $evidence.status -ne 'PASS' -or -not $evidence.hardware -or -not $evidence.controlled_hardware_gate) {
        throw 'Hardware certification did not PASS; Phase 2 remains BLOCKED.'
    }
} finally {
    $env:TEMP = $previousTemp
    $env:TMP = $previousTmp
    $env:ANDROID_HOME = $previousSdk
    $env:ANDROID_USER_HOME = $previousAndroidUser
}
