import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';

test('segmentation synthetic geometry, exact mask digests and counts are reproducible',()=>{
    execFileSync(process.execPath,['scripts/segmentation-evaluation-fixtures.mjs','--check']);
    const corpus=JSON.parse(fs.readFileSync('tests/fixtures/segmentation-evaluation-v1.json','utf8'));
    assert.equal(corpus.selection,'UNRESOLVED');assert.equal(corpus.cases.length,24);
    for(const c of corpus.cases){
        assert.ok([8,256].includes(c.size));assert.equal(c.pixels,c.size*c.size);
        const [tp,fp,fn,tn]=c.reference_counts;assert.equal(tp+fp+fn+tn,c.pixels);assert.equal(tp+fn,c.foreground);
        assert.match(c.expected_sha256,/^[a-f0-9]{64}$/);
        if(c.id==='low-contrast'||c.id==='small'){assert.equal(tp,0);assert.equal(fn,c.foreground);}
        else if(c.id!=='gradient'){assert.equal(tp,c.foreground);assert.equal(fp+fn,0);}
    }
    // Independent hand calculations, unrelated to interpreter output.
    const ratio=(a,b)=>b?a/b:1;
    assert.equal(ratio(1,1+1+1),1/3);assert.equal(ratio(2,2+1+1),1/2);
    assert.equal(ratio(0,0),1);assert.equal(ratio(0,3),0);
});

test('engineering evaluation stays isolated from release and uses checked provider output',()=>{
    const runner=fs.readFileSync('packages/core/tests/segmentation_evaluation_boundary.hpp','utf8');
    for(const call of ['verify_owned','pixaura_inference_prepare','pixaura_inference_execute','pixaura_inference_output_copy','pixaura_inference_output_info','evaluation_identity'])assert.ok(runner.includes(call));
    for(const file of ['inference_boundary.hpp','inference_android_boundary.hpp','inference_apple_boundary.hpp'])assert.ok(fs.readFileSync(`packages/core/tests/${file}`,'utf8').includes('segmentation_evaluation('));
    assert.ok(fs.readFileSync('scripts/check-model-release.mjs','utf8').includes("Buffer.from('PXSEG')"));
    const source=fs.readFileSync('packages/core/src/inference.cpp','utf8');
    assert.match(source,/#ifdef PIXAURA_CERTIFICATION_TESTING\s+#include "\.\.\/tests\/inference_boundary.hpp"/);
});

test('bounded platform evidence fragments preserve JSON and reject malformed transport',()=>{
    fs.mkdirSync('build',{recursive:true});const directory=fs.mkdtempSync('build/segmentation-transport-');
    try{
        const row={evaluation_version:1,metric_version:1,case_id:'transport-test-only',width:1,height:1,threshold:[1,2],
            expected_sha256:'0'.repeat(64),predictor:'all-background',candidate_state:'BASELINE',production_selection:'UNRESOLVED',
            provider_id:null,manifest_sha256:null,payload_sha256:null,counts:[0,0,0,1],iou:[1,1],dice:[1,1],accuracy:[1,1],precision:[1,1],recall:[1,1],
            transport_test_padding:'x'.repeat(2800)};
        const encoded=JSON.stringify(row);
        const packets=width=>{const parts=encoded.match(new RegExp(`.{1,${width}}`,'g'));return parts.map((part,i)=>`I/PixAuraEvaluation(123): PXSEG_PART 1 ${i} ${parts.length} ${part}`);};
        const lines=packets(400);
        const run=(records,success=true)=>{
            fs.writeFileSync(`${directory}/input.log`,records.join('\n')+'\n');let failed=false;
            try{execFileSync(process.execPath,['scripts/segmentation-results.mjs',`${directory}/input.log`,`${directory}/output.jsonl`,'transport-test-only'],{stdio:'pipe'});}
            catch{failed=true;}assert.equal(failed,!success);
            if(success){const value=JSON.parse(fs.readFileSync(`${directory}/output.jsonl`,'utf8'));delete value.evidence;assert.deepEqual(value,row);}
        };
        run(lines);run(packets(700));run(lines.slice(0,-1),false);run([...lines.slice(0,1),...lines],false);
        run([...lines].reverse(),false);run([lines[0].replace(' 0 ',' 4 ')],false);
        run([`PXSEG ${encoded}`]);
    }finally{fs.rmSync(directory,{recursive:true,force:true});}
});

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

test('Android fixed Conv has bounded parity against an independent arithmetic oracle',()=>{
    execFileSync(process.execPath,['scripts/android-inference-fixtures.mjs','--check']);
    const contract=fs.readFileSync('docs/contracts/android-inference-provider-v1.md','utf8');
    assert.match(contract,/4e-38 \+ 3e-7/);
    // Enumerate every accepted quarter-weight triple, including 0.75 cases.
    const values=[0,1,0.5,0.1,1/3,2**-126,2**-149,0.99999994];
    let combinations=0;
    for(let r=0;r<=4;r++)for(let g=0;g<=4-r;g++){
        const w=[r/4,g/4,(4-r-g)/4];combinations++;
        for(const a of values)for(const b of values)for(const c of values){
            const x=[a,b,c].map(Math.fround);
            const reference=Math.fround(x[0]*w[0]+x[1]*w[1]+x[2]*w[2]);
            const actual=Math.fround(Math.fround(Math.fround(x[0]*w[0])+Math.fround(x[1]*w[1]))+Math.fround(x[2]*w[2]));
            assert.ok(Math.abs(actual-reference)<=4e-38+3e-7*Math.abs(reference));
        }
    }
    assert.equal(combinations,15);
    const shader=fs.readFileSync('platforms/android/native/segmentation.comp','utf8');
    assert.match(shader,/precise float probability=rg\+b/);
    assert.doesNotMatch(shader,/\b(clamp|fma|float16_t|double)\b/);
});

test('Apple fixed rounded F32 chain independently bounds RTE/RTZ and FTZ',()=>{
    execFileSync(process.execPath,['scripts/metal-inference-shaders.mjs','--check']);
    const shader=fs.readFileSync('packages/core/swift/Sources/Shaders/segmentation.metal','utf8');
    assert.equal((shader.match(/=fma\(/g)||[]).length,5);
    assert.doesNotMatch(shader,/\b(half|double|clamp|fast)\b/);
    const scratch=Buffer.alloc(4);
    function rounded(value,rtz,ftz){let v=Math.fround(value);
        if(rtz&&v>value){scratch.writeFloatLE(v);scratch.writeUInt32LE(scratch.readUInt32LE()-1);v=scratch.readFloatLE();}
        return ftz&&v<2**-126?0:v;
    }
    const values=[0,1,0.1,1/3,0.5,0.99999994,2**-126,2**-149].map(Math.fround);
    for(let r=0;r<=4;r++)for(let g=0;g<=4-r;g++)for(const a of values)for(const b of values)for(const c of values){
        const w=[r/4,g/4,(4-r-g)/4],input=[a,b,c];
        const cpu=Math.fround(a*w[0]+b*w[1]+c*w[2]);
        for(const rtz of [false,true])for(const ftz of [false,true]){
            const mul=input.map((x,i)=>rounded((ftz&&x<2**-126?0:x)*w[i],rtz,ftz));
            const actual=rounded(rounded(mul[0]+mul[1],rtz,ftz)+mul[2],rtz,ftz);
            assert.ok(Math.abs(actual-cpu)<=4e-38+3e-7*Math.abs(cpu));
        }
    }
    const swift=fs.readFileSync('packages/core/swift/Sources/MetalValidation.swift','utf8');
    assert.match(swift,/pixaura_metal_exposure_dispatch/);assert.doesNotMatch(swift,/makeCommandQueue|makeBuffer\(|makeLibrary\(/);
});
