param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet(
        "status",
        "diff",
        "stage",
        "unstage",
        "commit",
        "push",
        "log",
        "head"
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
# ------------------------------------------------------------
# Fixed PixAuraAI repository boundary
# ------------------------------------------------------------

$ExpectedRepo = [System.IO.Path]::GetFullPath("G:\PixAuraAI").TrimEnd('\')

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = [System.IO.Path]::GetFullPath(
    (Join-Path $ScriptDir "..")
).TrimEnd('\')

if (-not [string]::Equals(
    $RepoRoot,
    $ExpectedRepo,
    [System.StringComparison]::OrdinalIgnoreCase
)) {
    throw "Refusing Git operation: wrapper is not running from the approved PixAuraAI repository."
}

$GitTopLevelRaw = (& git -C $RepoRoot rev-parse --show-toplevel 2>$null)

if ($LASTEXITCODE -ne 0 -or -not $GitTopLevelRaw) {
    throw "PixAuraAI Git repository could not be verified."
}

$GitTopLevel = [System.IO.Path]::GetFullPath(
    ($GitTopLevelRaw.Trim() -replace '/', '\')
).TrimEnd('\')

if (-not [string]::Equals(
    $GitTopLevel,
    $ExpectedRepo,
    [System.StringComparison]::OrdinalIgnoreCase
)) {
    throw "Refusing Git operation outside G:\PixAuraAI."
}

# ------------------------------------------------------------
# Helpers
# ------------------------------------------------------------

function Invoke-Git {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    & git -C $RepoRoot @Arguments

    if ($LASTEXITCODE -ne 0) {
        throw "Git command failed with exit code $LASTEXITCODE."
    }
}

function Assert-SafeRepoPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        throw "Empty Git path is not allowed."
    }

    if ([System.IO.Path]::IsPathRooted($Path)) {
        throw "Absolute paths are not allowed: $Path"
    }

    $Segments = $Path -split '[\\/]'

    if ($Segments -contains "..") {
        throw "Parent-directory traversal is not allowed: $Path"
    }
}

# ------------------------------------------------------------
# Actions
# ------------------------------------------------------------

switch ($Action) {

    "status" {
        if ($Rest.Count -gt 0) {
            throw "status does not accept additional arguments."
        }

        Invoke-Git @("status", "--short", "--branch")
    }

    "diff" {
        if ($Rest.Count -eq 0) {
            Invoke-Git @("diff")
        }
        elseif ($Rest.Count -eq 1 -and $Rest[0] -eq "--cached") {
            Invoke-Git @("diff", "--cached")
        }
        else {
            throw "diff supports only no arguments or --cached."
        }
    }

    "stage" {
        if ($Rest.Count -eq 0) {
            throw "stage requires one or more repository-relative paths."
        }

        foreach ($Path in $Rest) {
            Assert-SafeRepoPath $Path
        }

        Invoke-Git (@("add", "--") + $Rest)
    }

    "unstage" {
        if ($Rest.Count -eq 0) {
            throw "unstage requires one or more repository-relative paths."
        }

        foreach ($Path in $Rest) {
            Assert-SafeRepoPath $Path
        }

        Invoke-Git (@("restore", "--staged", "--") + $Rest)
    }

    "commit" {
        if ($Rest.Count -eq 0) {
            throw "commit requires a commit message."
        }

        $Message = ($Rest -join " ").Trim()

        if ([string]::IsNullOrWhiteSpace($Message)) {
            throw "Commit message cannot be empty."
        }

        # Deliberately does not support:
        # --amend
        # --no-verify
        # arbitrary Git flags
        Invoke-Git @("commit", "-m", $Message)
    }

    "push" {
        if ($Rest.Count -gt 0) {
            throw "push does not accept additional arguments."
        }

        $Remote = (& git -C $RepoRoot remote get-url origin).Trim()

        $AllowedHttps = "https://github.com/nguyenthanhtung20891-glitch/PixAuraAI.git"
        $AllowedHttpsNoGit = "https://github.com/nguyenthanhtung20891-glitch/PixAuraAI"
        $AllowedSsh = "git@github.com:nguyenthanhtung20891-glitch/PixAuraAI.git"

        if (
            $Remote -ne $AllowedHttps -and
            $Remote -ne $AllowedHttpsNoGit -and
            $Remote -ne $AllowedSsh
        ) {
            throw "Refusing push: origin does not match the approved PixAuraAI repository."
        }

        $Branch = (& git -C $RepoRoot branch --show-current).Trim()

        if ($Branch -ne "main") {
            throw "Refusing push: autonomous push is allowed only from main."
        }

        # No --force / --force-with-lease / ref deletion supported.
        Invoke-Git @("push", "origin", "main")
    }

    "log" {
        if ($Rest.Count -gt 0) {
            throw "log does not accept additional arguments."
        }

        Invoke-Git @(
            "log",
            "-10",
            "--oneline",
            "--decorate"
        )
    }

    "head" {
        if ($Rest.Count -gt 0) {
            throw "head does not accept additional arguments."
        }

        Invoke-Git @("rev-parse", "HEAD")
    }

    default {
        throw "Unsupported action."
    }
}
