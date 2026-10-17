// TEST ONLY: genuine ONNX profile, public RFC8032 signing material, independent dyadic oracle.
import fs from 'node:fs';
import crypto from 'node:crypto';
import {fileURLToPath} from 'node:url';
const seed=Buffer.from('9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60','hex');
const key=crypto.createPrivateKey({key:Buffer.concat([Buffer.from('302e020100300506032b657004220420','hex'),seed]),format:'der',type:'pkcs8'});
const cat=(...b)=>Buffer.concat(b.flat());
const varint=n=>{n=BigInt(n);const b=[];do{const v=Number(n&127n);n>>=7n;b.push(v|(n?128:0));}while(n);return Buffer.from(b);};
const integer=(id,n)=>cat(varint(id*8),varint(n));
const bytes=(id,b)=>cat(varint(id*8+2),varint(b.length),b);
const text=(id,s)=>bytes(id,Buffer.from(s));
const float=v=>{const b=Buffer.alloc(4);b.writeFloatLE(v);return b;};
const type=(dims,dynamic=false)=>bytes(1,cat(integer(1,1),bytes(2,cat(dims.map((d,i)=>bytes(1,dynamic&&i===2?text(2,'H'):integer(1,d)))))));
const value=(name,dims,dynamic=false)=>cat(text(1,name),bytes(2,type(dims,dynamic)));
function onnx(o={}){
    const h=o.h??8,w=o.w??8;
    const node=cat(text(1,'rgb'),text(1,'weights'),text(2,'probability'),text(4,o.op??'Conv'),o.attribute?bytes(5,cat(text(1,'group'),integer(3,1),integer(20,2))):Buffer.alloc(0));
    const tensor=cat([1,3,1,1].map(d=>integer(1,d)),integer(2,o.type??1),text(8,'weights'),o.storage?bytes(4,cat([.25,.5,.25].map(float))):bytes(9,o.badWeights?Buffer.alloc(11):cat((o.weights??[.25,.5,.25]).map(float))),o.external?integer(14,1):Buffer.alloc(0));
    const graph=cat(bytes(1,node),o.extraNode?bytes(1,node):Buffer.alloc(0),text(2,'pixaura-reference-v1'),bytes(5,tensor),o.extraInitializer?bytes(5,tensor):Buffer.alloc(0),bytes(11,value('rgb',o.rank===3?[1,3,h]:[1,3,h,w],o.dynamic)),bytes(12,value('probability',[1,1,o.conflict?h+1:h,w])),o.extraOutput?bytes(12,value('other',[1,1,h,w])):Buffer.alloc(0),o.sparse?bytes(15,Buffer.alloc(0)):Buffer.alloc(0));
    return cat(integer(1,9),bytes(7,graph),bytes(8,integer(2,o.opset??13)),o.training?bytes(20,Buffer.alloc(0)):Buffer.alloc(0));
}
const u32=n=>{const b=Buffer.alloc(4);b.writeUInt32BE(n);return b;};
const u64=n=>{const b=Buffer.alloc(8);b.writeBigUInt64BE(BigInt(n));return b;};
const string=s=>{const b=Buffer.from(s),n=Buffer.alloc(2);n.writeUInt16BE(b.length);return cat(n,b);};
function manifest(payload,o={}){
    const hash=crypto.createHash('sha256').update(payload).digest(),h=o.h??8,w=o.w??8;
    const t=c=>[4,1,1,1,c,h,w].map(u32);
    const b=cat(Buffer.from('PXMP'),u32(1),u32(0),[1,1,7,2,1].map(u32),string('signature.pxs'),u32(1),string('reference-segmentation'),u32(1),u32(1),t(3),t(1),[0,1000000,0,1000000,0,1000000,0,1000000].map(u32),[o.scale??1000000,0,1000000,0,1000000,0].map(u32),string('background'),string('foreground'),u32(1),[1,1,1,0,1].map(u32),hash,u32(1),u32(1),u32(o.tier??0),u32(o.threads??1),u64(o.memory??16777216),[o.budget??0,o.accel??0,o.validation??0].map(u32),string('ISC'),string('PixAuraAI original synthetic ONNX reference; no production weights'),[1,1,0].map(u32),string(''),u32(1),string('reference.onnx'),u32(1),u64(payload.length),hash);
    b.writeUInt32BE(b.length,8);return b;
}
const cases=[];
function add(name,status,o={},edit=p=>p){const payload=edit(onnx(o)),m=manifest(payload,o);const signature=cat(Buffer.from('PXSG'),[1,1,7].map(u32),crypto.sign(null,m,key));cases.push({name,status,payload,manifest:m,signature});}
add('valid-A',0);add('valid-B',0,{weights:[1,0,0]});add('valid-1x1',0,{h:1,w:1});add('valid-boundary',0,{h:1024,w:1024});add('valid-normalization',0,{scale:500000});
add('unsupported-opset',5,{opset:14});add('unsupported-operator',5,{op:'Add'});add('unsupported-attribute-valid-ONNX',5,{attribute:true});
add('unsupported-initializer-type',5,{type:7});add('unsupported-storage',5,{storage:true});add('dynamic-shape',5,{dynamic:true});
add('multiple-nodes',5,{extraNode:true});add('multiple-initializers',5,{extraInitializer:true});add('multiple-outputs',5,{extraOutput:true});
add('external-data',5,{external:true});add('sparse-tensor',5,{sparse:true});add('training',5,{training:true});add('conflicting-shape',5,{conflict:true});
add('malformed-initializer',6,{badWeights:true});add('invalid-coefficients',5,{weights:[1,1,1]});
add('truncated-wire',6,{},p=>p.subarray(0,p.length-1));add('bad-wire',6,{},()=>Buffer.from([0]));
add('oversized-field',6,{},()=>Buffer.from([58,255,255,255,255,15]));
add('overflow-varint',6,{},()=>Buffer.from([8,255,255,255,255,255,255,255,255,255,2]));
add('nonminimal-varint',6,{},()=>Buffer.from([8,137,0]));
add('unknown-semantic-field',5,{},p=>cat(p,integer(99,1)));
add('duplicate-graph',6,{},p=>cat(p,bytes(7,Buffer.alloc(0))));
add('parser-payload-limit',11,{},()=>Buffer.alloc(4097));
add('field-count-limit',11,{},()=>cat(Array.from({length:129},()=>integer(1,9))));
// Graph dimensions overflow, while the signed package still declares safe dimensions.
add('shape-overflow',5,{},()=>onnx({h:4294967295}));
add('shape-rank',5,{rank:3});
const array=b=>'{'+[...b].join(',')+'}';
let header='// GENERATED TEST ONLY: genuine ONNX, RFC8032 public test signing material.\n#ifndef PIXAURA_INFERENCE_FIXTURES_HPP\n#define PIXAURA_INFERENCE_FIXTURES_HPP\n#include <vector>\n#include <cstdint>\nnamespace pixaura::inference::fixtures {\nstruct Case {const char* name;int status;std::vector<uint8_t> payload,manifest,signature;};\ninline std::vector<Case> cases(){return {\n';
header+=cases.map(c=>`{"${c.name}",${c.status},${array(c.payload)},${array(c.manifest)},${array(c.signature)}}`).join(',\n')+'};}\n}\n#endif\n';
const goldens=['zeros','ones','gradient','checker'].map(name=>{
    const units=Array.from({length:12},(_,i)=>name==='zeros'?0:name==='ones'?4:name==='gradient'?i%5:((i+Math.floor(i/4))%2?4:0));
    const numerators=Array.from({length:4},(_,i)=>units[i]+2*units[i+4]+units[i+8]);
    return {name,dimensions:[1,3,2,2],input_quarter_units:units,output_hex:numerators.map(n=>float(n/16).readUInt32LE().toString(16).padStart(8,'0'))};
});
const golden=JSON.stringify({profile:'pixaura.onnx.reference-segmentation.v1',model_sha256:crypto.createHash('sha256').update(onnx({h:2,w:2})).digest('hex'),comparison:'exact binary32 words',oracle:'integer quarter-input units: (R+2*G+B)/16; independently computed, no interpreter',cases:goldens},null,2)+'\n';
add('valid-golden-2x2',0,{h:2,w:2});
// Refresh header with the golden-size model included.
header=header.slice(0,header.indexOf('inline std::vector<Case>'))+'inline std::vector<Case> cases(){return {\n'+cases.map(c=>`{"${c.name}",${c.status},${array(c.payload)},${array(c.manifest)},${array(c.signature)}}`).join(',\n')+'};}\n';
header+='struct Golden {const char* name;std::vector<uint32_t> input_units,output_bits;};\ninline std::vector<Golden> goldens(){return {\n'+goldens.map(g=>`{"${g.name}",${array(g.input_quarter_units)},${array(g.output_hex.map(s=>parseInt(s,16)))}}`).join(',\n')+'};}\n}\n#endif\n';
const outputs=[['packages/core/tests/inference_fixtures.hpp',header],['tests/fixtures/inference-reference-v1.json',golden]];
if(process.argv[1]===fileURLToPath(import.meta.url)){
    for(const [path,data] of outputs){if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8')!==data)throw Error('Inference fixture drift');}else fs.writeFileSync(path,data);}
    console.log(`Inference fixtures: ${cases.length} signed ONNX packages; ${goldens.length} independent goldens`);
}
// Engineering-only reuse for Step 3 fixtures; never linked into application code.
export function signedReferenceFixture(o){const payload=onnx(o),m=manifest(payload,o);
    return {payload,manifest:m,signature:cat(Buffer.from('PXSG'),[1,1,7].map(u32),crypto.sign(null,m,key))};}
