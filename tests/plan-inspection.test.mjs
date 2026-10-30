import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import crypto from 'node:crypto';
const text=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n'),read=p=>JSON.parse(text(p));
const hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const m=read('docs/reports/phase-8-step-3-v1.json');
const changed=['CMakeLists.txt','packages/core/src/editor_api.cpp','packages/core/swift/Sources/ManualEditor.swift','platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','platforms/android/native/editor_bridge.cpp','platforms/ios/PixAuraAI/ManualEditorModel.swift','platforms/ios/PixAuraAI/ManualEditorScreen.swift'];
const added=['docs/contracts/plan-inspection-action-review-v1.md','packages/core/include/pixaura/review_inspection.h','packages/core/src/review_inspection_boundary.hpp','platforms/android/app/src/main/java/ai/pixaura/app/PlanInspectionPanel.kt'];
test('Step 3 is exact bounded additive inspection, preserving certified Step 2 and frozen engines',()=>{
 assert.equal(m.baseline,'b3d40ae0327d904c8f67995646b249fd95ecfa26');assert.equal(m.step,3);assert.equal(m.runtime_authority,false);assert.equal(m.editing_authority,false);assert.equal(m.step4,'NOT_STARTED');
 assert.deepEqual(Object.keys(m.changes).sort(),changed.sort());assert.deepEqual(Object.keys(m.additions).sort(),added.sort());
 const prior=read('docs/reports/phase-8-step-2-v1.json');
 for(const [p,c] of Object.entries(m.changes)){assert.match(c.before_sha256_lf,/^[a-f0-9]{64}$/);assert.equal(hash(text(p)),c.after_sha256_lf,p);if(prior.changes[p])assert.equal(c.before_sha256_lf,prior.changes[p].after_sha256_lf,p);}
 for(const [p,h] of Object.entries(m.additions))assert.equal(hash(text(p)),h,p);
 const editor=text('packages/core/src/editor_api.cpp').replace('#include "pixaura/review_inspection.h"\n','').replace('#include "review_inspection_boundary.hpp"\n','');assert.equal(hash(editor),m.changes['packages/core/src/editor_api.cpp'].before_sha256_lf);
 const frozen=read('docs/reports/phase-7-closure-v1.json');
 for(const p of ['packages/core/src/document.cpp','packages/core/src/manual_tools.cpp','packages/core/src/evaluation.cpp','packages/core/src/geometry.cpp','packages/core/src/preview.cpp','packages/core/src/agent_execution_boundary.hpp','packages/core/src/budget.cpp','packages/core/src/dynamic_policy.cpp'])assert.equal(hash(text(p)),frozen.sha256[p],p);
 for(const p of ['packages/core/include/pixaura/review_preview.h','packages/core/src/review_preview_boundary.hpp','packages/core/src/review_preview_state.hpp','packages/core/src/review_raster.hpp','docs/contracts/review-preview-foundation-v1.md'])assert.equal(hash(text(p)),prior.additions[p],p);
});
test('native description uses exact existing Plan Registry and source authority without capability or worker',()=>{
 const s=text('packages/core/src/review_inspection_boundary.hpp');
 for(const term of ['auto_current(s,p)','plan==s.auto_plan','canonical_agent_schema','pixaura_agent_tools_bind','manual::registry()','auto_presentation(validated','projection.action_count==p.action_count','std::memcmp(projection.actions,p.actions','repository->read().epoch','raw_value=a.values[j]','parameter.minimum','parameter.maximum','parameter.neutral','std::memcmp(&value,&expected','review_pair(s,*pair)','sizeof(value->review)'])assert.ok(s.includes(term),term);
 assert.doesNotMatch(s,/pixaura_editor_agent_(approve|execute)|repository->commit|publish_revision|std::thread|std::async|clamp/);
 const api=text('packages/core/include/pixaura/review_inspection.h');for(const term of ['actions[8]','parameters[4]','plan_sha256[64]','source_revision[32]','pixaura_review_inspection_preview'])assert.ok(api.includes(term),term);
 assert.equal(m.record_bytes,4808);assert.equal(m.action_bytes,544);assert.equal(m.parameter_bytes,112);assert.equal(m.max_actions,8);assert.equal(m.max_parameters,4);assert.equal(m.max_plan_bytes,8192);
});
test('platform review presents exact raw units and order with no action editing controls',()=>{
 const androidPanel=text('platforms/android/app/src/main/java/ai/pixaura/app/PlanInspectionPanel.kt');
 assert.ok(androidPanel.includes('contentDescription = spoken.joinToString'));assert.ok(!androidPanel.includes('mergeDescendants = true'));
 assert.doesNotMatch(text('platforms/android/app/src/androidTest/java/ai/pixaura/app/PlanInspectionTest.kt'),/onNodeWithTag\("(?:auto-exact-values|review-action-detail|review-geometry-warning)[^"]*",\s*useUnmergedTree/);
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/PlanInspectionPanel.kt','platforms/ios/PixAuraAI/ManualEditorScreen.swift']){
  const panel=text(p).split(p.endsWith('.kt')?'internal fun PlanInspectionPanel':'private struct PlanInspectionPanel')[1];assert.ok(panel);
  for(const id of ['review-action-list','review-plan-count','review-action-','review-action-detail-','review-geometry-warning-','auto-exact-values-'])assert.ok(panel.includes(id),p+id);
  assert.ok(panel.includes('Action '));assert.ok(panel.includes('.raw'));assert.ok(panel.includes('.unit'));assert.ok(panel.includes('.minimum'));assert.ok(panel.includes('.maximum'));assert.ok(panel.includes('.neutral'));
  assert.doesNotMatch(panel,/Slider\(|TextField\(|Button\(|\.sorted|\.sort\(|\.filter|Double\(|Float\(|adviceSetting|approveAgent|executeAgent/);
 }
 for(const p of ['platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','packages/core/swift/Sources/ManualEditor.swift']){
  const s=text(p);for(const term of ['4808','inspectReview','inspection','Inspection'])assert.ok(s.includes(term),p+term);assert.ok(s.includes('nativeInspectionPreview')||s.includes('pixaura_review_inspection_preview'));
 }
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','platforms/ios/PixAuraAI/ManualEditorScreen.swift'])for(const term of ['review-context','review-inspection-stale','review-inspection-unsupported','PlanInspectionPanel','reference.deterministic.v1'])assert.ok(text(p).includes(term),p+term);
});
test('retained complete workflows execute inspection regressions and release-negative fixtures',()=>{
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(text(p).includes('tests/plan-inspection.test.mjs'));
 for(const marker of ['InspectionTestHooks','inspectionCandidateTest','inspection-test-multi'])for(const p of ['scripts/check-android-release.py','scripts/check-model-release.mjs','tests/android-release-scanner.test.py'])assert.ok(text(p).includes(marker),p+marker);
 for(const p of ['packages/core/tests/review_inspection_test.cpp','packages/core/tests/review_inspection_c_consumer.c','packages/core/swift/Tests/PlanInspectionTests.swift','platforms/android/app/src/androidTest/java/ai/pixaura/app/PlanInspectionTest.kt'])assert.ok(text(p).includes('inspection')||text(p).includes('Inspection'));
 const sweep=text('packages/core/tests/review_preview_allocation_test.cpp');for(const s of ['path_index<2','n<4096','pixaura_review_inspect','std::memcmp(&output,&old','status==8','cumulative_requested_max'])assert.ok(sweep.includes(s),s);
});
test('current Step 3 authority requires independent certification and stops before editable review',()=>{
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-8-roadmap.md']){
  const s=text(p).split('<!-- phase8-step3-current -->')[1]?.split('<!-- phase8-step3-history -->')[0];assert.ok(s,p);
  for(const t of ['Steps 1-2 COMPLETE / FULL PASS','Steps 4-6 NOT STARTED','No editable review','Schema 3 unchanged','HUMAN Apply','reference.assistance.v1 ADMITTED','reference.deterministic.v1','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(t),p+t);
  assert.match(s,/Step 3 (AUTHORIZED \/ IN PROGRESS|CLOSURE CERTIFICATION PENDING)/);
 }
 const r=read('docs/reports/evidence/phase-8-step-2-closure-certification.json');assert.equal(r.sha,m.baseline);assert.equal(r.runs.length,2);
 for(const [i,run] of r.runs.entries()){assert.equal(run.head_sha,m.baseline);assert.equal(run.attempt,1);assert.equal(run.conclusion,'success');assert.equal(run.jobs.length,i?3:7);for(const j of run.jobs)assert.equal(j.conclusion,'success');}
 if(m.status==='CLOSURE_CERTIFICATION_PENDING'){assert.match(m.certified_implementation_sha,/^[a-f0-9]{40}$/);for(const [i,r] of m.implementation_certification.runs.entries()){assert.equal(r.head_sha,m.certified_implementation_sha);assert.equal(r.attempt,1);assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i?3:7);for(const j of r.jobs)assert.equal(j.conclusion,'success');}}
 else assert.equal(m.status,'AUTHORIZED_IN_PROGRESS');
});
