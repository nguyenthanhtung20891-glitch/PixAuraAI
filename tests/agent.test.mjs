import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read=p=>fs.readFileSync(p,'utf8');
test('agent boundary is staging only and retains existing semantic owners',()=>{
    const native=read('packages/core/src/agent_boundary.hpp');
    for(const required of ['parse_evaluation','serialize_evaluation','manual::find','snapshot_locked','agent_current','agent_ranges','PIXAURA_AGENT_NOT_APPROVED'])assert.ok(native.includes(required));
    for(const forbidden of ['document_apply','transition(','repository','system(','popen','shader','http','getenv','pixaura_inference_execute'])assert.ok(!native.includes(forbidden));
    const header=read('packages/core/include/pixaura/agent.h');
    assert.match(header,/PIXAURA_AGENT_MAX_ACTIONS 8u/);assert.match(header,/PIXAURA_AGENT_MAX_BYTES 8192u/);
    assert.match(header,/PIXAURA_AGENT_MAX_PROPOSALS 8u/);assert.ok(!/pixaura_agent_(commit|approve|execute)\(/.test(header));
    for(const symbol of ['agent_test','agent_c_consumer'])assert.ok(read('CMakeLists.txt').includes(`NAME ${symbol}`));
});
test('finite Phase 6 authority preserves approval separation and later stop gates',()=>{
    const roadmap=read('docs/plans/phase-6-roadmap.md');
    assert.deepEqual([...roadmap.matchAll(/^\| (\d+) \|/gm)].map(m=>+m[1]),[1,2,3,4,5,6,7,8]);
    for(const file of ['AGENTS.md','ROADMAP.md','QUALITY_GATES.md','ARCHITECTURE.md','DECISIONS.md','docs/reports/phase-6-step-1.md']){
        const current=read(file).split('## Phase 5 Step 8')[0];
        for(const marker of ['Phase 6 Step 1','Step 2 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01'])assert.ok(current.includes(marker),`${file}: ${marker}`);
    }
    const contract=read('docs/contracts/editing-agent-core-v1.md');
    for(const marker of ['INTENT','PLAN','EXECUTION','UNTRUSTED INPUT','NOT_APPROVED','Schema 3','Step 2 NOT STARTED'])assert.ok(contract.includes(marker));
});
