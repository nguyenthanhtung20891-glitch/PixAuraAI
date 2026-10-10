import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {validateOperation,parseManifest,serialize,transition,replay} from './support/document-contract.mjs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');
function gaussian(pixels,w,h,x,y,c) {
  let sum=0; const k=[1,2,1];
  for(let dy=-1;dy<=1;dy++)for(let dx=-1;dx<=1;dx++)sum+=pixels[(Math.max(0,Math.min(h-1,y+dy))*w+Math.max(0,Math.min(w-1,x+dx)))*4+c]*k[dy+1]*k[dx+1];
  return sum/16;
}
test('independent hand-calculated binomial impulse and clamp borders',()=>{
  const p=Array(36).fill(0);for(let i=3;i<36;i+=4)p[i]=1;p[16]=1;
  assert.equal(gaussian(p,3,3,1,1,0),1/4);
  assert.equal(gaussian(p,3,3,0,0,0),1/16);
  p[16]=0;p[0]=1;assert.equal(gaussian(p,3,3,0,0,0),9/16);
  const transparent=[0,0,0,0,1,0,0,1];
  assert.equal(gaussian(transparent,2,1,0,0,0),1/4);
  assert.equal(gaussian(transparent,2,1,0,0,3),1/4);
});
test('frozen detail recipes exist before reliance on implementation',()=>{
  const contract=read('docs/contracts/manual-detail-v1.md');
  for(const word of ['milli_strength','milli_amount','0..1000','Clamp','786432','2->3'])assert.ok(contract.includes(word));
  assert.ok(read('packages/core/src/detail.hpp').includes('pixaura.blur'));
  assert.ok(read('packages/core/src/detail.hpp').includes('pixaura.sharpen'));
});
test('canonical detail integer validation and ordered history replay',()=>{
  const id=n=>n.toString(16).padStart(32,'0');
  let state={document:parseManifest(read('tests/fixtures/image-document-v1.json')),session_id:id(500),generation:0};
  for(const [i,[tool,key]] of [['blur','milli_strength'],['sharpen','milli_amount']].entries()){
    const op=v=>({id:id(700+i),type:'pixaura.'+tool,operation_version:1,parameter_version:1,parameters:{[key]:v}});
    for(const v of [0,1,500,999,1000])assert.doesNotThrow(()=>validateOperation(op(v)));
    for(const v of [-1,1001,.1,NaN,Infinity,-0,'0'])assert.throws(()=>validateOperation(op(v)),/INVALID_PARAMETERS/);
    assert.throws(()=>validateOperation({...op(0),parameter_version:2}),/UNSUPPORTED_OPERATION/);
    const stack=[...state.document.revisions.find(r=>r.id===state.document.current_revision_id).stack,id(700+i)];
    state=transition(state,{kind:'commit',expected_revision_id:state.document.current_revision_id,expected_session_id:state.session_id,expected_generation:state.generation,revision_id:id(800+i),actor:'manual',plan_id:null,operations:[op(500)],stack});
    const canonical=serialize(state.document);assert.equal(serialize(parseManifest(canonical)),canonical);assert.equal(replay(state.document).at(-1).type,'pixaura.'+tool);assert.equal(state.document.source.sha256,'a'.repeat(64));
  }
});
test('schemas 1/2 stay immutable; independent strict schema 3 and bounded scope',()=>{
  for(const [file,hash] of [['storage_schema.hpp','b3a43dae7e25d8efea564a8f6eeebfab29ea45cfe355d4ba99879e3e1ed84e86'],['storage_schema_v2.hpp','7408aa00c2324d0d35d674a5799ac20372eae179dc27c918dd36cad9947a5d75']])assert.equal(crypto.createHash('sha256').update(read('packages/core/src/'+file).replace(/\r\n/g,'\n')).digest('hex'),hash);
  const sql=read('packages/core/src/storage_schema_v3.hpp');assert.match(sql,/PRAGMA user_version=3/);for(const tool of ['blur','sharpen'])assert.ok(sql.includes(`${tool} IS NOT NULL AND ${tool} BETWEEN 0 AND 1000`));assert.doesNotMatch(sql,/\bBLOB\b|\bJSON\b/);
  for(const gate of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml','scripts/check-apple.sh'])assert.ok(read(gate).includes('tests/detail-tools.test.mjs'));
  assert.match(read('CMakeLists.txt'),/add_test\(NAME detail_test COMMAND detail_test\)/);
  for(let n=6;n<=8;n++)assert.match(read('ROADMAP.md'),new RegExp(`\\| ${n} \\|[^\\n]+\\| NOT STARTED \\|`));
  for(const tool of ['denoise','clarity','texture','dehaze'])assert.ok(!read('packages/core/src/detail.hpp').includes('pixaura.'+tool));
});
