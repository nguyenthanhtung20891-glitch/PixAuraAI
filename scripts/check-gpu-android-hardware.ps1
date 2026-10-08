param([string]$SdkRoot = $env:ANDROID_HOME)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$workspacePath = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $workspacePath
$sourceChanges = & git status --porcelain --untracked-files=normal
if ($LASTEXITCODE -ne 0 -or $sourceChanges) { throw 'FAIL: hardware certification requires a clean committed checkout.' }
if (-not $SdkRoot) { throw 'Set ANDROID_HOME or supply -SdkRoot for the installed SDK.' }
$adbPath = Join-Path $SdkRoot 'platform-tools\adb.exe'
if (-not (Test-Path -LiteralPath $adbPath)) { throw 'Installed adb.exe is required.' }
if (-not (Get-Command node -ErrorAction SilentlyContinue)) { throw 'Node is required for bounded evidence validation.' }
# A single previously authorized connected device; no serial recorded.
$state = & $adbPath get-state
if ($LASTEXITCODE -ne 0 -or "$state".Trim() -ne 'device') { throw 'One authorized physical Android device must be connected.' }
$qemu = & $adbPath shell getprop ro.kernel.qemu
if ($LASTEXITCODE -ne 0 -or "$qemu".Trim() -eq '1') { throw 'UNSUPPORTED: emulator cannot certify physical hardware.' }
$validationRun = [Guid]::NewGuid().ToString('N')
$deviceRunPath = "/data/local/tmp/pixaura-gpu-$validationRun"
$runPath = Join-Path $workspacePath ('build\gpu-android-hardware\run-' + $validationRun)
New-Item -ItemType Directory -Force -Path $runPath | Out-Null
$previousTemp = $env:TEMP
$previousTmp = $env:TMP
$previousSdk = $env:ANDROID_HOME
$previousAndroidUser = $env:ANDROID_USER_HOME
$deviceDirectoryCreated = $false
try {
    # Exclusive fresh directory: never read an old app-private file or scan artifacts.
    & $adbPath shell "umask 077; mkdir '$deviceRunPath'"
    if ($LASTEXITCODE -ne 0) { throw 'FAIL: fresh evidence directory creation failed.' }
    $deviceDirectoryCreated = $true
    & $adbPath push "$PSScriptRoot\publish-gpu-android-evidence.sh" "$deviceRunPath/publish.sh" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'FAIL: evidence publisher setup failed.' }
    $env:TEMP = $runPath
    $env:TMP = $runPath
    $env:ANDROID_HOME = $SdkRoot
    $env:ANDROID_USER_HOME = Join-Path $workspacePath 'build\android-user'
    $sourceSha = (& git rev-parse HEAD).Trim()
    if ($LASTEXITCODE -ne 0 -or $sourceSha -notmatch '^[0-9a-f]{40}$') { throw 'Source SHA unavailable.' }
    $sourceSha | Set-Content -LiteralPath (Join-Path $runPath 'commit.txt')
    & "$workspacePath\platforms\android\gradlew.bat" -p "$workspacePath\platforms\android" --no-daemon `
        --gradle-user-home "$workspacePath\build\gradle-home" --project-cache-dir "$workspacePath\build\gpu-android-hardware\gradle-cache" `
        --dependency-verification strict :app:connectedDebugAndroidTest `
        '-Pandroid.testInstrumentationRunnerArguments.class=ai.pixaura.app.GpuHardwareTest' `
        '-Pandroid.testInstrumentationRunnerArguments.pixauraHardware=true' `
        "-Pandroid.testInstrumentationRunnerArguments.pixauraRun=$validationRun" `
        "-Pandroid.testInstrumentationRunnerArguments.pixauraSha=$sourceSha" 2>&1 | Tee-Object -FilePath (Join-Path $runPath 'gradle.log')
    $testExit = $LASTEXITCODE
    # Bounded read of the exact current shell-owned artifact after UTP uninstall.
    $json = & $adbPath shell "test -f '$deviceRunPath/evidence.json' && head -c 16385 '$deviceRunPath/evidence.json'"
    if ($LASTEXITCODE -ne 0) { throw 'FAIL: hardware evidence readback failed.' }
    [System.IO.File]::WriteAllText((Join-Path $runPath 'evidence.json'), ($json -join "`n"),
        (New-Object System.Text.UTF8Encoding($false)))
    & node "$PSScriptRoot\gpu-android-evidence.mjs" (Join-Path $runPath 'evidence.json') $validationRun $sourceSha $testExit
    if ($LASTEXITCODE -ne 0) { throw 'Hardware certification did not PASS; Phase 2 remains BLOCKED.' }
    Write-Output "Evidence: $runPath"
} finally {
    # Only this invocation's synthetic diagnostic artifact; no phone data scans.
    if ($deviceDirectoryCreated) {
        & $adbPath shell "rm -f '$deviceRunPath/evidence.json' '$deviceRunPath/publish.sh'; rmdir '$deviceRunPath'" | Out-Null
        if ($LASTEXITCODE -ne 0) { Write-Warning 'Current synthetic evidence cleanup failed.' }
    }
    $env:TEMP = $previousTemp
    $env:TMP = $previousTmp
    $env:ANDROID_HOME = $previousSdk
    $env:ANDROID_USER_HOME = $previousAndroidUser
}
