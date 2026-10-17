// TEST ONLY: no new model profile; malformed signed payloads and deterministic mutations.
import fs from 'node:fs';
import {signedReferenceFixture,signedReferencePayloadFixture} from './inference-fixtures.mjs';
const original=signedReferenceFixture({}).payload;
const cases=[];
const add=(name,status,payload)=>cases.push({name,status,...signedReferencePayloadFixture(payload)});
add('unsupported-wire-fixed32',5,Buffer.from([13,0,0,0,0]));
add('unsupported-wire-group',5,Buffer.from([11,12]));
let nested=Buffer.from([8,1]);for(let depth=0;depth<16;++depth){nested=Buffer.concat([Buffer.from([58,nested.length]),nested]);}
add('excessive-nesting',5,Buffer.concat([Buffer.from([8,9]),nested,Buffer.from([66,2,16,13])]));
for(const [name,word] of [['nan',0x7fc00000],['positive-inf',0x7f800000],['negative-inf',0xff800000],['negative',0xbe800000]]){
    const bytes=Buffer.from(original),at=bytes.indexOf(Buffer.from([0,0,128,62,0,0,0,63,0,0,128,62]));
    if(at<0)throw Error('weight fixture drift');bytes.writeUInt32LE(word,at);add(`invalid-f32-${name}`,5,bytes);
}
const topology=Buffer.from(original);topology[topology.indexOf(Buffer.from('rgb'))+2]='x'.charCodeAt(0);add('topology-input-name',5,topology);
// Exhaustive bounded truncations are signed; rejection reaches the wire parser,
// rather than stopping at signature/digest verification.
for(const n of [1,2,3,7,15,31,original.length-1])add(`truncation-${n}`,6,original.subarray(0,n));
const array=b=>'{'+[...b].join(',')+'}';
const header='// GENERATED TEST ONLY: certification security corpus v1.\n#ifndef PIXAURA_INFERENCE_CERTIFICATION_FIXTURES_HPP\n#define PIXAURA_INFERENCE_CERTIFICATION_FIXTURES_HPP\n#include "inference_fixtures.hpp"\nnamespace pixaura::inference::fixtures {\ninline std::vector<Case> certification_cases(){return {\n'+cases.map(c=>`{"${c.name}",${c.status},${array(c.payload)},${array(c.manifest)},${array(c.signature)}}`).join(',\n')+'};}\n}\n#endif\n';
const path='packages/core/tests/inference_certification_fixtures.hpp';
if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8')!==header)throw Error('Certification fixture drift');}else fs.writeFileSync(path,header);
console.log(`Certification security corpus v1: ${cases.length} signed malformed ONNX fixtures`);
