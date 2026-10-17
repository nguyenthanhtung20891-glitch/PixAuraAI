import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {execFileSync} from 'node:child_process';

test('Step 7 malformed signed fixtures are deterministic and preserve frozen release trust',()=>{
    execFileSync(process.execPath,['scripts/inference-certification-fixtures.mjs','--check']);
    const record=JSON.parse(fs.readFileSync('tests/fixtures/inference-certification-v1.json','utf8'));
    assert.equal(record.runtime_authority,false);assert.equal(record.production_model,'UNRESOLVED');
    assert.deepEqual(record.providers.map(p=>[p.id,p.release_trust_value]),[
        ['cpu.reference.v1',1],['android.vulkan.v1',0],['apple.metal.v1',0]]);
    const corpus=fs.readFileSync('packages/core/tests/inference_certification_boundary.hpp','utf8');
    for(const call of ['verify_owned','pixaura_inference_prepare','pixaura_inference_execute','pixaura_inference_output_copy','pixaura_inference_output_info'])assert.ok(corpus.includes(call));
    for(const [file,provider] of [['inference_boundary.hpp','provider'],['inference_android_boundary.hpp','vulkan_provider'],['inference_apple_boundary.hpp','metal_provider']]){
        assert.ok(fs.readFileSync(`packages/core/tests/${file}`,'utf8').includes(`provider_security_certification(${provider})`));
    }
});

test('threshold classification is guaranteed outside the closed tolerance band only',()=>{
    const bytes=Buffer.alloc(4),fromWord=word=>{bytes.writeUInt32LE(word);return bytes.readFloatLE();};
    const cpu=0.5,below=fromWord(0x3effffff),bound=x=>4e-38+3e-7*Math.abs(x);
    assert.ok(Math.abs(below-cpu)<=bound(cpu));assert.notEqual(below>=0.5,cpu>=0.5);
    let stable=0,ambiguous=0;
    for(const word of [0,1,0x00800000,0x3efffff0,0x3effffff,0x3f000000,0x3f000001,0x3f000010,0x3f7fffff,0x3f800000]){
        const reference=fromWord(word),tolerance=bound(reference);
        for(let delta=-8;delta<=8;++delta){const actual=fromWord(Math.max(0,Math.min(0x3f800000,word+delta)));
            if(Math.abs(actual-reference)>tolerance)continue;
            if(Math.abs(reference-0.5)>tolerance){assert.equal(actual>=0.5,reference>=0.5);stable++;}else{ambiguous++;}
        }
    }
    assert.ok(stable>0&&ambiguous>0);
});
