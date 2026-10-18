import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const read=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
const fixtures=JSON.parse(read('tests/fixtures/agent-schema-v1/goldens.json'));
const sort=v=>Array.isArray(v)?v.map(sort):v&&typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,sort(v[k])])):v;
const cpp=s=>JSON.stringify(s).replace(/[^\x00-\x7f]/gu,c=>[...Buffer.from(c)].map(b=>'\\x'+b.toString(16).padStart(2,'0')+'""').join(''));
test('independent canonical goldens/digests and native embedded fixtures agree completely',()=>{
    assert.equal(fixtures.cases.length,7);
    for(const f of fixtures.cases){
        assert.equal(JSON.stringify(sort(JSON.parse(f.input)))+'\n',f.canonical);
        assert.equal(crypto.createHash('sha256').update(f.canonical).digest('hex'),f.sha256);
        assert.ok(Buffer.byteLength(f.input)<=8192 && Buffer.byteLength(f.canonical)<=8192);
        const value=JSON.parse(f.canonical);assert.deepEqual(value.binding,fixtures.binding);
        if(f.kind===2){assert.deepEqual(value.actions.map(a=>a.action_index),value.actions.map((_,i)=>i));assert.equal(value.actions.length>8,f.status===8);}
    }
    let header='#ifndef PIXAURA_AGENT_SCHEMA_FIXTURES_HPP\n#define PIXAURA_AGENT_SCHEMA_FIXTURES_HPP\nnamespace pixaura::agent_schema_fixtures {\nstruct Fixture { const char* name; unsigned kind; const char* input; const char* canonical; int status; };\n';
    header+='inline constexpr const char document[] = R"PX('+read('tests/fixtures/image-document-v1.json')+')PX";\n';
    header+='inline constexpr Fixture cases[] = {\n'+fixtures.cases.map(c=>'{"'+c.name+'",'+c.kind+','+cpp(c.input)+','+cpp(c.canonical)+','+c.status+'}').join(',\n')+'\n};\n}\n#endif\n';
    assert.equal(read('packages/core/tests/agent_schema_fixtures.hpp'),header);
    const eight=JSON.parse(fixtures.cases.find(c=>c.name==='maximum-eight').canonical);
    assert.equal(eight.actions.filter(a=>a.tool_id==='pixaura.exposure').length,2);
});
test('current Step 2 authority retains eight steps and closes execution/approval',()=>{
    for(const file of ['AGENTS.md','ROADMAP.md','QUALITY_GATES.md','ARCHITECTURE.md','DECISIONS.md','docs/reports/phase-6-step-2.md']){
        const all=read(file),text=(all.includes('## Phase 6 Step 2')?all.slice(all.indexOf('## Phase 6 Step 2')):all).split('## Phase 6 Step 1')[0];
        for(const marker of ['Step 2','Step 3 NOT STARTED','Phase 7 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01'])assert.ok(text.includes(marker),`${file}: ${marker}`);
    }
    const plan=read('docs/plans/phase-6-roadmap.md');
    assert.deepEqual([...plan.matchAll(/^\| (\d+) \|/gm)].map(m=>+m[1]),[1,2,3,4,5,6,7,8]);
    const source=read('packages/core/src/agent_schema_boundary.hpp');
    for(const forbidden of ['document_apply','transition(','repository','system(','popen','pixaura_inference_execute'])assert.ok(!source.includes(forbidden));
    for(const marker of ['SchemaLease','schema_epoch','PIXAURA_DOCUMENT_CANCELLED','PIXAURA_CERTIFICATION_TESTING'])assert.ok(source.includes(marker));
    assert.ok(read('CMakeLists.txt').includes('NAME agent_schema_release_surface'));
    for(const file of ['platforms/android/native/capability_bridge.cpp','platforms/android/bridge/src/main/kotlin/ai/pixaura/bridge/CoreProbe.kt'])assert.ok(read(file).includes('nativeAgentSchemaBoundary'));
});
