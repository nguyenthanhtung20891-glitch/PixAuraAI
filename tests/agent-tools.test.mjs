import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
const golden=JSON.parse(read('tests/fixtures/agent-tools-v1.json'));
test('complete agent table is exact manual-derived metadata with fixed exposure',()=>{
 const native=read('packages/core/src/manual_tools.cpp');
 const parameters=new Map([...native.matchAll(/constexpr Parameter (\w+)\[\]\s*=\s*\{([\s\S]*?)\};/g)].map(m=>[m[1],[...m[2].matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(\d+)\}/g)].map(v=>({default:+v[5],maximum:+v[4],minimum:+v[3],name:v[1],step:+v[6],type:'integer',unit:v[2]}))]));
 const tools=[...native.matchAll(/\{"(pixaura\.[^"]+)",\s*"[^"]+",\s*"([^"]+)",\s*"([^"]+)",\s*1,\s*1,\s*(\w+),\s*\d+\}/g)].map(m=>({agent_exposable:true,capability:'offline_cpu',category:m[2],commit:'one_immutable_revision_per_changed_gesture',invalid:'reject',name:m[3],operation_version:1,parameter_version:1,parameters:parameters.get(m[4]),preview:'detached_latest_generation',replay:'ordered_phase2_versions',serialization:'schema1_canonical_integer_json',tool_id:m[1],type:m[1]}));
 assert.equal(tools.length,11);assert.deepEqual(golden,{registry_version:1,tools});
 assert.deepEqual(tools.map(t=>t.tool_id),tools.map(t=>t.tool_id).sort());
 const header='#ifndef PIXAURA_AGENT_TOOLS_FIXTURE_HPP\n#define PIXAURA_AGENT_TOOLS_FIXTURE_HPP\ninline constexpr const char agent_tools_golden[] = R"PX('+JSON.stringify(golden)+'\n)PX";\n#endif\n';
 assert.equal(read('packages/core/tests/agent_tools_fixture.hpp'),header);
 assert.ok(native.includes('String agent_registry_json() { return registry_json(true); }'));
 assert.ok(native.includes('if (agent && !agent_exposable(d)) continue;'));
});
test('all Step 2 accepted plans map exactly to registry schema without changing bytes',()=>{
 const fixtures=JSON.parse(read('tests/fixtures/agent-schema-v1/goldens.json'));
 for(const f of fixtures.cases.filter(f=>f.kind===2)){
  const p=JSON.parse(f.canonical);assert.equal(p.actions.length>8,f.status===8);
  for(const [i,a] of p.actions.entries()){
   const d=golden.tools.find(t=>t.tool_id===a.tool_id);assert.ok(d);assert.equal(a.action_index,i);
   assert.equal(a.operation_version,d.operation_version);assert.equal(a.parameter_version,d.parameter_version);
   assert.deepEqual(Object.keys(a.parameters).sort(),d.parameters.map(p=>p.name));
   for(const s of d.parameters){const v=a.parameters[s.name];assert.ok(Number.isSafeInteger(v)&&v>=s.minimum&&v<=s.maximum);}
  }
 }
});
test('Step 3 authority retains eight steps and excludes planning approval execution',()=>{
 for(const f of ['AGENTS.md','ARCHITECTURE.md','ROADMAP.md','QUALITY_GATES.md','DECISIONS.md','docs/reports/phase-6-step-3.md']){
  const s=read(f).split('## Phase 6 Step 2')[0];for(const text of ['Step 3','Step 4 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01'])assert.ok(s.includes(text),f+': '+text);
 }
 const roadmap=read('docs/plans/phase-6-roadmap.md');assert.deepEqual([...roadmap.matchAll(/^\| (\d+) \|/gm)].map(m=>+m[1]),[1,2,3,4,5,6,7,8]);
 const abi=read('packages/core/include/pixaura/agent_tools.h');for(const marker of ['9216u','32768u','registry_version','pixaura_agent_tools_bind'])assert.ok(abi.includes(marker));
 const boundary=read('packages/core/src/agent_schema_boundary.hpp');for(const forbidden of ['transition(','document_apply','dlopen','popen','system(','repository'])assert.ok(!boundary.includes(forbidden));
 assert.ok(boundary.includes('agent_exposable(*descriptor)'));assert.ok(boundary.includes('not_approved'));
 for(const f of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(read(f).includes('tests/agent-tools.test.mjs'));
});
