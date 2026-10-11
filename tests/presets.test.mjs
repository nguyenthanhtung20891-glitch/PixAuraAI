import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
test('preset foundation freezes bounded ordered bundles and ships no catalog',()=>{
 const contract=fs.readFileSync('docs/contracts/presets-v1.md','utf8');
 for(const word of ['16384','1..16','Schema 3','No Schema 4','STALE_BASE','All-neutral'])assert.ok(contract.includes(word));
 assert.ok(fs.existsSync('packages/core/include/pixaura/preset.h'));
});
import { validateOperation } from './support/document-contract.mjs';
import crypto from 'node:crypto';
test('independent reference recipe has exact cross-platform canonical bytes',()=>{
 const text=fs.readFileSync('tests/fixtures/preset-reference-v1.json','utf8');
 const recipe=JSON.parse(text);
 const sorted=value=>Array.isArray(value)?value.map(sorted):value&&typeof value==='object'?Object.fromEntries(Object.keys(value).sort().map(k=>[k,sorted(value[k])])):value;
 assert.equal(JSON.stringify(sorted(recipe))+'\n',text);
 assert.equal(fs.readFileSync('packages/core/swift/Tests/Fixtures/preset-reference-v1.json','utf8'),text);
 assert.deepEqual(recipe.operations.map(op=>op.type),['pixaura.exposure','pixaura.brightness']);
 recipe.operations.forEach(validateOperation);
 for(const path of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml','scripts/check-apple.sh'])assert.ok(fs.readFileSync(path,'utf8').includes('tests/presets.test.mjs'));
});
test('all historical schemas remain independently immutable',()=>{
 const hashes={"storage_schema.hpp": "b3a43dae7e25d8efea564a8f6eeebfab29ea45cfe355d4ba99879e3e1ed84e86", "storage_schema_v2.hpp": "7408aa00c2324d0d35d674a5799ac20372eae179dc27c918dd36cad9947a5d75", "storage_schema_v3.hpp": "ce1a3995998cb224dc5c17309fff1e21096adbaf0d16cd0967bd257980c1e797"};
 for(const [file,hash] of Object.entries(hashes))assert.equal(crypto.createHash('sha256').update(fs.readFileSync('packages/core/src/'+file,'utf8').replace(/\r\n/g,'\n')).digest('hex'),hash);
 assert.ok(!fs.existsSync('packages/core/src/storage_schema_v4.hpp'));
 for(let step=7;step<=8;step++)assert.match(fs.readFileSync('ROADMAP.md','utf8'),new RegExp(`\\| ${step} \\|[^\\n]+\\| NOT STARTED \\|`));
});
