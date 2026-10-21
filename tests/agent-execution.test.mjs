import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');

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
 assert.ok(kotlin.includes('while (active != 0 || agentCalls != 0)'));
 assert.ok(swift.includes('while renders != 0 || agentCalls != 0'));
 assert.ok(!kotlin.includes('agentTestControl'));
 assert.ok(swift.includes('#if DEBUG'));
 assert.ok(read('scripts/check-model-release.mjs').includes('pixaura_test_agent_platform_control'));
 for(const p of ['packages/core/swift/Tests/AgentPlatformTests.swift','platforms/android/app/src/androidTest/java/ai/pixaura/app/AgentPlatformTest.kt']){
  const s=read(p);for(const marker of ['executeAgentPlan','approveAgentPlanFromHuman','revisionCount','close()'])assert.ok(s.includes(marker));
 }
});
