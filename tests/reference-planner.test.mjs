import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';import crypto from 'node:crypto';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
const f=JSON.parse(read('tests/fixtures/reference-planner-v1.json'));
const sort=v=>Array.isArray(v)?v.map(sort):v&&typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,sort(v[k])])):v;
const canon=v=>JSON.stringify(sort(v))+'\n';const hash=s=>crypto.createHash('sha256').update(s).digest('hex');
test('Step 4 closure authority records observed implementation certification consistently',()=>{
 for(const path of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','ROADMAP.md','QUALITY_GATES.md','docs/plans/phase-6-roadmap.md','docs/reports/phase-6-step-4.md']){
  const current=read(path).split('## Phase 6 Step 3')[0];
  for(const expected of ['Step 4 COMPLETE / FULL PASS','7c00b9193c649fb2f7addf2dc6cd6a558a84dee5','39136505109 SUCCESS 7/7','39136505094 SUCCESS 3/3','closure-only','Phase 7 NOT STARTED'])assert.ok(current.includes(expected),path+': '+expected);
  assert.ok(current.includes(path==='docs/plans/phase-6-roadmap.md'&&current.includes('## Phase 6 Step 5')?'Step 6 NOT STARTED':'Step 5 NOT STARTED'));
  assert.doesNotMatch(current,/Step 4 AUTHORIZED \/ IN PROGRESS|PENDING_EXACT_SHA|certification candidate|exact-SHA certification pending/);
 }
 const rows=[...read('docs/plans/phase-6-roadmap.md').matchAll(/^\| (\d+) \| [^|]+ \| ([^|]+) \|$/gm)];
 assert.deepEqual(rows.slice(0,4).map(m=>m[2]),Array(4).fill('COMPLETE / FULL PASS'));
 assert.deepEqual(rows.slice(5).map(m=>m[2]),Array(3).fill('NOT STARTED'));
 assert.ok(['NOT STARTED','AUTHORIZED / IN PROGRESS','COMPLETE / FULL PASS'].includes(rows[4][2])); // later Step 5 authorization preserves Step 4 certification
});
test('independent structured intent to plan goldens and exact embedded bytes',()=>{
 assert.equal(f.cases.length,39);assert.equal(f.negative.length,42);assert.equal(f.mappings.length,11);
 const tools=JSON.parse(read('tests/fixtures/agent-tools-v1.json')).tools;
 for(const c of f.cases){
  const i=JSON.parse(c.input),p=JSON.parse(c.expected_plan);assert.equal(canon(i),c.input);assert.equal(canon(p),c.expected_plan);assert.equal(hash(c.input),c.intent_sha256);assert.equal(hash(c.expected_plan),c.plan_sha256);
  assert.deepEqual(Object.keys(i),['binding','goal_id','parameters','structured_intent_version']);assert.equal(i.structured_intent_version,1);
  const tool=f.mappings.find(m=>m[0]===i.goal_id)[1],d=tools.find(t=>t.tool_id===tool);assert.equal(d.operation_version,1);assert.equal(d.parameter_version,1);
  assert.deepEqual(p,{actions:[{action_index:0,operation_version:1,parameter_version:1,parameters:i.parameters,tool_id:tool}],binding:i.binding,plan_version:1});
  assert.deepEqual(Object.keys(i.parameters),d.parameters.map(p=>p.name));
  for(const spec of d.parameters){const v=i.parameters[spec.name];assert.ok(Number.isSafeInteger(v)&&v>=spec.minimum&&v<=spec.maximum);}
  if(tool==='pixaura.crop'){assert.ok(i.parameters.x_ppm+i.parameters.width_ppm<=1000000);assert.ok(i.parameters.y_ppm+i.parameters.height_ppm<=1000000);}
  assert.ok(Buffer.byteLength(c.input)<=2048&&Buffer.byteLength(c.expected_plan)<=8192);
  assert.deepEqual(JSON.parse(c.test_stage).operations[0].parameters,i.parameters);
 }
 const header='#ifndef PIXAURA_REFERENCE_PLANNER_FIXTURES_HPP\n#define PIXAURA_REFERENCE_PLANNER_FIXTURES_HPP\nnamespace pixaura::reference_planner_fixtures {\nstruct Fixture { const char* name; const char* input; const char* plan; const char* stage; };\ninline constexpr Fixture cases[] = {\n'+f.cases.map(c=>'{'+[c.name,c.input,c.expected_plan,c.test_stage].map(JSON.stringify).join(',')+'}').join(',\n')+'\n};\nstruct Negative {const char* name;const char* input;int status;};\ninline constexpr Negative negative[] = {\n'+f.negative.map(c=>'{'+[JSON.stringify(c.name),JSON.stringify(c.input),c.status].join(',')+'}').join(',\n')+'\n};\n}\n#endif\n';assert.equal(read('packages/core/tests/reference_planner_fixtures.hpp'),header);
 const mappings=[...read('packages/core/src/structured_goals.hpp').matchAll(/\{"([^"]+)","([^"]+)"\}/g)].map(m=>[m[1],m[2]]);assert.deepEqual(mappings,f.mappings);
});
test('free-form intent, plan and registry v1 remain byte-compatible and unmodified',()=>{
 for(const [p,digest] of Object.entries(f.retained_sha256))assert.equal(hash(read(p)),digest,p);
 const rejected=f.negative.find(c=>c.name==='free-form-non-mapping');assert.equal(JSON.parse(rejected.input).goal,'make this photo brighter');assert.equal(rejected.status,5);
 const policy=read('packages/core/src/structured_goals.hpp');for(const text of ['minimum','maximum','milli_ev','milli_linear','kelvin'])assert.ok(!policy.includes(text));
});
test('Step 4 authority preserves separate contracts and later-step stop gates',()=>{
 for(const path of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','ROADMAP.md','QUALITY_GATES.md','docs/reports/phase-6-step-4.md']){
  const text=read(path).split('## Phase 6 Step 3')[0];for(const m of ['Step 4','Step 5 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01'])assert.ok(text.includes(m),path+': '+m);
 }
 assert.ok(read('DECISIONS.md').includes('Phase 6 Step 4 may introduce a separately versioned structured-intent contract\nwithout modifying free-form intent v1.'));
 assert.deepEqual([...read('docs/plans/phase-6-roadmap.md').matchAll(/^\| (\d+) \|/gm)].map(m=>+m[1]),[1,2,3,4,5,6,7,8]);
 const boundary=read('packages/core/src/reference_planner_boundary.hpp');for(const m of ['SchemaLease','schema_epoch','agent_bind_tools','PIXAURA_DOCUMENT_CANCELLED','PIXAURA_CERTIFICATION_TESTING'])assert.ok(boundary.includes(m));
 for(const forbidden of ['transition(','document_apply','pixaura_inference_execute','system(','popen','Repository','random','chrono'])assert.ok(!boundary.includes(forbidden));
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(read(p).includes('tests/reference-planner.test.mjs'));
});
