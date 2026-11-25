#!/usr/bin/env python3
"""Informational Darwin Debug-host measurements; never mobile G8 acceptance.

Runs the actual 4000x3000 shipping C-bridge regression under its synthetic LOW
64MiB native admission, and independently verifies each full-resolution export.
Uses existing stage instrumentation, not a fabricated traversal/latency model.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import statistics
import subprocess
import sys


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--samples", type=int, default=5)
    args = parser.parse_args()
    if sys.platform != "darwin" or args.samples < 1:
        parser.error("requires Darwin /usr/bin/time -l and a positive sample count")
    repository = Path(__file__).resolve().parents[2]
    build, output = args.build.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)  # Preserve every previous receipt.
    tests = json.loads(subprocess.check_output(
        ["ctest", "--test-dir", str(build), "--show-only=json-v1"], text=True))
    command = next(t["command"] for t in tests["tests"] if t["name"] == "low_tier_application_v2_test")
    binary, fixture = Path(command[0]).resolve(), Path(command[2]).resolve()
    if len(command) != 4 or command[1] != "--exercise" or binary.parent != build or fixture.parent != build:
        raise ValueError("unexpected native regression command or source location")
    configuration = next(line.split("=", 1)[1] for line in (build / "CMakeCache.txt").read_text().splitlines()
                         if line.startswith("CMAKE_BUILD_TYPE:"))
    # Exact native source fingerprint, separately from still-uncommitted docs/UI.
    native = {}
    for parent in ("packages/core", "packages/commerce", "packages/editor", "packages/integration"):
        for path in (repository / parent).rglob("*"):
            if path.is_file() and path.suffix in {".c", ".cpp", ".h", ".hpp", ".mm", ".inc", ".cmake"}:
                native[str(path.relative_to(repository))] = sha256(path)
    native["CMakeLists.txt"] = sha256(repository / "CMakeLists.txt")
    source_digest = hashlib.sha256(json.dumps(native, sort_keys=True).encode()).hexdigest()
    stages = ["import complete", "original preview complete", "HumanApply complete", "edited preview complete",
              "history/reopen complete", "JPEG complete", "PNG complete"]
    samples = []
    for index in range(1, args.samples + 1):
        manifest = output / f"sample-{index}.project.txt"
        with (output / f"sample-{index}.stdout.log").open("w") as stdout, (output / f"sample-{index}.time.log").open("w") as timing:
            subprocess.run(["/usr/bin/time", "-l", str(binary), "--exercise", str(fixture), str(manifest)],
                           cwd=output, stdout=stdout, stderr=timing, check=True)
        text = (output / f"sample-{index}.stdout.log").read_text()
        timing = (output / f"sample-{index}.time.log").read_text()
        measured = re.findall(r"^LOW12MP elapsed_ms(\d+) (.+)$", text, re.MULTILINE)
        if [name for _, name in measured] != stages or "Critical32MiB high-entropy denial8" not in text:
            raise ValueError("missing actual stage/admission instrumentation")
        elapsed, durations, previous = {}, {}, 0
        for value, name in measured:
            value = int(value)
            if value < previous:
                raise ValueError("non-monotonic native stage clock")
            elapsed[name], durations[name], previous = value, value - previous, value
        rss = re.search(r"^\s*(\d+)\s+maximum resident set size\s*$", timing, re.MULTILINE)
        if not rss:
            raise ValueError("missing Darwin peak RSS observation")
        with (output / f"sample-{index}.oracle.log").open("w") as oracle:
            subprocess.run([str(binary), "--verify", str(fixture), str(manifest)], cwd=output,
                           stdout=oracle, stderr=subprocess.STDOUT, check=True)
        samples.append({"index": index, "cumulative_stage_ms": elapsed, "stage_delta_ms": durations,
                        "darwin_peak_rss_bytes": int(rss.group(1)), "full_export_oracle": "PASS"})
        print(f"Host sample {index}/{args.samples}: real12MP stages and independent full export oracle PASS", flush=True)
    report = {
        "scope": "native Debug/configured host; synthetic LOW64MiB admission; NOT physical/mobile/UI/p95 acceptance",
        "configuration": configuration, "samples": samples, "sample_count": len(samples),
        "methodology": "No warmup exclusion, cache eviction or thermal control. Other validation may run concurrently. Stage deltas include API/control bookkeeping between existing markers. Oracle time/RSS is excluded. No p95 or threshold PASS claimed.",
        "source_head_at_measurement": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repository, text=True).strip(),
        "native_source_digest": source_digest, "native_source_sha256": native,
        "binary_sha256": sha256(binary), "library_sha256": sha256(build / "libpixaura_core.dylib"),
        "source_fixture_sha256": sha256(fixture), "source_fixture_bytes": fixture.stat().st_size,
        "script_sha256": sha256(Path(__file__).resolve()),
        "stage_delta_ms_descriptive": {name: {"median": statistics.median(s["stage_delta_ms"][name] for s in samples),
                                               "min": min(s["stage_delta_ms"][name] for s in samples),
                                               "max": max(s["stage_delta_ms"][name] for s in samples)} for name in stages},
        "peak_rss_bytes_observed": max(s["darwin_peak_rss_bytes"] for s in samples),
        "rss_is_admission_ceiling": False, "mobile_cohort_qualified": False,
    }
    (output / "measurements.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
