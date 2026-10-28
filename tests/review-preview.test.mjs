import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
const read=p=>JSON.parse(text(p));
const hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const m=read('docs/reports/phase-8-step-2-v1.json');
const changed=['CMakeLists.txt','packages/core/src/agent_schema_boundary.hpp','packages/core/src/decode_api.cpp','packages/core/src/editor_api.cpp','packages/core/swift/Sources/ManualEditor.swift','platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt','platforms/ios/PixAuraAI/ManualEditorModel.swift','platforms/ios/PixAuraAI/ManualEditorScreen.swift'];
const added=['docs/contracts/review-preview-foundation-v1.md','packages/core/include/pixaura/review_preview.h','packages/core/src/review_preview_boundary.hpp','packages/core/src/review_preview_state.hpp','packages/core/src/review_raster.hpp','platforms/android/app/src/main/java/ai/pixaura/app/ReviewPreviewPanel.kt'];

test('Step 2 permits only individually bound preview integration; existing contracts and editing engines stay frozen',()=>{
 assert.equal(m.phase,8);assert.equal(m.step,2);assert.equal(m.baseline,'7aa21dc449de5143ddf1af7998ea7e233bb691b7');
 assert.equal(m.preview_publication_authority,false);assert.equal(m.runtime_authority,false);assert.equal(m.step3,'NOT_STARTED');
 assert.deepEqual(Object.keys(m.changes).sort(),changed.sort());assert.deepEqual(Object.keys(m.additions).sort(),added.sort());
 const frozen=read('docs/reports/phase-7-closure-v1.json');
 for(const [p,v] of Object.entries(m.changes)){assert.equal(v.before_sha256_lf,frozen.sha256[p],p);assert.equal(hash(text(p)),v.after_sha256_lf,p);}
 for(const [p,v] of Object.entries(m.additions))assert.equal(hash(text(p)),v,p);
 // Remove only the new orchestration include/block: all prior decode functions
 // must still be byte-identical, including admission, evaluation and preview APIs.
 const old=text('packages/core/src/decode_api.cpp').replace('#include "review_raster.hpp"\n','').replace(/\/\/ BEGIN Phase 8 Step 2 non-publication review composition\.\n[\s\S]*?\/\/ END Phase 8 Step 2 non-publication review composition\.\n/,'');
 assert.equal(hash(old),frozen.sha256['packages/core/src/decode_api.cpp']);
 // Exact Registry binding construction, not a schema/authority change. Removing
 // only this root-caused exception-safety correction restores the frozen bytes.
 const binding=text('packages/core/src/agent_schema_boundary.hpp');
 const original=binding.replace(/            \/\/ Chained base-string operator\+[\s\S]*?            canonical.value=std::move\(binding\);/, '            canonical.value=String("{\\"approval\\":\\"not_approved\\",\\"binding_version\\":1,\\"plan\\":")+canonical.value+",\\"registry_version\\":1}\\n";');
 assert.equal(hash(original),frozen.sha256['packages/core/src/agent_schema_boundary.hpp']);
 for(const p of ['packages/core/src/evaluation.cpp','packages/core/src/geometry.cpp','packages/core/src/preview.cpp','packages/core/src/document.cpp','packages/core/src/agent_execution_boundary.hpp','packages/core/src/manual_tools.cpp','packages/core/src/budget.cpp','packages/core/src/dynamic_policy.cpp'])assert.equal(hash(text(p)),frozen.sha256[p],p);
 assert.equal(hash(text('docs/contracts/full-review-experience-v1.md')),read('docs/reports/phase-8-step-1-v1.json').contract_sha256_lf);
});
test('native review composition has only image ownership, exact validation and existing raster semantics',()=>{
 const state=text('packages/core/src/review_preview_state.hpp'),api=text('packages/core/src/review_preview_boundary.hpp');
 for(const term of ['approval_issuer','candidate_handle','Repository','std::thread','std::async'])assert.ok(!state.includes(term),term);
 for(const term of ['pixaura_editor_agent_approve','pixaura_editor_agent_execute','publish_revision','repository->commit','executeAgentPlan'])assert.ok(!api.includes(term),term);
 for(const term of ['plan==s.auto_plan','canonical==plan','pixaura_agent_tools_bind','repository->read().epoch','policy_epoch','review_bound(s)','review_ticket(s,*ticket)','!r.active','UINT64_MAX','pixaura_cancel_release'])assert.ok(api.includes(term),term);
 const raster=text('packages/core/src/decode_api.cpp').split('// BEGIN Phase 8 Step 2')[1].split('// END Phase 8 Step 2')[0];
 for(const term of ['PIXAURA_BUDGET_PREVIEW','policy::admit','evaluation::reservation','evaluation::evaluate','geometry::plan','preview::render','q.retained_bytes=retained_locked','q.requested_handles=2','edge)*edge*4*8-automatic_display','std::lock_guard<std::mutex> pixels'])assert.ok(raster.includes(term),term);
 for(const term of ['repository','approval','candidate','std::thread','std::async'])assert.ok(!raster.includes(term),term);
 assert.equal(m.display_capacity_count,8);assert.equal(m.ticket_bytes,56);assert.equal(m.pair_bytes,176);assert.equal(m.max_actions,8);assert.equal(m.request_plan_bytes,8192);
});
test('both platforms expose separate semantic preview controls and native-gated installation',()=>{
 const kotlin=text('platforms/android/app/src/main/java/ai/pixaura/app/ReviewPreviewPanel.kt');
 const swift=text('platforms/ios/PixAuraAI/ManualEditorScreen.swift');
 for(const id of ['review-preview-request','review-before','review-after','review-preview-loading','review-preview-cancel','review-preview-stale','review-preview-failed','review-toggle','review-split','review-divider','review-zoom','review-pan-x','review-pan-y','review-reset']){assert.ok(kotlin.includes(id),id);assert.ok(swift.includes(id),id);}
 assert.ok(kotlin.includes('enabled = !pair.geometryChanged'));assert.ok(swift.includes('.disabled(pair.geometryChanged)'));
 const k=text('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt'),s=text('packages/core/swift/Sources/ManualEditor.swift');
 assert.ok(k.includes('nativeReviewCurrent'));assert.ok(k.includes('installReviewPreview'));assert.ok(k.includes('assistCalls++'));
 assert.ok(s.includes('pixaura_review_preview_current'));assert.ok(s.includes('installReviewPreview'));assert.ok(s.includes('assistCalls += 1'));
 const model=text('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt');
 assert.ok(model.includes('reviewAttempt.get() !== attempt'));assert.ok(model.includes('reviewRequest.get() === call'));assert.ok(model.includes('reviewRendering.get()'));
 const apple=text('platforms/ios/PixAuraAI/ManualEditorModel.swift');assert.ok(apple.includes('reviewAdmission == admission'));assert.ok(apple.includes('if abandoned'));
 for(const p of ['platforms/android/app/src/androidTest/java/ai/pixaura/app/ReviewPreviewTest.kt','packages/core/swift/Tests/ReviewPreviewTests.swift','platforms/ios/PixAuraAIUITests/ShellUITests.swift'])assert.ok(text(p).includes('reviewPreview')||text(p).includes('ReviewPreview'));
});
test('preview tests and release marker rejection remain in both complete workflows',()=>{
 const c=text('CMakeLists.txt');for(const name of ['review_preview_test','review_preview_c_consumer','review_preview_allocation_test'])assert.ok(c.includes(`add_test(NAME ${name}`),name);
 assert.ok(c.includes('get_target_property(review_production_sources pixaura_core SOURCES)'));
 const defs=c.match(/target_compile_definitions\(review_preview_allocation_test PRIVATE ([^)]+)\)/)?.[1];assert.equal(defs,'${review_production_definitions}');
 assert.ok(c.includes('get_target_property(review_production_definitions pixaura_core COMPILE_DEFINITIONS)'));
 assert.ok(c.includes('list(FILTER review_production_definitions EXCLUDE REGEX "^PIXAURA_.*TEST")'));
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(text(p).includes('tests/review-preview.test.mjs'));
 for(const marker of ['ReviewPreviewTestHooks','reviewPreviewTestControl','review-preview-test-pause','review-preview-test-failure']){
  assert.ok(text('scripts/check-android-release.py').includes(marker));assert.ok(text('scripts/check-model-release.mjs').includes(marker));assert.ok(text('tests/android-release-scanner.test.py').includes(marker));
 }
 const n=text('packages/core/tests/review_preview_test.cpp');for(const term of ['tools=11','scenario<8','count:{2u,8u}','i<9','bench.ordinary()==expected','pixaura_test_agent_external_session'])assert.ok(n.includes(term),term);
 const fixture='tests/fixtures/review-preview-v1/gradient.png';assert.ok(text('tests/fixtures/review-preview-v1/README.md').includes(hash(fs.readFileSync(fixture))));
});
test('current Step 2 authority preserves Step 1 observed closure and later-step stop',()=>{
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-8-roadmap.md']){
  const s=text(p).split('<!-- phase8-step2-current -->')[1]?.split('<!-- phase8-step2-history -->')[0];assert.ok(s,p);
  for(const term of ['Phase 7 CLOSED / COMPLETE / FULL PASS','Step 1 COMPLETE / FULL PASS','Steps 3-6 NOT STARTED','HUMAN Apply','Schema 3','reference.assistance.v1 ADMITTED','reference.deterministic.v1','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(term),p+term);
  assert.match(s,/Step 2 (AUTHORIZED \/ IN PROGRESS|CLOSURE CERTIFICATION PENDING)/);
 }
 const r=read('docs/reports/evidence/phase-8-step-1-closure-certification.json');assert.equal(r.sha,m.baseline);assert.equal(r.runs.length,2);
 for(const [i,run] of r.runs.entries()){assert.equal(run.head_sha,m.baseline);assert.equal(run.attempt,1);assert.equal(run.conclusion,'success');assert.equal(run.jobs.length,i?3:7);for(const j of run.jobs){assert.equal(j.status,'completed');assert.equal(j.conclusion,'success');}}
 if(m.status==='CLOSURE_CERTIFICATION_PENDING'){
  assert.match(m.certified_implementation_sha,/^[a-f0-9]{40}$/);
  for(const [i,r] of m.implementation_certification.runs.entries()){assert.equal(r.head_sha,m.certified_implementation_sha);assert.equal(r.attempt,1);assert.equal(r.status,'completed');assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i?3:7);for(const j of r.jobs)assert.equal(j.conclusion,'success');}
 }else assert.equal(m.status,'AUTHORIZED_IN_PROGRESS');
});
