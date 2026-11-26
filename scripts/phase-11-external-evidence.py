#!/usr/bin/env python3
"""Operator arithmetic for Phase 11 external evidence.

This tool does not talk to a device, store, or screen reader. It summarizes
numbers the operator already measured, and it rejects records that try to
turn an automated run into physical, sandbox, or human evidence. It never
prints PASS.
"""

import json
import math
import sys

CERTIFIED_SHA = "a3551b2e0fb21896ae80fb6ae8fb7fa195cc2f14"
EXTERNAL_CLASSES = {"physical_device", "authentic_sandbox", "human_accessibility"}
REQUIRED = (
    "gate",
    "evidence_class",
    "result",
    "source_sha",
    "build_kind",
    "device_model",
    "device_os",
    "operator",
    "observed_at",
)


def nearest_rank(samples, percent):
    ordered = sorted(samples)
    rank = math.ceil(percent / 100 * len(ordered))
    return ordered[rank - 1]


def summarize(lines):
    samples = []
    for line in lines:
        text = line.strip()
        if not text or text.startswith("#"):
            continue
        samples.append(float(text))
    if len(samples) < 1:
        raise SystemExit("no samples")
    summary = {
        "tool": "phase-11-external-evidence",
        "result": "NOT_A_GATE",
        "evidence_class": "operator_supplied_samples",
        "count": len(samples),
        "p50": nearest_rank(samples, 50),
        "nearest_rank_p95": nearest_rank(samples, 95),
        "max": max(samples),
        "note": "Computed from operator-supplied samples. Not a device measurement and not acceptance.",
    }
    json.dump(summary, sys.stdout, indent=2)
    sys.stdout.write("\n")


def check(path):
    record = json.load(open(path))
    missing = [key for key in REQUIRED if not str(record.get(key, "")).strip()]
    errors = []
    if missing:
        errors.append("missing " + ", ".join(missing))
    if record.get("evidence_class") not in EXTERNAL_CLASSES:
        errors.append("evidence_class must be physical_device, authentic_sandbox, or human_accessibility")
    if record.get("source_sha") != CERTIFIED_SHA:
        errors.append("source_sha is not the certified Phase 11 source")
    if record.get("automated_substitute") is True:
        errors.append("automated output cannot satisfy an external gate")
    if record.get("result") == "PASS" and errors:
        errors.append("PASS rejected")
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        raise SystemExit(2)
    print("record has the required external provenance fields; this is not acceptance")


def self_check():
    samples = list(range(1, 101))
    if nearest_rank(samples, 50) != 50 or nearest_rank(samples, 95) != 95 or nearest_rank(samples, 100) != 100:
        raise SystemExit("rank check failed")
    ten = list(range(1, 11))
    if nearest_rank(ten, 95) != 10:
        raise SystemExit("p95 rank check failed")
    print("self-check ok")


def main(argv):
    if len(argv) == 2 and argv[1] == "--self-check":
        self_check()
        return
    if len(argv) == 2 and argv[1] == "summarize":
        summarize(sys.stdin)
        return
    if len(argv) == 3 and argv[1] == "check":
        check(argv[2])
        return
    raise SystemExit("usage: phase-11-external-evidence.py --self-check | summarize | check RECORD.json")


if __name__ == "__main__":
    main(sys.argv)
