// TEST ONLY. Reuse the reviewed RFC8032 fixture signer; never production signing.
import fs from 'node:fs';
import {signedReferenceFixture} from './inference-fixtures.mjs';
const cases=[
    ['baseline',0,{}],['minimum-tier-2',11,{tier:2}],['threads-64',11,{threads:64}],
    ['memory-256MiB',11,{memory:268435456}],['validation-2',4,{validation:2}],
    ['advisory-budget-preference',0,{budget:2,accel:1}]
];
const array=b=>`{${[...b].join(',')}}`;
const rows=cases.map(([name,status,options])=>{const f=signedReferenceFixture({h:2,w:2,...options});return `{"${name}",${status},${array(f.payload)},${array(f.manifest)},${array(f.signature)}}`;});
const output='// GENERATED TEST ONLY: signed capability declarations, public RFC8032 fixture keys.\n#ifndef PIXAURA_INFERENCE_ADMISSION_FIXTURES_HPP\n#define PIXAURA_INFERENCE_ADMISSION_FIXTURES_HPP\n#include "inference_fixtures.hpp"\nnamespace pixaura::inference::fixtures {\ninline std::vector<Case> admission_cases(){return {\n'+rows.join(',\n')+'};}\n}\n#endif\n';
const path='packages/core/tests/inference_admission_fixtures.hpp';
if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8')!==output)throw Error('Admission fixture drift');}else fs.writeFileSync(path,output);
console.log(`Admission fixtures: ${cases.length} signed requirements; unchanged ONNX profile`);
