import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const text = p => fs.readFileSync(p, 'utf8').replaceAll('\r\n', '\n');
const read = p => JSON.parse(text(p));
const files = ['AGENTS.md', 'ARCHITECTURE.md', 'DECISIONS.md', 'QUALITY_GATES.md', 'ROADMAP.md', 'docs/plans/phase-8-roadmap.md'];

test('observed Step 5 closure activates COMPLETE and Step 6 stays unclosed', () => {
  const prior = read('docs/reports/evidence/phase-8-step-5-closure-certification.json');
  assert.equal(prior.sha, 'dd6ac8f342232929fa505f62f29a6db3c17f23e5');
  assert.equal(prior.label, 'closure');
  assert.equal(prior.runtime_authority, false);
  assert.equal(prior.phase8, 'IN_PROGRESS');
  assert.equal(prior.step6, 'NOT_STARTED');
  for (const [i, run] of prior.runs.entries()) {
    assert.equal(run.head_sha, prior.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    assert.equal(run.status, 'completed');
    assert.equal(run.jobs.length, i ? 3 : 7);
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  for (const p of files) {
    const current = text(p).split('<!-- phase8-step6-current -->')[1]?.split('<!-- phase8-step6-history -->')[0];
    assert.ok(current, p);
    for (const term of ['Steps 1-5 COMPLETE / FULL PASS', 'Step 6 CLOSURE CERTIFICATION PENDING', 'eb47353f3088b5d3f42b01b6e5e6b01791d297ba', '41701559434', '41701559457', 'dd6ac8f342232929fa505f62f29a6db3c17f23e5', '41693690260', '41693690048', 'Schema 3 unchanged', 'HUMAN Apply unchanged', 'reference.assistance.v1 ADMITTED', 'reference.deterministic.v1', 'NONE REQUIRED / NONE ADMITTED', 'Vulkan/Metal UNKNOWN', 'NOT WAIVED', 'NOT SATISFIED', 'RESULT REVIEW != NEW HISTORY']) assert.ok(current.includes(term), p + term);
    const old = text(p).split('<!-- phase8-step5-current -->')[1]?.split('<!-- phase8-step5-history -->')[0];
    assert.ok(old.includes('Step 5 CLOSURE CERTIFICATION PENDING'), p);
  }
  const plan = text('docs/plans/phase-8-roadmap.md');
  assert.ok(plan.includes('| 5 | History / Result Review Integration | COMPLETE / FULL PASS |'));
  assert.ok(plan.includes('| 6 | Cross-Platform Polish & Phase 8 Closure | CLOSURE CERTIFICATION PENDING |'));
});

test('both platforms keep preview, restore, and committed result distinct', () => {
  for (const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ReviewPreviewPanel.kt', 'platforms/ios/PixAuraAI/ManualEditorScreen.swift']) {
    const s = text(p);
    for (const label of ['After Preview — not committed', 'Show After Preview', 'Before — bound source revision']) assert.ok(s.includes(label), p + label);
  }
  for (const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ReviewEditPanel.kt', 'platforms/ios/PixAuraAI/ManualEditorScreen.swift']) assert.ok(text(p).includes('Restore original review content — not document undo'), p);
  for (const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ResultReviewPanel.kt', 'platforms/ios/PixAuraAI/ManualEditorScreen.swift']) {
    const s = text(p);
    for (const label of ['Committed result', 'Undo document', 'Redo document', 'Back to editing']) assert.ok(s.includes(label), p + label);
  }
  const boundary = text('packages/core/src/result_review_boundary.hpp');
  assert.ok(!boundary.includes('pixaura_test_'));
  assert.ok(!boundary.includes('http'));
  assert.ok(text('scripts/check-model-release.mjs').includes('review-test-pause'));
  for (const p of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) assert.ok(text(p).includes('tests/phase-8-polish.test.mjs'), p);
  assert.ok(text('docs/reports/phase-8-step-6.md').includes('No unresolved A-class blocker'));
  const closed = read('docs/reports/evidence/phase-8-step-6-closure-certification.json');
  assert.equal(closed.sha, '029f84eaa86f3e5a8bffc696a6abd10f9477ae2a');
  assert.equal(closed.label, 'closure');
  assert.equal(closed.phase8, 'CLOSED_COMPLETE_FULL_PASS');
  for (const [i, run] of closed.runs.entries()) {
    assert.equal(run.head_sha, closed.sha);
    assert.equal(run.attempt, 1);
    assert.equal(run.conclusion, 'success');
    assert.equal(run.jobs.length, i ? 3 : 7);
    for (const job of run.jobs) assert.equal(job.conclusion, 'success');
  }
  assert.ok(text('docs/reports/mvp-readiness-gap-audit.md').includes('BLOCKING MVP'));
  assert.ok(text('AGENTS.md').split('<!-- phase8-closed-current -->')[1].includes('Phase 8 CLOSED / COMPLETE / FULL PASS'));
});
