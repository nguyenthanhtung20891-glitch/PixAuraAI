import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';

test('genuine signed reference ONNX and independent dyadic goldens remain reproducible',()=>{
    execFileSync(process.execPath,['scripts/inference-fixtures.mjs','--check']);
    const data=JSON.parse(fs.readFileSync('tests/fixtures/inference-reference-v1.json','utf8'));
    assert.equal(data.comparison,'exact binary32 words');assert.equal(data.cases.length,4);
    assert.match(data.model_sha256,/^[a-f0-9]{64}$/);
    for(const g of data.cases)for(let i=0;i<4;++i){
        const numerator=g.input_quarter_units[i]+2*g.input_quarter_units[i+4]+g.input_quarter_units[i+8];
        const bytes=Buffer.alloc(4);bytes.writeFloatLE(numerator/16);
        assert.equal(bytes.readUInt32LE().toString(16).padStart(8,'0'),g.output_hex[i]);
    }
    // A signed-package fixture's model bytes must be genuinely structured ONNX,
    // not Step 1's non-executable text renamed to .onnx.
    const header=fs.readFileSync('packages/core/tests/inference_fixtures.hpp','utf8');
    const raw=header.match(/\{"valid-golden-2x2",0,\{([^}]+)\}/)[1];
    const model=Buffer.from(raw.split(',').map(Number));
    assert.equal(crypto.createHash('sha256').update(model).digest('hex'),data.model_sha256);
    assert.ok(model.includes(Buffer.from('Conv')));assert.ok(model.includes(Buffer.from('weights')));
    assert.equal(model[0],8);assert.equal(model[1],9);
});
test('reference runtime has no external engine or release certification surface',()=>{
    const source=fs.readFileSync('packages/core/src/inference.cpp','utf8');
    assert.match(source,/#ifdef PIXAURA_CERTIFICATION_TESTING\s+#include "\.\.\/tests\/inference_boundary.hpp"/);
    const header=fs.readFileSync('packages/core/include/pixaura/inference.h','utf8');
    assert.ok(!/set_trust|execute_path|load_raw|private_key/.test(header));
    const contract=fs.readFileSync('docs/contracts/cpu-reference-provider-v1.md','utf8');
    assert.match(contract,/does NOT support general ONNX execution/);
    assert.ok(!/onnxruntime|tensorflow|ncnn|executorch/i.test(fs.readFileSync('CMakeLists.txt','utf8')));
});
