// TEST ONLY. Reuse the frozen engineering encoder; no production signing surface.
import fs from 'node:fs';
import {signedReferenceFixture} from './inference-fixtures.mjs';
const cases=[];
const array=b=>'{'+[...b].join(',')+'}';
for(let r=0;r<=4;r++)for(let g=0;g<=4-r;g++){
    const b=4-r-g,f=signedReferenceFixture({h:256,w:256,weights:[r/4,g/4,b/4]});
    cases.push(`{"weights-${r}${g}${b}",0,${array(f.payload)},${array(f.manifest)},${array(f.signature)}}`);
}
const path='packages/core/tests/inference_android_fixtures.hpp';
const data='// GENERATED TEST ONLY: all 15 quarter-weight triples; public RFC8032 trust.\n#ifndef PIXAURA_INFERENCE_ANDROID_FIXTURES_HPP\n#define PIXAURA_INFERENCE_ANDROID_FIXTURES_HPP\n#include "inference_fixtures.hpp"\nnamespace pixaura::inference::fixtures {\ninline std::vector<Case> android_cases(){return {\n'+cases.join(',\n')+'};}\n}\n#endif\n';
if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8')!==data)throw Error('Android inference fixture drift');}
else fs.writeFileSync(path,data);
console.log('Android inference fixtures: 15 signed 256x256 genuine ONNX models');
