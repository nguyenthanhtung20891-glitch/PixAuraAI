// Engineering evidence only. Never linked into inference or document persistence.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';

export const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const read=p=>fs.readFileSync(path.join(root,p),'utf8').replaceAll('\r\n','\n');
export function auditClosure(){
    const plan=JSON.parse(read('docs/reports/phase-5-closure-v1.json'));
    assert.equal(plan.version,1);assert.equal(plan.phase,5);assert.equal(plan.runtime_authority,false);assert.equal(plan.record_kind,'CLOSURE_CANDIDATE');
    assert.equal(plan.status,'PENDING_EXACT_SHA');assert.equal(plan.final_sha,'FINAL_WORKFLOW_HEAD_SHA');
    assert.equal(plan.phase6,'NOT STARTED');assert.equal(plan.production_model,'UNRESOLVED');
    assert.deepEqual(plan.steps.map(s=>s.step),[1,2,3,4,5,6,7,8]);
    for(const step of plan.steps.slice(0,7)){
        assert.equal(step.status,'COMPLETE / FULL PASS');assert.match(step.final_sha,/^[a-f0-9]{40}$/);
        assert.match(step.foundation_run,/^\d+$/);assert.match(step.shells_run,/^\d+$/);
    }
    assert.equal(plan.steps[7].status,'PENDING_EXACT_SHA');assert.equal(plan.steps[7].final_sha,'FINAL_WORKFLOW_HEAD_SHA');
    assert.equal(plan.steps[7].foundation_run,null);assert.equal(plan.steps[7].shells_run,null);
    assert.deepEqual(plan.providers.map(p=>[p.id,p.release_trust,p.release_trust_value]),[
        ['cpu.reference.v1','VALIDATED',1],['android.vulkan.v1','UNKNOWN',0],['apple.metal.v1','UNKNOWN',0]]);
    for(const provider of plan.providers){assert.equal(provider.accepted_software_suite,'PASS');assert.equal(provider.accepted_sha,plan.baseline);assert.equal(provider.closure_software_suite,'PENDING_EXACT_SHA');}
    assert.equal(plan.deferred_gate.status,'DEFERRED');assert.equal(plan.deferred_gate.waived,false);assert.equal(plan.deferred_gate.satisfied,false);
    assert.equal(plan.contracts.length,8);
    for(const contract of plan.contracts){assert.equal(contract.version,1);assert.equal(crypto.createHash('sha256').update(read(contract.path)).digest('hex'),contract.sha256_lf,`${contract.path}: frozen contract drift`);}
    for(const file of ['AGENTS.md','ROADMAP.md','QUALITY_GATES.md','ARCHITECTURE.md','DECISIONS.md','docs/plans/phase-5-roadmap.md','docs/reports/phase-5-closure.md','docs/reports/phase-5-step-8.md']){
        const text=read(file),current=file==='AGENTS.md'?text.split('## 6. Current authorized scope')[1].split('## 7.')[0]:text.split('<!-- historical-authority -->')[0];
        for(const marker of ['PENDING_EXACT_SHA','Phase 6 NOT STARTED','Production model UNRESOLVED','DH-APPLE-METAL-01','DEFERRED'])assert.ok(current.includes(marker),`${file}: missing ${marker}`);
        assert.ok(!/^Phase 5 (?:— )?CLOSED \/ COMPLETE \/ FULL PASS\.?$/m.test(current),`${file}: premature unconditional closure`);
    }
    const b=plan.resource_bounds,source=read('packages/core/src/inference.cpp');
    assert.equal(Number(source.match(/control=(\d+);/)[1]),b.context_control_bytes);
    const descriptor=source.match(/d.max_handles=(\d+);d.max_dimension=(\d+);d.max_context_bytes=(\d+)/);
    assert.deepEqual(descriptor.slice(1).map(Number),[b.max_handles,b.max_dimension,b.context_max_bytes]);
    assert.equal(Number(source.match(/prepared_lease=std::make_shared<Lease>\(\*h.state,(\d+)\)/)[1]),b.reference_prepared_control_bytes);
    assert.match(source,/d\.validation=1/);assert.equal((source.match(/d\.validation=0/g)||[]).length,2);
    assert.equal(Number(read('packages/core/src/inference_admission.hpp').match(/admission_metadata=(\d+)/)[1]),b.admission_control_bytes);
    for(const [file,value] of [['platforms/android/native/inference_vulkan.hpp',b.vulkan_transient_bytes],['packages/core/src/inference_metal.hpp',b.metal_transient_bytes]])
        assert.equal(Number(read(file).match(/transient_bytes\(\)const override\{return (\d+);/)[1]),value);
    assert.equal(b.phase4_safety_room_bytes+b.context_control_bytes,b.phase4_total_control_safety_bytes);
    assert.equal(b.reference_input_max_bytes,3*b.reference_max_pixels*4);assert.equal(b.reference_output_max_bytes,b.reference_max_pixels*4);
    assert.equal(b.tensor_max_elements*4,b.tensor_max_bytes);assert.equal(b.package_payload_total_max_bytes+b.manifest_max_bytes+b.signature_bytes,b.package_data_max_bytes);
    assert.match(read('packages/core/src/model_package.cpp'),/\{1,1,1,nullptr,0\}/);
    assert.equal(plan.admission.recovery_observations,3);assert.equal(plan.admission.post_start_cpu_replay,false);
    assert.equal(plan.admission.automatic_release_selection,'cpu.reference.v1');
    assert.deepEqual(plan.u2netp,['AUDITED','UNSUPPORTED BY CURRENT PROFILE','UNAPPROVED','UNINCORPORATED','CHECKPOINT RIGHTS UNRESOLVED']);
    return plan;
}

// A reporting gate, not authentication of arbitrary JSON or a runtime trust store.
// Actual snapshots must be observed through scripts/codex-gh.ps1.
export function certifyClosure(plan,head,foundation,shells){
    assert.equal(plan.status,'PENDING_EXACT_SHA');assert.equal(plan.runtime_authority,false);
    assert.match(head,/^[a-f0-9]{40}$/);assert.notEqual(head,plan.baseline,'old baseline cannot certify Step 8');
    const workflows=[];
    for(const [kind,state] of [['foundation',foundation],['shells',shells]]){
        const expected=plan.required_workflows[kind];
        assert.equal(state.headSha,head,'workflow SHA mismatch');assert.equal(state.status,'completed');assert.equal(state.conclusion,'success');
        assert.deepEqual(state.jobs.map(j=>j.name).sort(),[...expected.jobs].sort(),'missing, duplicate or unexpected required job');
        for(const job of state.jobs){assert.equal(job.status,'completed');assert.equal(job.conclusion,'success','required job did not succeed');}
        const match=state.url.match(/^https:\/\/github\.com\/nguyenthanhtung20891-glitch\/PixAuraAI\/actions\/runs\/(\d+)$/);
        assert.ok(match,'workflow must belong to the approved repository');
        assert.ok(!plan.steps.slice(0,7).some(s=>s.foundation_run===match[1]||s.shells_run===match[1]),'historical run cannot certify closure');
        workflows.push({name:expected.name,id:match[1],head_sha:head,result:'SUCCESS',jobs:state.jobs.map(j=>({name:j.name,result:'SUCCESS'}))});
    }
    assert.notEqual(workflows[0].id,workflows[1].id);
    const receipt=structuredClone(plan);
    receipt.record_kind='OBSERVED_CLOSURE_RECEIPT';receipt.status=plan.certified_status;receipt.final_sha=head;receipt.workflows=workflows;
    receipt.steps[7]={...receipt.steps[7],status:'COMPLETE / FULL PASS',final_sha:head,foundation_run:workflows[0].id,shells_run:workflows[1].id,evidence:'observed both complete same-SHA workflows SUCCESS'};
    for(const provider of receipt.providers){provider.closure_software_suite='PASS';}
    return receipt;
}

export function readSnapshot(file){
    assert.ok(fs.statSync(file).size<=1048576,'oversized workflow snapshot');
    const bytes=fs.readFileSync(file);
    const text=bytes.toString(bytes[0]===255&&bytes[1]===254?'utf16le':'utf8').replace(/^\uFEFF/,'');
    // Wrapper writes bounded human text then a single API JSON record.
    const records=text.split(/\r?\n/).filter(line=>line.startsWith('{'));
    assert.equal(records.length,1,'require exactly one workflow API snapshot');
    return JSON.parse(records[0]);
}

if(process.argv[1]&&path.resolve(process.argv[1])===fileURLToPath(import.meta.url)){
    const plan=auditClosure();
    if(process.argv[2]==='--check')console.log('Phase 5 closure consistency PASS; effective status PENDING_EXACT_SHA until observed receipt');
    else{
        assert.equal(process.argv[2],'--certify');assert.equal(process.argv.length,6);
        const head=execFileSync('git',['rev-parse','HEAD'],{cwd:root,encoding:'utf8'}).trim();
        assert.equal(execFileSync('git',['status','--porcelain','--untracked-files=no'],{cwd:root,encoding:'utf8'}).trim(),'','tracked worktree must be clean');
        const output=path.resolve(root,process.argv[5]),build=fs.realpathSync(path.join(root,'build'));
        const parent=fs.realpathSync(path.dirname(output));assert.equal(parent,build,'receipt output must be directly under repository build directory');
        assert.ok(!fs.existsSync(output)||(!fs.lstatSync(output).isSymbolicLink()&&fs.lstatSync(output).isFile()),'unsafe receipt output');
        const receipt=certifyClosure(plan,head,readSnapshot(process.argv[3]),readSnapshot(process.argv[4]));
        fs.writeFileSync(output,JSON.stringify(receipt,null,2)+'\n');
        console.log(`Phase 5 ${receipt.status}; SHA=${head}; receipt=${path.basename(output)}; Phase 6 NOT STARTED`);
    }
}
