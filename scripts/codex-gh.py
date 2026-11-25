#!/usr/bin/env python3
"""Read-only PixAuraAI Actions inspection when PowerShell is unavailable.

No credential/configuration writes, arbitrary API routes, workflow reruns or
repository settings operations. Historical codex-gh.ps1 remains supported.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

REPOSITORY = "nguyenthanhtung20891-glitch/PixAuraAI"


def commands(arguments):
    if not arguments:
        raise ValueError("one allowed action required")
    action, *rest = arguments
    run = lambda value: bool(re.fullmatch(r"[1-9][0-9]{0,19}", value))
    if action == "auth-status" and not rest:
        return [["auth", "status"]]
    if action == "run-list" and len(rest) == 1 and re.fullmatch(r"[a-fA-F0-9]{7,40}", rest[0]):
        return [["run", "list", "--repo", REPOSITORY, "--commit", rest[0], "--limit", "100",
                 "--json", "databaseId,headSha,status,conclusion,workflowDatabaseId,event,url"]]
    if action == "run-view" and len(rest) == 1 and run(rest[0]):
        route = f"repos/{REPOSITORY}/actions/runs/{rest[0]}"
        return [["api", route, "--jq", "{id,head_sha,run_attempt,status,conclusion,workflow_id,event,html_url}"],
                ["run", "view", rest[0], "--repo", REPOSITORY,
                 "--json", "headSha,status,conclusion,jobs,url"]]
    if action == "run-failed" and len(rest) in (1, 2) and all(run(item) for item in rest):
        if len(rest) == 1:
            return [["run", "view", rest[0], "--repo", REPOSITORY, "--log-failed"]]
        route = f"repos/{REPOSITORY}/actions/jobs/{rest[1]}"
        return [["api", route, "--jq", ".run_id"], ["api", route+"/logs", "--allow-escape-sequences"]]
    if action == "run-watch" and len(rest) == 1 and run(rest[0]):
        return [["run", "watch", rest[0], "--repo", REPOSITORY, "--exit-status"]]
    raise ValueError("invalid action, argument count, SHA or numeric run/job ID")


def redact(value):
    # auth status may describe a credential, but this wrapper never exports it.
    return re.sub(r"(?im)^(\s*(?:-\s*)?Token:)\s*.*$", r"\1 [redacted]", value)


def cache_directory(root):
    cache = root/"build"/"gh-cache"
    for part in (root/"build", cache):
        if part.is_symlink():
            raise ValueError("repository cache directory must not be a symlink")
    cache.mkdir(parents=True, exist_ok=True)
    if not cache.resolve().is_relative_to(root.resolve()):
        raise ValueError("repository cache directory escaped its owner")
    return cache


def main(arguments):
    selected = commands(arguments)  # Reject foreign/options/path input BEFORE invoking gh.
    executable = shutil.which("gh")
    if not executable:
        raise ValueError("GitHub CLI is unavailable")
    environment = os.environ.copy()
    environment.pop("GH_DEBUG", None)
    environment.update(GH_PROMPT_DISABLED="1", GH_PAGER="cat", PAGER="cat")
    # Preserve the historical Windows cache hint. On macOS gh may also use
    # normal OS caches, subject to the session sandbox. Never set GH_CONFIG_DIR
    # or alter account/credential configuration.
    cache = cache_directory(Path(__file__).resolve().parents[1])
    environment["LOCALAPPDATA"] = str(cache)
    for index, command in enumerate(selected):
        result = subprocess.run([executable, *command], env=environment, capture_output=True, text=True, check=False)
        if arguments[0] == "run-failed" and len(arguments) == 3 and index == 0:
            if result.returncode or result.stdout.strip() != arguments[1]:
                raise ValueError("job does not belong to the requested workflow run")
            continue
        sys.stdout.write(redact(result.stdout))
        sys.stderr.write(redact(result.stderr))
        if result.returncode:
            return result.returncode
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except (ValueError, OSError) as error:
        print(f"PixAuraAI Actions inspection refused: {error}", file=sys.stderr)
        raise SystemExit(2)
