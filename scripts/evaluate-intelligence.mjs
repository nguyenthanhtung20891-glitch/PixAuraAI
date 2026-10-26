// Offline recorded-output comparison. No candidate runtime is loaded or invoked.
import fs from 'node:fs';
import assert from 'node:assert/strict';
import {compare,deterministicBaseline,canonical} from './intelligence-admission.mjs';
assert.equal(process.argv.length,3,'one versioned benchmark/recorded-output file required');
const input=fs.readFileSync(process.argv[2]);assert.ok(input.length<=1048576);
const data=JSON.parse(input);assert.equal(data.version,1);
assert.ok(Array.isArray(data.fixtures)&&Array.isArray(data.candidateResults)&&data.fixtures.length<=128);
const fixtures=data.fixtures.map(f=>{assert.ok(Array.isArray(f.image)&&f.image.length<=1048576&&f.image.every(n=>Number.isInteger(n)&&n>=0&&n<=255));return {...f,image:new Uint8Array(f.image)};});
assert.equal(data.candidateResults.length,fixtures.length*2);
let index=0;
const result=compare(fixtures,deterministicBaseline,{propose:()=>data.candidateResults[index++]});
result.latencyScope='recorded-output adapter overhead; NOT model inference latency';
process.stdout.write(canonical(result));
