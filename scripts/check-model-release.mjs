import fs from 'node:fs';
import assert from 'node:assert/strict';
// Inspect actual production-configuration objects/libraries, not just source text.
const testAnchor=Buffer.from('d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a','hex');
for(const file of process.argv.slice(2)){
    const b=fs.readFileSync(file);
    for(const marker of [testAnchor,Buffer.from('pixaura_model_test_boundary'),Buffer.from('pixaura_inference_test_boundary'),Buffer.from('pixaura_inference_android_test_boundary'),Buffer.from('synthetic-segmentation')])
        assert.equal(b.includes(marker),false,`${file}: test trust/certification surface in production object`);
    console.log(`Model production trust isolation PASS: ${b.length} bytes`);
}
assert.ok(process.argv.length>2,'No production object inspected');
