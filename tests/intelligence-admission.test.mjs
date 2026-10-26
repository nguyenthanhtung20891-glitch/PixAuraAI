import test from 'node:test';
import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import {canonical,validateProvenance,syntheticFixtures,deterministicBaseline,compare,evaluate} from '../scripts/intelligence-admission.mjs';
import fs from 'node:fs';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
const digest=b=>crypto.createHash('sha256').update(b).digest('hex');
const artifact=Buffer.from('owned fixture, not model weights');
const entry={name:'owned.fixture',bytes:artifact.length,sha256:digest(artifact)};
function manifest(){return {version:1,upstream:{project:'owned.synthetic',revision:'fixture-1',artifacts:[entry]},license:{identifier:'project-owned',text_sha256:digest(Buffer.from('license')),redistribution_evidence:'fixture test authority only'},processor:{tokenizer:'none',preprocessing:'rgb-fixture-v1'},architecture:'synthetic-not-a-model',conversion:{tool:'fixture-builder',revision:'1',argv:['fixture-builder','--owned'],quantization:'none',optimization:'none',environment:'fixture-v1',intermediates:[]},artifacts:[entry],runtime:'none',custody:'test-only original bytes',reproduction:'exact generated fixture'};}
test('complete provenance is canonical, integrity checked, and never production approval',()=>{
  const m=manifest(),files=new Map([[entry.name,artifact]]);
  assert.equal(validateProvenance(canonical(m),files).productionAdmitted,false);
  assert.equal(canonical(JSON.parse(canonical(m))),canonical(m));
  for(const mutate of [m=>delete m.conversion.revision,m=>delete m.conversion.argv,m=>delete m.upstream.artifacts[0].sha256,m=>delete m.license.identifier,m=>delete m.license.redistribution_evidence,m=>m.version=2,m=>m.artifacts[0].name='../model',m=>m.custody='bad\u0000text',m=>m.artifacts[0].sha256='0'.repeat(64)]){
    const bad=JSON.parse(canonical(m));mutate(bad);assert.throws(()=>validateProvenance(canonical(bad),files));
  }
  assert.throws(()=>validateProvenance(canonical(m),new Map([...files,['unexpected',artifact]])));
  assert.throws(()=>validateProvenance(canonical(m).replace('"version":1','"version":1,"version":1'),files));
  assert.throws(()=>validateProvenance(' '.repeat(65537),files));
});
test('candidate-neutral paired measurement protects expectations and reports abstention/failure',()=>{
  const fixtures=syntheticFixtures(),before=canonical(fixtures);
  const report=compare(fixtures,deterministicBaseline,deterministicBaseline);
  assert.equal(report.reference.semanticCorrect,8);assert.equal(report.candidate.abstentionCorrect,2);
  assert.equal(report.candidate.repeatConsistent,10);assert.equal(report.candidate.admitted,false);
  assert.equal(report.candidate.thresholds,'PENDING_PRINCIPAL_ARCHITECT');
  const invalid=evaluate(fixtures,{propose(input){assert.equal(input.expected,undefined);input.image.fill(255);return {actions:[{tool:'transform.crop_normalized',value:1}],memoryEvidence:null};}});
  assert.equal(invalid.invalid,20);assert.equal(canonical(fixtures),before);
  const wrong=evaluate(fixtures,{propose(){return {actions:[{tool:'adjust.exposure',value:-100}],memoryEvidence:null};}});
  assert.equal(wrong.neutralFalsePositives,2);assert.ok(wrong.semanticCorrect<8);assert.equal(wrong.directionCorrect,0);
  const conflict=evaluate(fixtures,{propose(){return {actions:[{tool:'adjust.exposure',value:100},{tool:'adjust.brightness',value:10}],memoryEvidence:12};}});
  assert.equal(conflict.conflicts,10);
  let n=0;const unstable=evaluate(fixtures,{propose(){return {actions:n++%2?[{tool:'adjust.exposure',value:100}]:[],memoryEvidence:null};}});
  assert.equal(unstable.repeatConsistent,0);
  for(const value of [NaN,Infinity,-Infinity,5001]){
    const malformed=evaluate(fixtures,{propose(){return {actions:[{tool:'adjust.exposure',value}],memoryEvidence:null};}});
    assert.equal(malformed.invalid,20);
  }
  assert.throws(()=>evaluate([{...fixtures[0],image:new Uint8Array(1048577)}],deterministicBaseline));
});
test('offline recorded-output CLI labels transport timing honestly and never loads a runtime',()=>{
  const root=path.resolve('build');fs.mkdirSync(root,{recursive:true});const dir=fs.mkdtempSync(path.join(root,'intelligence-cli-'));
  try {
    const fixtures=syntheticFixtures();const input={version:1,fixtures:fixtures.map(f=>({...f,image:Array.from(f.image)})),candidateResults:fixtures.flatMap(f=>[deterministicBaseline.propose(f),deterministicBaseline.propose(f)])};
    const file=path.join(dir,'fixture.json');fs.writeFileSync(file,canonical(input));
    const report=JSON.parse(execFileSync(process.execPath,['scripts/evaluate-intelligence.mjs',file],{encoding:'utf8'}));
    assert.equal(report.candidate.semanticCorrect,8);assert.equal(report.candidate.admitted,false);assert.match(report.latencyScope,/NOT model inference/);
  }finally{fs.rmSync(dir,{recursive:true,force:true});}
});
test('round 1 authority remains additive and unadmitted; complete retained CI includes tools',()=>{
  const read=p=>fs.readFileSync(p,'utf8');
  for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md']){
    const current=read(p).split('<!-- phase7-step7-round1 -->')[1]?.split('<!-- phase7-step7-round1-history -->')[0];assert.ok(current,p);
    for(const term of ['ADMISSION BLOCKED','Production model UNRESOLVED','Step 8 NOT STARTED','Phase 8 NOT STARTED','256 MiB','Vulkan/Metal UNKNOWN'])assert.ok(current.includes(term),p+term);
  }
  const native=read('packages/core/src/intelligence_resources.hpp');
  assert.doesNotMatch(native,/document\.hpp|editor\.h|agent_execution|storage\.hpp|litert|approval/i);
  assert.match(read('scripts/check-native-zig.ps1'),/check-decode-zig.ps1/);
  assert.doesNotMatch(read('scripts/check-native-zig.ps1'),/document\.cpp|storage\.cpp/);
  assert.match(read('CMakeLists.txt'),/add_test\(NAME core_ubsan_test COMMAND core_ubsan_test\)/);
  for(const workflow of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(read(workflow).includes('tests/intelligence-admission.test.mjs'));
  assert.ok(read('.github/workflows/native-shells.yml').includes('check-android-release.py'));
});
