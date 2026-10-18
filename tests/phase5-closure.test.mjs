import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import {auditClosure,certifyClosure,readSnapshot,root} from '../scripts/phase5-closure.mjs';

test('finite Phase 5 closure preserves accepted history, frozen contracts, resources and scope',()=>{
    const p=auditClosure();
    assert.deepEqual(p.steps.slice(0,7).map(s=>s.final_sha),[
        '8d5a30338c97a05e987ecf1324b86a2afb795356','58202caf1fd86215abe4650b208b2134cdca9b58',
        'bf31eb33d5b5027236d71cf56da4c648d6b56255','a3ad7e7a61030992d933b3ca68572f83604dd5a9',
        '59d846967fffef7e735597f6b65301841659336f','5592cd839c2607379c6cb8c02a4b42a44e80311d',
        'b46b30c26af0790d0715bc48d24093b83290b8fe']);
    const roadmap=fs.readFileSync(path.join(root,'docs/plans/phase-5-roadmap.md'),'utf8');
    assert.deepEqual([...roadmap.matchAll(/^\| (\d+) \|/gm)].map(m=>Number(m[1])),[1,2,3,4,5,6,7,8]);
    for(const step of p.steps){assert.ok(roadmap.includes(step.name));assert.ok(roadmap.includes(step.final_sha));}
    for(const flag of ['weights','external_runtime','ai_ux','editing_agent'])assert.equal(p[flag],'NONE');
    assert.equal(p.schema3,'UNCHANGED');
});

// SYNTHETIC workflow records test reporting rejection only, never actual evidence.
const head='e'.repeat(40);
function evidence(plan,kind,id){return {headSha:head,status:'completed',conclusion:'success',
    url:`https://github.com/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/${id}`,
    jobs:plan.required_workflows[kind].jobs.map(name=>({name,status:'completed',conclusion:'success'}))};}

test('only two complete same-final-SHA workflows resolve pending closure and Step 8 identity',()=>{
    const p=auditClosure(),f=evidence(p,'foundation','999000001'),s=evidence(p,'shells','999000002');
    const receipt=certifyClosure(p,head,f,s);
    assert.equal(receipt.record_kind,'OBSERVED_CLOSURE_RECEIPT');assert.equal(p.record_kind,'CLOSURE_CANDIDATE');
    assert.equal(receipt.status,'CLOSED / COMPLETE / FULL PASS');assert.equal(receipt.final_sha,head);
    assert.ok(receipt.steps.every(step=>step.status==='COMPLETE / FULL PASS'));
    assert.equal(receipt.steps[7].final_sha,head);assert.equal(receipt.steps[7].foundation_run,'999000001');
    assert.deepEqual(receipt.providers.map(provider=>provider.release_trust),['VALIDATED','UNKNOWN','UNKNOWN']);
    assert.ok(receipt.providers.every(provider=>provider.closure_software_suite==='PASS'));
    assert.equal(receipt.phase6,'NOT STARTED');assert.equal(receipt.production_model,'UNRESOLVED');
    assert.equal(receipt.deferred_gate.status,'DEFERRED');assert.equal(receipt.deferred_gate.satisfied,false);
    assert.equal(p.status,'PENDING_EXACT_SHA');assert.equal(p.steps[7].status,'PENDING_EXACT_SHA');
});

test('closure rejects old SHA, pending/failed/skipped/missing/duplicate jobs and foreign evidence',()=>{
    const p=auditClosure(),f=evidence(p,'foundation','999000001'),s=evidence(p,'shells','999000002');
    const reject=edit=>{const bad=structuredClone(f);edit(bad);assert.throws(()=>certifyClosure(p,head,bad,s));};
    reject(x=>{x.headSha=p.baseline;});reject(x=>{x.status='in_progress';});reject(x=>{x.conclusion='failure';});
    reject(x=>{x.jobs.pop();});reject(x=>{x.jobs.push(x.jobs[0]);});
    reject(x=>{x.jobs[0].status='in_progress';});reject(x=>{x.jobs[0].conclusion='skipped';});
    reject(x=>{x.jobs[0].conclusion='failure';});reject(x=>{x.jobs[0].name='unexpected';});
    reject(x=>{x.url=x.url.replace('PixAuraAI','unrelated');});
    reject(x=>{x.url=x.url.replace('999000001',p.steps[6].foundation_run);});
    assert.throws(()=>certifyClosure(p,p.baseline,{...f,headSha:p.baseline},{...s,headSha:p.baseline}));
    assert.throws(()=>certifyClosure(p,head,s,f));
});

test('bounded wrapper snapshots accept UTF8/Windows UTF16 and reject ambiguous or oversized evidence',()=>{
    fs.mkdirSync(path.join(root,'build'),{recursive:true});
    const directory=fs.mkdtempSync(path.join(root,'build','phase5-closure-test-'));
    try{
        const value=evidence(auditClosure(),'foundation','999000001'),file=path.join(directory,'snapshot.log');
        fs.writeFileSync(file,'human wrapper text\n'+JSON.stringify(value)+'\n');assert.deepEqual(readSnapshot(file),value);
        fs.writeFileSync(file,'\uFEFFwrapper\r\n'+JSON.stringify(value)+'\r\n','utf16le');assert.deepEqual(readSnapshot(file),value);
        fs.writeFileSync(file,JSON.stringify(value)+'\n'+JSON.stringify(value));assert.throws(()=>readSnapshot(file));
        fs.writeFileSync(file,'x'.repeat(1048577));assert.throws(()=>readSnapshot(file));
    }finally{fs.rmSync(directory,{recursive:true,force:true});}
});
