// Offline engineering gates. No editor, runtime loading, approval or network API.
import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
const registry=JSON.parse(fs.readFileSync(new URL('../tests/fixtures/agent-tools-v1.json',import.meta.url),'utf8')).tools;
const parameter=tool=>registry.find(t=>t.tool_id==='pixaura.'+tool.split('.')[1]).parameters[0];
export const tools=Object.freeze(['adjust.exposure','adjust.brightness','adjust.contrast','adjust.highlights','adjust.shadows','adjust.saturation','adjust.temperature','detail.sharpen']);
export function canonical(value) {
  const ordered=v=>Array.isArray(v)?v.map(ordered):v&&typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,ordered(v[k])])):v;
  return JSON.stringify(ordered(value))+'\n';
}
const text=(s,n=512)=>assert.ok(typeof s==='string'&&s.isWellFormed()&&s.length>0&&Buffer.byteLength(s)<=n&&!/[\x00-\x1f\x7f]/.test(s),'bounded text');
const hash=s=>assert.match(s,/^[a-f0-9]{64}$/);
const keys=(v,names)=>{assert.ok(v&&typeof v==='object'&&!Array.isArray(v));assert.deepEqual(Object.keys(v).sort(),names.sort());};
export function validateProvenance(bytes,artifacts) {
  assert.ok(Buffer.byteLength(bytes)<=65536);const m=JSON.parse(bytes);assert.equal(canonical(m),bytes,'canonical manifest required');
  keys(m,['version','upstream','license','processor','architecture','conversion','artifacts','runtime','custody','reproduction']);
  assert.equal(m.version,1);
  keys(m.upstream,['project','revision','artifacts']);text(m.upstream.project);text(m.upstream.revision);
  keys(m.license,['identifier','text_sha256','redistribution_evidence']);text(m.license.identifier);hash(m.license.text_sha256);text(m.license.redistribution_evidence,1024);
  keys(m.processor,['tokenizer','preprocessing']);text(m.processor.tokenizer);text(m.processor.preprocessing);
  text(m.architecture);text(m.runtime);text(m.custody,1024);text(m.reproduction,1024);
  keys(m.conversion,['tool','revision','argv','quantization','optimization','environment','intermediates']);
  for(const k of ['tool','revision','quantization','optimization','environment'])text(m.conversion[k]);
  assert.ok(Array.isArray(m.conversion.argv)&&m.conversion.argv.length>0&&m.conversion.argv.length<=32);
  m.conversion.argv.forEach(v=>text(v));
  const entries=(list,required)=>{
    assert.ok(Array.isArray(list)&&list.length<=16&&(!required||list.length>0));const seen=new Set();
    for(const a of list){keys(a,['name','sha256','bytes']);text(a.name,128);assert.match(a.name,/^[a-zA-Z0-9][a-zA-Z0-9_.-]*$/);assert.ok(!a.name.includes('..')&&!a.name.endsWith('.')&&!/^(con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\.|$)/i.test(a.name)&&!seen.has(a.name.toLowerCase()));seen.add(a.name.toLowerCase());hash(a.sha256);assert.ok(Number.isSafeInteger(a.bytes)&&a.bytes>0);}
  };
  entries(m.upstream.artifacts,true);entries(m.conversion.intermediates,false);entries(m.artifacts,true);
  assert.ok(artifacts instanceof Map);assert.deepEqual([...artifacts.keys()].sort(),m.artifacts.map(a=>a.name).sort(),'unexpected or missing artifact');
  for(const a of m.artifacts){const b=artifacts.get(a.name);assert.ok(Buffer.isBuffer(b));assert.equal(b.length,a.bytes);assert.equal(crypto.createHash('sha256').update(b).digest('hex'),a.sha256);}
  // Structural completeness/byte integrity only. Human review of claims and
  // reproduction remains mandatory; strings cannot certify license or quality.
  return Object.freeze({version:1,structurallyComplete:true,productionAdmitted:false});
}

// Versioned candidate-neutral semantic output protocol for offline measurement,
// NOT Proposal Envelope v1 and NOT accepted by an editor. Values are normalized
// benchmark values from native registry goldens; this is no execution API.
export function evaluate(fixtures,provider,repeats=2) {
  assert.ok(Array.isArray(fixtures)&&fixtures.length>0&&fixtures.length<=128);
  assert.ok(Number.isInteger(repeats)&&repeats>=2&&repeats<=10);
  const report={version:1,thresholds:'PENDING_PRINCIPAL_ARCHITECT',cases:fixtures.length,invalid:0,semanticCorrect:0,directionCorrect:0,abstentionCorrect:0,neutralFalsePositives:0,conflicts:0,repeatConsistent:0,parameterAbsoluteError:0,parameterErrorsByTool:{},latencyMilliseconds:[],memoryEvidence:[],admitted:false};
  for(const f of fixtures){
    assert.ok(f.image instanceof Uint8Array&&f.image.length>0&&f.image.length<=1048576);assert.ok(f.expected===null||tools.includes(f.expected.tool));
    if(f.expected){const p=parameter(f.expected.tool);assert.ok(Number.isSafeInteger(f.expected.value)&&f.expected.value>=p.minimum&&f.expected.value<=p.maximum&&f.expected.value!==p.default);}
    let first;
    for(let i=0;i<repeats;i++){
      let result;const before=performance.now();
      try{result=provider.propose(Object.freeze({request:f.request?Object.freeze({...f.request}):null,image:f.image.slice()}));
        keys(result,['actions','memoryEvidence']);assert.ok(Array.isArray(result.actions)&&result.actions.length<=8);
        const seen=new Set();for(const a of result.actions){keys(a,['tool','value']);assert.ok(tools.includes(a.tool)&&Number.isSafeInteger(a.value)&&!seen.has(a.tool));const p=parameter(a.tool);assert.ok(a.value>=p.minimum&&a.value<=p.maximum&&a.value!==p.default);seen.add(a.tool);}
        assert.ok(result.memoryEvidence===null||(Number.isSafeInteger(result.memoryEvidence)&&result.memoryEvidence>=0));
      }catch{report.invalid++;result=null;}
      report.latencyMilliseconds.push(performance.now()-before);
      if(i===0){first=result===null?null:canonical(result.actions);if(result){
        report.memoryEvidence.push(result.memoryEvidence);
        const actions=result.actions;
        if(!f.expected){if(actions.length===0)report.abstentionCorrect++;else report.neutralFalsePositives++;}
        else {const found=actions.find(a=>a.tool===f.expected.tool);if(found){if(actions.length===1)report.semanticCorrect++;if(Math.sign(found.value-parameter(found.tool).default)===Math.sign(f.expected.value-parameter(found.tool).default))report.directionCorrect++;const error=Math.abs(found.value-f.expected.value);report.parameterAbsoluteError+=error;(report.parameterErrorsByTool[found.tool]??=[]).push(error);}}
        if(actions.some(a=>a.tool==='adjust.exposure')&&actions.some(a=>a.tool==='adjust.brightness'))report.conflicts++;
      }}else if(result!==null&&first===canonical(result.actions))report.repeatConsistent++;
    }
  }
  return report;
}
export function compare(fixtures,reference,candidate){return {version:1,reference:evaluate(fixtures,reference),candidate:evaluate(fixtures,candidate),decision:'MEASUREMENT_ONLY_NOT_ADMITTED'};}
// Explicit deterministic baseline, not an image-understanding provider. Both
// adapters receive the SAME bounded inputs and never receive expected labels.
export const deterministicBaseline=Object.freeze({propose:({request})=>({actions:request?[{...request}]:[],memoryEvidence:null})});
export function syntheticFixtures(){
  const conditions=['underexposed','low-brightness','low-contrast','clipped-highlights','crushed-shadows','low-saturation','warm-cast','softness'];
  const values=[100,10,100,-100,100,1100,7000,100];
  return [
    ...tools.map((tool,i)=>({id:conditions[i],image:new Uint8Array(8*8*3).fill(16+i*24),request:{tool,value:values[i]},expected:{tool,value:values[i]}})),
    ...['neutral','ambiguous-conflict'].map(id=>({id,image:new Uint8Array([0,255,128]),request:null,expected:null}))
  ];
}
