# ADR 0004: AI plans and approval boundary

Status: Accepted
Date: 2026-10-01

## Context
An agent must not gain arbitrary execution or silently overwrite accepted edits.

## Decision
Typed allowlisted plans against a base revision; validate budgets and parameters; render candidates; user approval alone commits. Advisor only suggests; manual and AI share tools/history.

## Alternatives
Direct model-to-filesystem actions are unauditable. Automatic approval undermines user control. Image regeneration for ordinary adjustments breaks deterministic inspectability.

## Consequences
Plan schema/tool versions and stale-revision tests are required. Audit records actions and explanations, not private reasoning. Generative tools are outside MVP.

## Validation
Applicable evidence is required by [QUALITY_GATES.md](../../QUALITY_GATES.md). Implementation status is separate from acceptance of this decision.
