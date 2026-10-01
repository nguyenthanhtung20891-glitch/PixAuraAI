# ADR 0005: Local inference and capability tiers

Status: Accepted
Date: 2026-10-01

## Context
Models and hardware differ; local editing must survive absent accelerators and memory pressure.

## Decision
Provider-neutral inference, ONNX interchange, CPU/XNNPACK fallback; evaluated Core ML on iOS and optional converted Android LiteRT GPU artifacts. No dependency on deprecated NNAPI. Rules first for basic enhancement; measured progressive capability tiers.

## Alternatives
Cloud-only inference violates offline/privacy constraints. A universal large language model excludes low-tier devices. Selecting a provider by chipset label alone does not prove operator coverage.

## Consequences
Model licenses, conversion parity, quantization quality and download integrity are mandatory. No model weights selected yet. Manual tools and rules work without model packs.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
