import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');


test('Phase 7 Step 6 lifecycle authority and cancellation admission remain coherent',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-6.md'];
 const states=paths.map(p=>{
  const current=read(p).split('<!-- phase7-step6-current-authority -->')[1]?.split('<!-- phase7-step6-history -->')[0];assert.ok(current,p);
  const state=current.match(/^Step 6 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(state,p);
  for(const marker of ['Step 5 COMPLETE / FULL PASS','Phase 6 CLOSED / COMPLETE / FULL PASS','Phase 7 Steps 7-8 NOT STARTED','Phase 8 NOT STARTED','Production model UNRESOLVED','CPU VALIDATED','Vulkan/Metal UNKNOWN','Schema 3','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(marker),p+marker);
  if(state[1]==='COMPLETE / FULL PASS'){assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);assert.ok(!current.includes('Exact-SHA certification pending'));}
  return state[1];
 });assert.equal(new Set(states).size,1);
 assert.ok(read('docs/plans/phase-7-roadmap.md').includes('| 6 | Cross-Platform Lifecycle & Interaction Integration | '+states[0]+' |'));
 const k=read('platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt');
 assert.ok(k.includes('cancellingAdvice.get() || autoCancelling.get() || cancellingApplication.get()'));
 assert.ok(k.includes('admission occupied through native validation/presentation'));
 const apple=read('platforms/ios/PixAuraAI/ManualEditorModel.swift');assert.equal(apple.split('if assistWorking || cancellingAdvice || cancellingAuto').length-1,3);
 assert.ok(apple.includes('closingWorker.commands.async { closingWorker.close() }'));
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/ios/PixAuraAI/ManualEditorModel.swift'])assert.doesNotMatch(read(p),/SavedStateHandle|encode.*approval|put.*approval|set.*approval.*forKey/);
 const contract=read('docs/contracts/ai-interaction-lifecycle-integration-v1.md');
 for(const marker of ['MANUAL | ASSIST','ASSIST | MANUAL','MANUAL | AUTO','AUTO | MANUAL','ASSIST | AUTO','AUTO | ASSIST','stop -> join ALL','Process death','Schema 3 unchanged','Steps 7-8 NOT STARTED'])assert.ok(contract.includes(marker),marker);
});

test('Phase 7 Step 5 review uses only explicit human Apply and retained native authority',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-5.md'];
 const states=paths.map(p=>{
  const current=read(p).split('<!-- phase7-step5-current-authority -->')[1]?.split('<!-- phase7-step5-history -->')[0];assert.ok(current,p);
  const match=current.match(/^Step 5 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(match,p);
  for(const marker of ['Step 4 COMPLETE / FULL PASS','Phase 6 CLOSED / COMPLETE / FULL PASS','Phase 7 Steps 6-8 NOT STARTED','Phase 8 NOT STARTED','Production model UNRESOLVED','CPU VALIDATED','Vulkan/Metal UNKNOWN','Schema 3','NOT WAIVED','NOT SATISFIED','No automatic approval'])assert.ok(current.includes(marker),p+marker);
  if(match[1]==='COMPLETE / FULL PASS'){assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);assert.ok(!current.includes('Exact-SHA certification pending'))}
  return match[1];
 });assert.equal(new Set(states).size,1);
 assert.ok(read('docs/plans/phase-7-roadmap.md').includes('| 5 | Plan Review & Explicit Approval UX | '+states[0]+' |'));
 const kotlin=read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt'),swift=read('packages/core/swift/Sources/ManualEditor.swift');
 for(const s of [kotlin,swift])for(const marker of ['prepareAutoReview','applyAutoFromHuman','cancelAutoReview','attemptedAutoSerial','applyingApproval','sourceRevision','canonicalBytes','cancelAgentExecution'])assert.ok(s.includes(marker),marker);
 for(const [p,call] of [['platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt','model.applyAutoFromHuman(review)'],['platforms/ios/PixAuraAI/ManualEditorScreen.swift','model.applyAutoFromHuman(review)']]){
  const s=read(p);assert.equal(s.split(call).length-1,1);for(const id of ['auto-apply','auto-execution-cancel','auto-completed','auto-source','auto-exact-values-'])assert.ok(s.includes(id),p+id);
  assert.doesNotMatch(s,/approveAgentPlan|executeAgentPlan|pixaura_editor_agent_|\.commit\(/);
 }
 for(const p of ['platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','platforms/ios/PixAuraAI/ManualEditorModel.swift']){
  const s=read(p);assert.ok(s.includes('REVIEW_READY')||s.includes('.reviewReady'));assert.ok(s.includes('receipt'));assert.ok(s.includes('cancelRequested'));
  assert.doesNotMatch(s,/approveAgentPlanFromHuman|pixaura_editor_agent_approve|for.*executeAgentPlan/);
 }
 for(const marker of ['reviewTestControl','review-test-pause','review-test-failure'])assert.ok(read('scripts/check-model-release.mjs').includes(marker));
 const appleModel=read('platforms/ios/PixAuraAI/ManualEditorModel.swift').split('@MainActor\nfinal class ManualEditorModel:');assert.equal(appleModel.length,2);
 assert.doesNotMatch(appleModel[0],/\bautoProposal\b/,'worker must not own/read published UI interaction state');
 assert.ok(appleModel[1].includes('func cancelAuto() {\n        if autoProposal.status == .completed { return }'));
 const contract=read('docs/contracts/ai-auto-edit-review-approval-v1.md');for(const marker of ['No new public C ABI','Revoke','COMPLETED','Schema 3','Phase 8 NOT STARTED','No automatic'])assert.ok(contract.toLowerCase().includes(marker.toLowerCase()),marker);
});

test('Phase 7 Step 4 authority and proposal-only transport agree without Step 5',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-4.md'];
 const states=paths.map(p=>{
  const current=read(p).split('<!-- phase7-step4-current-authority -->')[1]?.split('<!-- phase7-step4-history -->')[0];assert.ok(current,p);
  const state=current.match(/^Step 4 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(state,p);
  for(const marker of ['Phase 6 CLOSED / COMPLETE / FULL PASS','Step 1 COMPLETE / FULL PASS','Step 2 COMPLETE / FULL PASS','Step 3 COMPLETE / FULL PASS','Phase 7 Steps 5-8 NOT STARTED','Phase 8 NOT STARTED','Production model UNRESOLVED','CPU VALIDATED','Vulkan/Metal UNKNOWN','Schema 3','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(marker),p+marker);
  if(state[1]==='COMPLETE / FULL PASS'){assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);assert.ok(!current.includes('Exact-SHA certification pending'))}
  return state[1];
 });assert.equal(new Set(states).size,1);
 assert.ok(read('docs/plans/phase-7-roadmap.md').includes('| 4 | AI Auto-Edit Proposal Flow | '+states[0]+' |'));
 const boundary=read('packages/core/src/auto_proposal_boundary.hpp');
 for(const marker of ['assistance::validate','auto_presentation','s.live.serial==source','s.loaded.epoch==epoch','s.auto_sequence==serial','s.auto_active&&!s.assist_active','auto_current(s,*value)'])assert.ok(boundary.includes(marker),marker);
 assert.doesNotMatch(boundary,/pixaura_editor_agent_(?:approve|execute)|pixaura_manual_|repository->|\.accept\(|commit_candidate/);
 for(const file of ['platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt'])assert.doesNotMatch(read(file),/model\.(?:action|useSuggestion|select|update|execute|approve)/);
 const header=read('packages/core/include/pixaura/auto_proposal.h');assert.equal((header.match(/PIXAURA_API int32_t/g)||[]).length,4);
 for(const marker of ['pixaura_test_auto_control','AutoTestHooks_nativeControl','auto-test-pause','auto-test-failure'])assert.ok(read('scripts/check-model-release.mjs').includes(marker));
 for(const marker of ['auto-ready','auto-goal','auto-request','auto-cancel','auto-dismiss','auto-error']){
  assert.ok(read('platforms/android/app/src/main/java/ai/pixaura/app/AutoProposalPanel.kt').includes(marker));assert.ok(read('platforms/ios/PixAuraAI/ManualEditorScreen.swift').includes(marker));
 }
 assert.ok(read('docs/contracts/ai-auto-edit-proposal-v1.md').includes('REVIEW_READY -> STOP'));
});

test('Phase 7 Step 3 current authority, manual-only navigation, and release isolation agree',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-3.md'];
 const states=paths.map(p=>{
  const current=read(p).split('<!-- phase7-step3-current-authority -->')[1]?.split('<!-- phase7-step3-history -->')[0];assert.ok(current,p);
  const state=current.match(/^Step 3 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(state,p);
  for(const marker of ['Phase 6 CLOSED / COMPLETE / FULL PASS','Step 1 COMPLETE / FULL PASS','Step 2 COMPLETE / FULL PASS','Phase 7 Steps 4-8 NOT STARTED','Phase 8 NOT STARTED','Production model UNRESOLVED','CPU VALIDATED','Vulkan/Metal UNKNOWN','Schema 3','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(marker),p+marker);
  if(state[1]==='COMPLETE / FULL PASS'){assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);assert.ok(!current.includes('Exact-SHA certification pending'))}
  return state[1];
 });assert.equal(new Set(states).size,1);
 assert.ok(read('docs/plans/phase-7-roadmap.md').includes('| 3 | Manual + AI Assist Experience Foundation | '+states[0]+' |'));
 const contract=read('docs/contracts/ai-assist-experience-v1.md');assert.ok(contract.includes('navigation-only, NOT Apply'));assert.ok(contract.includes('stop, joins all render/Agent/advice calls'));
 for(const [path,start,end] of [
  ['platforms/android/app/src/main/java/ai/pixaura/app/ManualEditorModel.kt','fun useSuggestion()','private fun command'],
  ['platforms/ios/PixAuraAI/ManualEditorModel.swift','func useAdvice(','func update(']]) {
  const body=read(path).split(start)[1]?.split(end)[0];assert.ok(body,path);assert.ok(body.includes('.inspectAdvice(')&&body.includes('.select('),path);
  assert.doesNotMatch(body,/\.(?:update|action|executeAgentPlan|approveAgentPlanFromHuman|render)\(/,path);
 }
 const boundary=read('packages/core/src/assist_experience_boundary.hpp');
 assert.doesNotMatch(boundary,/pixaura_editor_agent_(?:execute|approve)|\.accept\(|commit_candidate\(/);
 for(const marker of ['pixaura_test_assist_control','AssistTestHooks_nativeControl','assist-test-failure','assist-test-pause'])assert.ok(read('scripts/check-model-release.mjs').includes(marker));
 for(const path of ['platforms/android/app/src/main/java/ai/pixaura/app/AssistPanel.kt','platforms/ios/PixAuraAI/ManualEditorScreen.swift'])for(const id of ['assist-request','assist-goal','assist-use','assist-dismiss','assist-error','assist-cancel'])assert.ok(read(path).includes(id),path+id);
});

test('Phase 7 Step 1 authority agrees without reopening Phase 6 or authorizing later steps',()=>{
 const roots=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md'];
 const files=[...roots,'docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-1.md'];
 const states=files.map(p=>{
  const s=read(p),current=roots.includes(p)?s.split('<!-- phase7-current-authority -->')[1]?.split('<!-- phase7-history -->')[0]:s;
  assert.ok(current,p);const m=current.match(/^Step 1 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(m,p);
  for(const marker of ['Phase 6 CLOSED / COMPLETE / FULL PASS','Phase 7 AUTHORIZED / IN PROGRESS','Phase 7 Steps 2-8 NOT STARTED','Phase 8','NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(marker),p+': '+marker);
  return m[1];
 });assert.equal(new Set(states).size,1);
 const rows=[...read('docs/plans/phase-7-roadmap.md').matchAll(/^\| (\d+) \| ([^|]+) \| ([^|]+) \|$/gm)];
 assert.deepEqual(rows.map(m=>+m[1]),[1,2,3,4,5,6,7,8]);assert.equal(rows[0][3],states[0]);for(const row of rows.slice(1,6))assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(row[3]));assert.deepEqual(rows.slice(6).map(m=>m[3]),Array(2).fill('NOT STARTED')); // Later authorized steps are checked separately; Step 1 remains historical.
 assert.deepEqual(rows.map(m=>m[2]),['AI-Assisted MVP Contract & Interaction State Machine','Assistance Provider & Proposal Envelope v1','Manual + AI Assist Experience Foundation','AI Auto-Edit Proposal Flow','Plan Review & Explicit Approval UX','Cross-Platform Lifecycle & Interaction Integration','Production Intelligence Admission & MVP Provider','Phase 7 Closure']);
});

test('Phase 7 Step 2 authority and native non-executing provider boundary agree',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md','docs/reports/phase-7-step-2.md'];
 const states=paths.map(p=>{
  const s=read(p).split('<!-- phase7-step2-current-authority -->')[1]?.split('<!-- phase7-step2-history -->')[0];assert.ok(s,p);
  const state=s.match(/^Step 2 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(state,p);
  for(const marker of ['Phase 6 CLOSED / COMPLETE / FULL PASS','Step 1 COMPLETE / FULL PASS','Phase 7 Steps 3-8 NOT STARTED','Phase 8 NOT STARTED','Production model UNRESOLVED','CPU VALIDATED','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(marker),p+': '+marker);
  if(state[1]==='COMPLETE / FULL PASS'){
   assert.match(s,/Implementation certification SHA: `[a-f0-9]{40}`/);
   assert.match(s,/Foundation \d+ SUCCESS 7\/7/);assert.match(s,/Native application shells \d+ SUCCESS 3\/3/);
   assert.ok(!s.includes('Exact-SHA certification pending'));
  }
  return state[1];
 });assert.equal(new Set(states).size,1);
 const roadmap=read('docs/plans/phase-7-roadmap.md');assert.ok(roadmap.includes('| 2 | Assistance Provider & Proposal Envelope v1 | '+states[0]+' |'));
 const header=read('packages/core/src/assistance.hpp');
 for(const marker of ['manual::find','manual::agent_exposable','try_to_lock','epoch!=epoch_','stopped_=true','class Provider','class ReferenceProvider','envelope_limit=12288'])assert.ok(header.includes(marker),marker);
 for(const denied of ['editor.h','agent_execution.h','pixaura_editor_','pixaura_agent_approve','std::thread','getenv(','dlopen(','system('])assert.ok(!header.includes(denied),denied);
 const codec=read('packages/core/src/assistance_codec.inc');for(const marker of ['Parser','Writer','check_agent_binding','plan_structured_intent','canonical_agent_schema','result.value.envelope==input'])assert.ok(codec.includes(marker),marker);
 assert.ok(read('CMakeLists.txt').includes('NAME assistance_test'));
 assert.ok(read('scripts/check-model-release.mjs').includes('Assistance v1 checks='));
 assert.ok(read('scripts/check-model-release.mjs').includes('assistance_checkpoint'));
 assert.equal((read('CMakeLists.txt').match(/PIXAURA_ASSISTANCE_TESTING/g)||[]).length,1);
 assert.ok(!read('packages/core/Package.swift').includes('PIXAURA_ASSISTANCE_TESTING'));
 const contract=read('docs/contracts/assistance-provider-proposal-v1.md');for(const marker of ['12288','8192','2048','512','1..64','ADVICE','STRUCTURED_INTENT','PLAN_CANDIDATE','stop -> join ALL calls -> destroy','No new public C ABI','No clamping','Schema 3 change'])assert.ok(contract.includes(marker),marker);
});

test('MVP contract has finite mode/state and transition documentation with explicit authority guards',()=>{
 const s=read('docs/contracts/ai-assisted-mvp-v1.md');
 const modes=[...s.matchAll(/^\| (MODE_[A-Z]+) \|/gm)].map(m=>m[1]);assert.deepEqual(modes,['MODE_MANUAL','MODE_ASSIST','MODE_AUTO']);
 const states=[...s.matchAll(/^\| ([A-Z_]+) \| [^|]+ \|$/gm)].map(m=>m[1]);
 assert.deepEqual(states,['IDLE','REQUESTING','PROPOSAL_AVAILABLE','REVIEWING','APPROVAL_REQUIRED','EXECUTING','COMPLETED','REJECTED','CANCELLED','STALE','FAILED']);
 const rows=[...s.matchAll(/^\| ([A-Z_, ]+) \| ([a-z_]+) \| ([A-Z_]+) \| ([^|]+) \|$/gm)];assert.equal(rows.length,20);
 const keys=new Set();for(const [,from,event,to,guard] of rows){assert.ok(states.includes(to));assert.ok(guard.trim().length>20);for(const state of from.split(', ')){assert.ok(states.includes(state));const key=state+'/'+event;assert.ok(!keys.has(key));keys.add(key);}}
 const admission=rows.filter(m=>m[3]==='EXECUTING'&&m[1]!=='EXECUTING');assert.equal(admission.length,1);assert.equal(admission[0][1],'APPROVAL_REQUIRED');assert.equal(admission[0][2],'human_apply_admitted');assert.ok(admission[0][4].includes('native execution admission consumes'));
 const completed=rows.filter(m=>m[3]==='COMPLETED');assert.equal(completed.length,1);assert.equal(completed[0][2],'native_published_receipt');
 for(const marker of ['Advice grants no mutation','not new ABI enum values','never an automatic loop','No model/runtime/weights','Schema 3 unchanged','stop -> join ALL calls -> destroy','Step 2 owns Assistance Provider & Proposal Envelope v1','unlisted'])assert.ok(s.toLowerCase().includes(marker.toLowerCase()),marker);
});

test('MVP provider/privacy/deferred boundaries preserve frozen trust and minimum future review scope',()=>{
 const s=read('docs/contracts/ai-assisted-mvp-v1.md');
 for(const marker of ['UNTRUSTED Assistance Proposal','provider NEVER receives an approval issuer','Free-form Intent v1 remains non-interpreted','No provider CERTIFIED','cpu.reference.v1 VALIDATED','android.vulkan.v1 UNKNOWN','apple.metal.v1 UNKNOWN','Production model UNRESOLVED','No persistence','NO network requirement','checkpoint rights unresolved','Phase 8 FULL REVIEW EXPERIENCE remains NOT STARTED'])assert.ok(s.includes(marker),marker);
 for(const category of ['A authority bypass','B forged/malformed output','C stale proposal','D approval confusion/replay','E mode-switch leakage','F platform mismatch','G owner lifetime','H cancel/publish race','I resource exhaustion','J test-hook leakage','K privacy exposure'])assert.ok(s.includes('| '+category+' |'));
});

test('Phase 6 closure current authority, eight-step state and frozen contracts agree',()=>{
 const paths=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-6-roadmap.md'];
 const states=paths.map(p=>{
  const current=read(p).split('## Phase 6 Step 8 - current authority')[1]?.split('Historical step authority follows')[0];
  assert.ok(current,p);const m=current.match(/^Step 8 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)\./m);assert.ok(m,p);
  for(const marker of ['Phase 7 NOT STARTED','Production model UNRESOLVED','Schema 3','DH-APPLE-METAL-01','NOT WAIVED','NOT SATISFIED','39613164583 SUCCESS 7/7','39613164251 SUCCESS 3/3'])assert.ok(current.includes(marker),p+': '+marker);
  return m[1];
 });
 assert.equal(new Set(states).size,1);
 const rows=[...read('docs/plans/phase-6-roadmap.md').matchAll(/^\| (\d+) \| [^|]+ \| ([^|]+) \|$/gm)];
 assert.deepEqual(rows.map(m=>+m[1]),[1,2,3,4,5,6,7,8]);assert.deepEqual(rows.slice(0,7).map(m=>m[2]),Array(7).fill('COMPLETE / FULL PASS'));assert.equal(rows[7][2],states[0]);
 const inventory=JSON.parse(read('docs/reports/phase-6-closure-v1.json'));assert.equal(inventory.runtime_authority,false);assert.equal(inventory.contracts.length,8);
 for(const c of inventory.contracts)assert.equal(crypto.createHash('sha256').update(read(c.path)).digest('hex'),c.sha256_lf,c.path+': frozen contract drift');
 const report=read('docs/reports/phase-6-closure.md');assert.ok(report.includes('Step 8 '+states[0]));
 for(const marker of ['stop → join ALL calls → destroy','No provider CERTIFIED','UNINCORPORATED','Source serial','epoch CAS','Phase 7','only after that observation'])assert.ok(report.includes(marker),marker);
 if(states[0]==='COMPLETE / FULL PASS')for(const p of paths)assert.ok(read(p).split('Historical step authority follows')[0].includes('Phase 6 CLOSED / COMPLETE / FULL PASS'));
});
test('Step 5 current authority and closure status agree across tracked sources',()=>{
 const files=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-6-roadmap.md','docs/reports/phase-6-step-5.md'];
 const states=files.map(p=>{
  const s=read(p);
  const current=s.includes('## Phase 6 Step 5')?s.slice(s.indexOf('## Phase 6 Step 5')).split('## Phase 6 Step 4')[0]:s;
  const states=[...current.matchAll(/^Step 5 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)[.;]/gm)];
  assert.equal(states.length,1,p+': require one explicit current Step 5 status');const at=states[0][1];
  for(const m of ['Step 6 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(m),p+': '+m);
  if(at==='COMPLETE / FULL PASS'){
   assert.match(current,/Implementation certification SHA: `([a-f0-9]{40})`/);
   assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);
   assert.doesNotMatch(current,/not yet closed|PENDING_EXACT_SHA|certification candidate/);
  }
  return at;
 });
 assert.equal(new Set(states).size,1,'current authorities disagree');
 const rows=[...read('docs/plans/phase-6-roadmap.md').matchAll(/^\| (\d+) \| [^|]+ \| ([^|]+) \|$/gm)];
 assert.deepEqual(rows.map(m=>+m[1]),[1,2,3,4,5,6,7,8]);assert.deepEqual(rows.slice(0,4).map(m=>m[2]),Array(4).fill('COMPLETE / FULL PASS'));
 assert.equal(rows[4][2],states[0]);assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[7][2])); // Step 8 current authority is checked separately
 assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[6][2]));
 assert.ok(['AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[5][2]));
});
test('execution bridge reuses trusted validation and existing atomic history authority',()=>{
 const s=read('packages/core/src/agent_execution_boundary.hpp');
 for(const m of ['canonical_agent_schema','pixaura_agent_tools_bind','evaluation::reservation','pixaura_policy_admit','approval.canonical==','approval.state=2','pixaura_working_evaluate_cancel','document::transition','document::serialize_commit','s.accept()','agent_publication','try_to_lock'])assert.ok(s.includes(m));
 for(const m of ['system(','popen','pixaura_inference_execute','dlopen','getenv'])assert.ok(!s.includes(m));
 assert.ok(!s.slice(s.indexOf('pixaura_editor_agent_execute(')).includes('pixaura_reference_planner_plan('));
 assert.ok(read('packages/core/src/editor_api.cpp').includes('#include "agent_execution_boundary.hpp"'));
 assert.ok(read('scripts/check-model-release.mjs').includes('pixaura_test_agent_execution_checkpoint'));
 for(const target of ['agent_execution_test','agent_execution_c_consumer','agent_execution_release_surface'])assert.ok(read('CMakeLists.txt').includes(`NAME ${target}`));
});


test('Step 6 current authority cannot close with stale or missing certification evidence',()=>{
 const files=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-6-roadmap.md','docs/reports/phase-6-step-6.md'];
 const states=files.map(p=>{
  const s=read(p);const current=s.slice(s.indexOf('## Phase 6 Step 6')).split('## Phase 6 Step 5')[0];
  const state=[...current.matchAll(/^Step 6 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)[.;]/gm)];
  assert.equal(state.length,1,p);const at=state[0][1];
  for(const m of ['Step 7 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','Schema 3','DH-APPLE-METAL-01','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(m),p+': '+m);
  if(at==='COMPLETE / FULL PASS'){
   assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);
   assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);
   assert.doesNotMatch(current,/PENDING_EXACT_SHA|certification pending|IN PROGRESS/);
  }return at;
 });
 assert.equal(new Set(states).size,1);
 const rows=[...read('docs/plans/phase-6-roadmap.md').matchAll(/^\| (\d+) \| [^|]+ \| ([^|]+) \|$/gm)];
 assert.equal(rows[5][2],states[0]);assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[7][2])); // Step 8 current authority is checked separately
 assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[6][2]));
 assert.ok(read('docs/contracts/editing-candidate-lifecycle-v1.md').includes('No queue, transfer, rebinding or resurrection'));
});

test('Android debug storage and core certification flags agree without release authority',()=>{
 const s=read('platforms/android/native/CMakeLists.txt');const guarded=s.match(/if\(PIXAURA_CERTIFICATION_TESTING\)([\s\S]*?)endif\(\)/)[1];
 assert.ok(guarded.includes('target_compile_definitions(pixaura_core PRIVATE PIXAURA_CERTIFICATION_TESTING'));
 assert.ok(guarded.includes('target_compile_definitions(pixaura_storage PRIVATE PIXAURA_CERTIFICATION_TESTING)'));
 assert.equal(s.split('target_compile_definitions(pixaura_storage PRIVATE PIXAURA_CERTIFICATION_TESTING)').length,2);
 const app=read('platforms/android/app/build.gradle.kts');assert.match(app,/release[^\n]*PIXAURA_CERTIFICATION_TESTING=OFF/);
});

test('Step 7 tracked authority agrees and requires independent implementation and closure certification',()=>{
 const files=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-6-roadmap.md','docs/reports/phase-6-step-7.md'];
 const states=files.map(p=>{
  const s=read(p);const current=s.slice(s.indexOf('## Phase 6 Step 7')).split('## Phase 6 Step 6')[0];
  const match=[...current.matchAll(/^Step 7 (AUTHORIZED \/ IN PROGRESS|COMPLETE \/ FULL PASS)[.;]/gm)];
  assert.equal(match.length,1,p);
  for(const marker of ['Step 8 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','Schema 3','DH-APPLE-METAL-01','NOT WAIVED','NOT SATISFIED'])assert.ok(current.includes(marker),p+': '+marker);
  if(match[0][1]==='COMPLETE / FULL PASS'){
   assert.match(current,/Implementation certification SHA: `[a-f0-9]{40}`/);
   assert.match(current,/Foundation \d+ SUCCESS 7\/7/);assert.match(current,/Native application shells \d+ SUCCESS 3\/3/);
   assert.doesNotMatch(current,/PENDING_EXACT_SHA|IN PROGRESS|certification candidate/);
   assert.ok(current.includes('closure-SHA'));
  }
  return match[0][1];
 });
 assert.equal(new Set(states).size,1);
 const row=read('docs/plans/phase-6-roadmap.md').match(/^\| 7 \| [^|]+ \| ([^|]+) \|$/m);assert.equal(row[1],states[0]);
});

test('platform Agent adapters preserve native authority, bounded transport and quiescent ownership',()=>{
 const kotlin=read('platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/ManualEditor.kt');
 const swift=read('packages/core/swift/Sources/ManualEditor.swift');
 const jni=read('platforms/android/native/agent_bridge.cpp');
 for(const s of [kotlin,swift]){
  for(const marker of ['approveAgentPlanFromHuman','validateAgentPlan','referenceAgentPlan','executeAgentPlan','cancelAgentExecution','agentCalls','destroyed'])assert.ok(s.includes(marker));
  for(const forbidden of ['milli_ev','pixaura.exposure','plan_version','structured_intent_version','"approved"','Thread {','Task.detached'])assert.ok(!s.includes(forbidden),forbidden);
 }
 for(const marker of ['pixaura_editor_agent_plan','pixaura_editor_agent_validate','pixaura_editor_agent_approve','pixaura_editor_agent_execute','GetDirectBufferCapacity','alignof(T)','8192','catch(...)'])assert.ok(jni.includes(marker));
 assert.ok(kotlin.includes('while (active != 0 || agentCalls != 0 || assistCalls != 0)'));
 assert.ok(swift.includes('while renders != 0 || agentCalls != 0 || assistCalls != 0'));
 assert.ok(!kotlin.includes('agentTestControl'));
 assert.ok(swift.includes('#if DEBUG'));
 assert.ok(read('scripts/check-model-release.mjs').includes('pixaura_test_agent_platform_control'));
 for(const p of ['packages/core/swift/Tests/AgentPlatformTests.swift','platforms/android/app/src/androidTest/java/ai/pixaura/app/AgentPlatformTest.kt']){
  const s=read(p);for(const marker of ['executeAgentPlan','approveAgentPlanFromHuman','revisionCount','close()'])assert.ok(s.includes(marker));
 }
});
