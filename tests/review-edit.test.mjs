import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import crypto from 'node:crypto';
import {withoutReviewEditIntegration} from './helpers/review-edit-hashes.mjs';
const text=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n'),read=p=>JSON.parse(text(p)),hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const m=read('docs/reports/phase-8-step-4-v1.json');
test('Step 4 permits only exact bounded review-layer changes and freezes all prior engines',()=>{
 assert.equal(m.baseline,'2610ea2ed49ce2b6eb574ca14f60e0fde00adb67');assert.equal(m.step,4);assert.equal(m.runtime_authority,false);assert.equal(m.editing_authority,false);assert.equal(m.step5,'NOT_STARTED');
 const changes=['CMakeLists.txt','packages/core/src/editor_api.cpp','packages/core/swift/Sources/ManualEditor.swift','platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','platforms/android/native/editor_bridge.cpp','platforms/ios/PixAuraAI/ManualEditorModel.swift','platforms/ios/PixAuraAI/ManualEditorScreen.swift'];
 const additions=['docs/contracts/editable-review-before-apply-v1.md','packages/core/include/pixaura/review_edit.h','packages/core/src/review_edit_state.hpp','packages/core/src/review_edit_boundary.hpp','platforms/android/app/src/main/java/ai/pixaura/app/ReviewEditPanel.kt'];
 assert.deepEqual(Object.keys(m.changes).sort(),changes.sort());assert.deepEqual(Object.keys(m.additions).sort(),additions.sort());
 for(const [p,c] of Object.entries(m.changes)){assert.match(c.before_sha256_lf,/^[a-f0-9]{64}$/);assert.equal(hash(text(p)),c.after_sha256_lf,p);}
 for(const [p,c] of Object.entries(m.additions))assert.equal(hash(text(p)),c,p);
 assert.equal(hash(withoutReviewEditIntegration(text('packages/core/src/editor_api.cpp'))),m.changes['packages/core/src/editor_api.cpp'].before_sha256_lf);
 const old=read('docs/reports/phase-8-step-3-v1.json');for(const p of ['packages/core/include/pixaura/review_inspection.h','packages/core/src/review_inspection_boundary.hpp','docs/contracts/plan-inspection-action-review-v1.md'])assert.equal(hash(text(p)),old.additions[p],p);
 assert.equal(Object.keys(m.human_apply_bodies).length,2);
 for(const [p,b] of Object.entries(m.human_apply_bodies)){const source=text(p);assert.equal(hash(source.slice(source.indexOf(b.start),source.indexOf(b.end))),b.sha256_lf,p+' HUMAN Apply unchanged');}
});
test('native edit is typed bounded new-candidate validation with irreversible old eligibility retirement',()=>{
 const s=text('packages/core/src/review_edit_boundary.hpp');
 for(const term of ['std::try_to_lock','edit_current(s,request->ticket)','repository->read().epoch','inspection_description','s.auto_ready=false','s.auto_plan.clear()','s.review_preview.invalidate','entry.second.state=1','canonical_agent_schema','execution_plan(s,','auto_presentation','s.auto_plan.swap(candidate)','PIXAURA_REVIEW_NO_ACTIONS','PIXAURA_REVIEW_EDIT_FAILED','result.ticket.serial=e.sequence+1','candidate=e.original'])assert.ok(s.includes(term),term);
 assert.doesNotMatch(s,/pixaura_editor_agent_(approve|execute)|repository->apply|publish_revision|std::thread|std::async|clamp|Parser\(/);
 assert.ok(s.includes('std::memcmp(&request->expected_action,&a,sizeof(a))==0'));
 assert.ok(s.indexOf('s.auto_ready=false')<s.indexOf('candidate=e.original'));assert.ok(s.indexOf('execution_plan(s,')<s.indexOf('s.auto_plan.swap(candidate)'));
 const api=text('packages/core/include/pixaura/review_edit.h');for(const term of ['CHANGE_VALUE','REMOVE_ACTION','MOVE_ACTION','RESTORE_ORIGINAL','tool_id[32]','parameter_name[32]','plan_sha256[64]'])assert.ok(api.includes(term),term);
 assert.equal(m.ticket_bytes,136);assert.equal(m.request_bytes,776);assert.equal(m.result_bytes,480);assert.equal(m.max_plan_bytes,8192);assert.equal(m.max_actions,8);assert.equal(m.max_parameters,4);
});
test('platform edit controls propose raw integers and render new native inspection with separate HUMAN Apply',()=>{
 for(const p of ['platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','packages/core/swift/Sources/ManualEditor.swift']){
  const s=text(p);for(const t of ['beginReviewEditing','editReview','session.review','abandoned = true','prepareAutoReview'])assert.ok(s.includes(t),p+t);
  const body=s.split(p.endsWith('.kt')?'fun editReview(':'public func editReview(')[1].split(p.endsWith('.kt')?'private external fun nativeEditBegin':'/// ONLY explicit review Apply')[0];
  assert.doesNotMatch(body,/JSONSerialization|JSONObject|JSONEncoder|approveAgent|executeAgent|Slider|roundToInt/);
 }
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ReviewEditPanel.kt','platforms/ios/PixAuraAI/ManualEditorScreen.swift']){
  const s=text(p);for(const id of ['review-edit-value-','review-edit-change-','review-edit-remove-','review-edit-up-','review-edit-down-','review-edit-restore','review-edit-state'])assert.ok(s.includes(id),p+id);
  for(const t of ['NO_ACTIONS','EDIT_FAILED','ORIGINAL','EDITED','not document undo'])assert.ok(s.includes(t),p+t);
 }
 assert.ok(text('platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt').includes('drafts.isEmpty() && auto.status'));
 assert.ok(text('platforms/ios/PixAuraAI/ManualEditorScreen.swift').includes('reviewDraftIdentity == ObjectIdentifier(review) && !reviewDrafts.isEmpty'));
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/ios/PixAuraAI/ManualEditorModel.swift'])for(const label of ['Invalid owner, session or edit request','Unsupported edit','Invalid resulting Plan','Invalid value or complete Plan parameters','Busy or allocation/resource unavailable','Stale review','Cancelled or owner stopped','Internal or unknown native failure','No document edit was applied by this request.'])assert.ok(text(p).includes(label),p+label);
});
test('complete retained workflows run new native/platform/allocation tests without shipping new hooks',()=>{
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(text(p).includes('tests/review-edit.test.mjs'));
 for(const p of ['packages/core/tests/review_edit_test.cpp','packages/core/tests/review_edit_allocation_test.cpp','packages/core/swift/Tests/ReviewEditTests.swift','platforms/android/app/src/androidTest/java/ai/pixaura/app/ReviewEditTest.kt'])assert.ok(text(p).includes('edit')||text(p).includes('Edit'));
 const c=text('CMakeLists.txt');for(const t of ['add_test(NAME review_edit_test','add_test(NAME review_edit_allocation_test','${review_production_sources}'])assert.ok(c.includes(t),t);
 const sweep=text('packages/core/tests/review_edit_allocation_test.cpp');for(const t of ['kind<5','n<16384','status==8','std::memcmp(&output,&sentinel','state()==stable','cumulative_requested_max'])assert.ok(sweep.includes(t),t);
 for(const t of ['std::array<std::thread,4>','thread_local int64_t fail_after','n=worker','n+=4','last-first<4','failures)==static_cast<uint64_t>(first)','worker.join()'])assert.ok(sweep.includes(t),t);
});
test('current Step 4 authority preserves certified Step 3 and requires fresh exact-SHA implementation and closure',()=>{
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-8-roadmap.md']){
  const s=text(p).split('<!-- phase8-step4-current -->')[1]?.split('<!-- phase8-step4-history -->')[0];assert.ok(s,p);
  for(const t of ['Steps 1-3 COMPLETE / FULL PASS','Steps 5-6 NOT STARTED','Schema 3 unchanged','HUMAN Apply unchanged','NEW Plan/review identity','NO_ACTIONS','reference.assistance.v1 ADMITTED','reference.deterministic.v1','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(t),p+t);
  assert.match(s,/Step 4 (AUTHORIZED \/ IN PROGRESS|CLOSURE CERTIFICATION PENDING)/);
 }
 const prior=read('docs/reports/evidence/phase-8-step-3-closure-certification.json');assert.equal(prior.sha,m.baseline);
 for(const [i,r] of prior.runs.entries()){assert.equal(r.head_sha,m.baseline);assert.equal(r.attempt,1);assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i?3:7);for(const j of r.jobs)assert.equal(j.conclusion,'success');}
 if(m.status==='CLOSURE_CERTIFICATION_PENDING'){assert.match(m.certified_implementation_sha,/^[a-f0-9]{40}$/);for(const [i,r] of m.implementation_certification.runs.entries()){assert.equal(r.head_sha,m.certified_implementation_sha);assert.equal(r.attempt,1);assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i?3:7);for(const j of r.jobs)assert.equal(j.conclusion,'success');}}
 else assert.equal(m.status,'AUTHORIZED_IN_PROGRESS');
});
