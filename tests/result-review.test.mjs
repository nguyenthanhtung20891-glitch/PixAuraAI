import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import crypto from 'node:crypto';
import {resultReviewHash} from './helpers/review-edit-hashes.mjs';
const text=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n'),read=p=>JSON.parse(text(p)),hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const m=read('docs/reports/phase-8-step-5-v1.json');
const changes=['CMakeLists.txt','packages/core/src/editor_api.cpp','packages/core/swift/Sources/ManualEditor.swift','platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','platforms/android/native/editor_bridge.cpp','platforms/ios/PixAuraAI/ManualEditorModel.swift','platforms/ios/PixAuraAI/ManualEditorScreen.swift'];
const additions=['docs/contracts/result-review-v1.md','packages/core/include/pixaura/result_review.h','packages/core/src/result_review_boundary.hpp','platforms/android/app/src/main/java/ai/pixaura/app/ResultReviewPanel.kt'];
test('Step 5 reads ordinary history and freezes the certified Step 4 engines',()=>{
 assert.equal(m.baseline,'479562255b29cfa263c6de81e9ac8bfdc933dadc');assert.equal(m.step,5);assert.equal(m.status,'AUTHORIZED_IN_PROGRESS');
 assert.equal(m.runtime_authority,false);assert.equal(m.editing_authority,false);assert.equal(m.history_authority,false);assert.equal(m.step6,'NOT_STARTED');
 assert.deepEqual(Object.keys(m.changes).sort(),changes.sort());assert.deepEqual(Object.keys(m.additions).sort(),additions.sort());
 const prior=read('docs/reports/phase-8-step-4-v1.json');
 for(const [p,c] of Object.entries(m.changes)){assert.equal(c.before_sha256_lf,prior.changes[p].after_sha256_lf,p);assert.equal(hash(text(p)),resultReviewHash(p,c.before_sha256_lf),p);}
 for(const [p,h] of Object.entries(m.additions))assert.equal(hash(text(p)),h,p);
 for(const p of Object.keys(prior.additions))assert.equal(hash(text(p)),prior.additions[p],p);
 assert.equal(hash(text('packages/core/src/editor_api.cpp').replace('#include "pixaura/result_review.h"\n','').replace('#include "result_review_boundary.hpp"\n','')),prior.changes['packages/core/src/editor_api.cpp'].after_sha256_lf);
 for(const [p,b] of Object.entries(m.human_apply_bodies)){const source=text(p);assert.equal(hash(source.slice(source.indexOf(b.start),source.indexOf(b.end))),b.sha256_lf,p);}
});
test('result query reconciles a publication receipt and render cannot paint a preview as history',()=>{
 const s=text('packages/core/src/result_review_boundary.hpp');
 for(const term of ['PIXAURA_RESULT_NONE','PIXAURA_RESULT_CURRENT','PIXAURA_RESULT_STALE','PIXAURA_RESULT_UNCONFIRMED','PIXAURA_RESULT_ABSENT','PIXAURA_RESULT_PUBLICATION_VISIBLE','pixaura_working_evaluate','pixaura_preview_render','pixaura_preview_release','pixaura_decode_release','result_match'])assert.ok(s.includes(term),term);
 assert.doesNotMatch(s,/pixaura_editor_agent_(approve|execute)|repository->apply|publish_revision|std::thread|std::async/);
 assert.ok(s.includes('operations.empty()?&s.original:&evaluated'));
 assert.ok(!s.includes('pixaura_preview_begin'));
 const api=text('packages/core/include/pixaura/result_review.h');
 assert.ok(api.includes('sizeof')===false);assert.equal(m.struct_bytes,152);
 for(const term of ['pixaura_result_review_query','pixaura_result_review_render','RESULT REVIEW != NEW HISTORY'])assert.ok(api.includes(term),term);
});
test('platforms present ordinary undo and redo without calculating history',()=>{
 for(const id of ['result-review','result-previous','result-current','result-undo','result-redo','result-back','result-label','result-viewport']){
  assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ResultReviewPanel.kt').includes(id),id);
  assert.ok(text('platforms/ios/PixAuraAI/ManualEditorScreen.swift').includes(id),id);
 }
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ResultReviewPanel.kt','platforms/ios/PixAuraAI/ManualEditorScreen.swift']){
  const s=text(p);for(const label of ['Committed result','Undo document','Redo document','Back to editing','Current Result','Previous'])assert.ok(s.includes(label),p+label);
 }
 assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').includes('fun undoResult() = resultHistory(2)'));
 assert.ok(text('platforms/ios/PixAuraAI/ManualEditorModel.swift').includes('func undoResult() { resultHistory(2) }'));
 assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt').includes('Outcome unconfirmed. Ordinary history remains authoritative.'));
 assert.ok(text('platforms/ios/PixAuraAI/ManualEditorModel.swift').includes('Outcome unconfirmed. Ordinary history remains authoritative.'));
 assert.ok(text('platforms/android/app/src/androidTest/java/ai/pixaura/app/ResultReviewTest.kt').includes('result-undo'));
 assert.ok(text('platforms/ios/PixAuraAIUITests/ShellUITests.swift').includes('testResultReviewUsesOrdinaryUndoRedoAndLeavesEditing'));
 assert.ok(text('packages/core/swift/Tests/ResultReviewTests.swift').includes('testApplyBindsOneOrdinaryRevisionAndUndoRedoRefreshNativeHistory'));
});
test('workflows and native targets include result review without a new test hook',()=>{
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(text(p).includes('tests/result-review.test.mjs'),p);
 const c=text('CMakeLists.txt');assert.ok(c.includes('add_test(NAME result_review_test'));assert.ok(c.includes('add_test(NAME result_review_c_consumer'));
 assert.ok(!text('packages/core/src/result_review_boundary.hpp').includes('pixaura_test_'));
 assert.ok(!text('packages/core/include/pixaura/result_review.h').includes('pixaura_test_'));
});
test('observed Step 4 closure activates COMPLETE and Step 5 stays pending its own exact SHA',()=>{
 const prior=read('docs/reports/evidence/phase-8-step-4-closure-certification.json');
 assert.equal(prior.sha,m.baseline);assert.equal(prior.label,'closure');assert.equal(prior.runtime_authority,false);
 for(const [i,r] of prior.runs.entries()){assert.equal(r.head_sha,m.baseline);assert.equal(r.attempt,1);assert.equal(r.conclusion,'success');assert.equal(r.status,'completed');assert.equal(r.jobs.length,i?3:7);for(const j of r.jobs)assert.equal(j.conclusion,'success');}
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-8-roadmap.md']){
  const s=text(p).split('<!-- phase8-step5-current -->')[1]?.split('<!-- phase8-step5-history -->')[0];assert.ok(s,p);
  for(const term of ['Steps 1-4 COMPLETE / FULL PASS','Step 5 CLOSURE CERTIFICATION PENDING','Step 6 NOT STARTED','RESULT REVIEW != NEW HISTORY','Schema 3 unchanged','HUMAN Apply unchanged','reference.assistance.v1 ADMITTED','reference.deterministic.v1','NONE REQUIRED / NONE ADMITTED','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED','479562255b29cfa263c6de81e9ac8bfdc933dadc','41284220812','41284220803'])assert.ok(s.includes(term),p+term);
  const old=text(p).split('<!-- phase8-step4-current -->')[1]?.split('<!-- phase8-step4-history -->')[0];
  assert.ok(old.includes('Steps 5-6 NOT STARTED'),p);
 }
 assert.ok(text('docs/plans/phase-8-roadmap.md').includes('| 4 | Editable Review Before Apply | COMPLETE / FULL PASS |'));
 assert.ok(text('docs/plans/phase-8-roadmap.md').includes('| 5 | History / Result Review Integration | AUTHORIZED / IN PROGRESS |'));
 assert.ok(text('docs/reports/phase-8-step-5.md').includes('RESULT REVIEW != NEW HISTORY'));
 assert.ok(text('docs/contracts/result-review-v1.md').includes('BEFORE RESULT'));
});
