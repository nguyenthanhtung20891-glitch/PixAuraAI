import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
test('Step 5 current authority and closure status agree across tracked sources',()=>{
 const files=['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-6-roadmap.md','docs/reports/phase-6-step-5.md'];
 const states=files.map(p=>{
  const s=read(p),at=s.indexOf('Step 5 AUTHORIZED / IN PROGRESS')>=0?'AUTHORIZED / IN PROGRESS':'COMPLETE / FULL PASS';
  const current=s.includes('## Phase 6 Step 5')?s.slice(s.indexOf('## Phase 6 Step 5')).split('## Phase 6 Step 4')[0]:s;
  assert.ok(current.includes(`Step 5 ${at}`),p);
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
 assert.equal(rows[4][2],states[0]);assert.deepEqual(rows.slice(5).map(m=>m[2]),Array(3).fill('NOT STARTED'));
});
test('execution bridge reuses trusted validation and existing atomic history authority',()=>{
 const s=read('packages/core/src/agent_execution_boundary.hpp');
 for(const m of ['canonical_agent_schema','pixaura_agent_tools_bind','evaluation::reservation','pixaura_policy_admit','approval.canonical==','approval.state=2','pixaura_working_evaluate_cancel','document::transition','document::serialize_commit','s.accept()','agent_publication','try_to_lock'])assert.ok(s.includes(m));
 for(const m of ['system(','popen','pixaura_inference_execute','reference_planner_plan','dlopen','getenv'])assert.ok(!s.includes(m));
 assert.ok(read('packages/core/src/editor_api.cpp').includes('#include "agent_execution_boundary.hpp"'));
 assert.ok(read('scripts/check-model-release.mjs').includes('pixaura_test_agent_execution_checkpoint'));
 for(const target of ['agent_execution_test','agent_execution_c_consumer','agent_execution_release_surface'])assert.ok(read('CMakeLists.txt').includes(`NAME ${target}`));
});
