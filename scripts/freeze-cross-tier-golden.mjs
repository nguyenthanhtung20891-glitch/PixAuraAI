import fs from 'node:fs';
import assert from 'node:assert/strict';
const root = process.argv[2] ?? 'build/step3-linux';
const checking = process.argv.includes('--check');
const frozen = checking ? JSON.parse(fs.readFileSync('tests/fixtures/cross-tier/golden.json','utf8')) : null;
const header = checking ? fs.readFileSync('tests/fixtures/cross-tier/golden.h','utf8') : '';
const raw = name => header.match(new RegExp(`${name}\\[\\] = R"CERT\\(([\\s\\S]*?)\\)CERT";`))[1];
// Writer adds one artifact delimiter newline. Preserve the core canonical
// serializer's own newline; whitespace is part of exact canonical comparison.
const canonical = checking ? raw('cross_tier_golden_document') : fs.readFileSync(`${root}/cross-tier-canonical.json`, 'utf8').slice(0,-1);
const redo = checking ? raw('cross_tier_golden_redo') : fs.readFileSync(`${root}/cross-tier-redo.json`, 'utf8').slice(0,-1);
const reference = checking ? frozen.reference.sha256 : fs.readFileSync(`${root}/cross-tier-reference.sha256`, 'utf8').trim();
const document = JSON.parse(canonical), checkpoint = JSON.parse(redo);
const expected = [
  ['pixaura.crop', {height_ppm:750000,width_ppm:500000,x_ppm:125000,y_ppm:250000}],
  ['pixaura.rotate', {quarter_turns:1}], ['pixaura.exposure', {milli_ev:1000}],
  ['pixaura.contrast', {milli_stops:1000}], ['pixaura.saturation', {milli_ratio:1500}],
  ['pixaura.blur', {milli_strength:500}], ['pixaura.sharpen', {milli_amount:500}],
];
assert.equal(document.schema_version,1);assert.equal(document.revisions.length,8);
assert.deepEqual(document.operations.map(op=>[op.type,op.parameters]),expected);
for(const operation of document.operations){assert.equal(operation.operation_version,1);assert.equal(operation.parameter_version,1);}
for(let i=0;i<8;i++){
  assert.deepEqual(document.revisions[i].stack,document.operations.slice(0,i).map(op=>op.id));
  assert.equal(document.revisions[i].parent_id,i===0?null:document.revisions[i-1].id);
}
assert.equal(document.current_revision_id,document.revisions[7].id);assert.deepEqual(document.redo,[]);
assert.equal(checkpoint.current_revision_id,document.revisions[5].id);
assert.deepEqual(checkpoint.redo,[document.revisions[6].id,document.revisions[7].id]);
assert.deepEqual({...checkpoint,current_revision_id:document.current_revision_id,redo:[]},document);
assert.match(reference,/^[a-f0-9]{64}$/);
assert.equal(document.source.sha256,JSON.parse(fs.readFileSync('tests/fixtures/cross-tier/provenance.json','utf8')).fixtures[0].sha256);
for(const key of ['tier','thermal','memory_pressure','policy_epoch','capability']) assert.ok(!canonical.includes(`"${key}"`));
const initial = JSON.stringify({...document,current_revision_id:document.revisions[0].id,operations:[],redo:[],revisions:[document.revisions[0]]})+'\n';
if(checking){assert.deepEqual(document,frozen.document);assert.deepEqual(checkpoint,frozen.redoCheckpoint);assert.equal(raw('cross_tier_golden_root'),initial);assert.ok(header.includes(`"${reference}"`));}
else {
  fs.writeFileSync('tests/fixtures/cross-tier/golden.h',`#ifndef PIXAURA_CROSS_TIER_GOLDEN_H\n#define PIXAURA_CROSS_TIER_GOLDEN_H\n// SYNTHETIC canonical artifact; independently checked ordered graph/parameters.\nstatic const char cross_tier_golden_document[] = R"CERT(${canonical})CERT";\nstatic const char cross_tier_golden_redo[] = R"CERT(${redo})CERT";\nstatic const char cross_tier_golden_root[] = R"CERT(${initial})CERT";\nstatic const char cross_tier_golden_reference[] = "${reference}";\n#endif\n`);
  fs.writeFileSync('tests/fixtures/cross-tier/golden.json',JSON.stringify({evidence:'SYNTHETIC POLICY EVIDENCE',document,redoCheckpoint:checkpoint,reference:{format:'SHA256(width_BE32,height_BE32,RGBA32F_BE_words)',sha256:reference}},null,2)+'\n');
}
console.log(`independently checked canonical identities/order/history/parameters ${checking?'PASS':'frozen'}`);
