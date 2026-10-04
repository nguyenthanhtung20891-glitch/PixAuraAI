param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet(
        "auth-status",
        "run-list",
        "run-view",
        "run-failed",
        "run-watch"
    )]
    [string]$Action,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Rest
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($null -eq $Rest) {
    $Rest = @()
}

$Repo = "nguyenthanhtung20891-glitch/PixAuraAI"

$Candidates = @(
    "C:\Program Files\GitHub CLI\gh.exe",
    "$env:LOCALAPPDATA\Programs\GitHub CLI\gh.exe"
)

$Gh = $null

foreach ($Candidate in $Candidates) {
    if (Test-Path $Candidate) {
        $Gh = $Candidate
        break
    }
}

if (-not $Gh) {
    $Command = Get-Command gh -ErrorAction SilentlyContinue

    if ($Command) {
        $Gh = $Command.Source
    }
}

if (-not $Gh) {
    throw "GitHub CLI gh.exe could not be located."
}

function Invoke-Gh {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    # gh caches downloaded run logs beneath LOCALAPPDATA. Keep those writes
    # inside the approved repository without changing credential configuration.
    $PreviousLocalAppData = $env:LOCALAPPDATA
    $CacheRoot = Join-Path (Split-Path $PSScriptRoot -Parent) "build\gh-cache"
    New-Item -ItemType Directory -Force -Path $CacheRoot | Out-Null
    try {
        $env:LOCALAPPDATA = $CacheRoot
        & $Gh @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "GitHub CLI command failed with exit code $LASTEXITCODE."
        }
    } finally {
        $env:LOCALAPPDATA = $PreviousLocalAppData
    }
}

switch ($Action) {

    "auth-status" {
        if ($Rest.Count -gt 0) {
            throw "auth-status takes no arguments."
        }

        Invoke-Gh @("auth", "status")
    }

    "run-list" {
        if ($Rest.Count -ne 1) {
            throw "run-list requires exactly one commit SHA."
        }

        $Sha = $Rest[0]

        if ($Sha -notmatch '^[0-9a-fA-F]{7,40}$') {
            throw "Invalid commit SHA."
        }

        Invoke-Gh @(
            "run",
            "list",
            "--repo", $Repo,
            "--commit", $Sha
        )
    }

    "run-view" {
        if ($Rest.Count -ne 1 -or $Rest[0] -notmatch '^\d+$') {
            throw "run-view requires one numeric run ID."
        }

        Invoke-Gh @(
            "run",
            "view",
            $Rest[0],
            "--repo", $Repo
        )
    }

    "run-failed" {
        if ($Rest.Count -ne 1 -or $Rest[0] -notmatch '^\d+$') {
            throw "run-failed requires one numeric run ID."
        }

        Invoke-Gh @(
            "run",
            "view",
            $Rest[0],
            "--repo", $Repo,
            "--log-failed"
        )
    }

    "run-watch" {
        if ($Rest.Count -ne 1 -or $Rest[0] -notmatch '^\d+$') {
            throw "run-watch requires one numeric run ID."
        }

        Invoke-Gh @(
            "run",
            "watch",
            $Rest[0],
            "--repo", $Repo,
            "--exit-status"
        )
    }
}
