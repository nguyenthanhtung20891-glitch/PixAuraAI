import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import { validateOperation, parseManifest, serialize, transition, replay } from './support/document-contract.mjs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8').replaceAll('\r\n', '\n');
test('independent integer/canonical/history oracle accepts exactly frozen tone tuples', () => {
  const contracts = [['brightness','milli_linear',-1000,1000,0],['contrast','milli_stops',-2000,2000,0],['highlights','milli_ev',-2000,2000,0],['shadows','milli_ev',-2000,2000,0],['saturation','milli_ratio',0,2000,1000],['temperature','kelvin',4000,25000,6504]];
  const id = n => n.toString(16).padStart(32,'0');
  let state = { document: parseManifest(read('tests/fixtures/image-document-v1.json')), session_id: id(500), generation: 0 };
  for (const [index,[tool, parameter,low,high,neutral]] of contracts.entries()) {
    const operation = value => ({id:id(1000+index),type:`pixaura.${tool}`,operation_version:1,parameter_version:1,parameters:{[parameter]:value}});
    for (const value of [low,low+1,neutral,high-1,high]) assert.doesNotThrow(() => validateOperation(operation(value)));
    for (const value of [low-1,high+1,0.1,NaN,Infinity,-0,'0']) assert.throws(() => validateOperation(operation(value)),/INVALID_PARAMETERS/);
    assert.throws(() => validateOperation({...operation(neutral), operation_version:2}),/UNSUPPORTED_OPERATION/);
    const prior=serialize(state.document);
    const stack=[...state.document.revisions.find(r=>r.id===state.document.current_revision_id).stack,id(1000+index)];
    state=transition(state,{kind:'commit',expected_revision_id:state.document.current_revision_id,expected_session_id:state.session_id,expected_generation:state.generation,revision_id:id(2000+index),actor:'manual',plan_id:null,operations:[operation(high)],stack});
    const canonical=serialize(state.document);
    assert.equal(serialize(parseManifest(canonical)),canonical);
    assert.equal(replay(state.document).at(-1).type,`pixaura.${tool}`);
    assert.notEqual(prior,canonical);
    assert.equal(state.document.source.sha256,'a'.repeat(64));
  }
});
test('historical schema 1 bytes remain frozen and schema 2 is independent', () => {
  assert.equal(crypto.createHash('sha256').update(read('packages/core/src/storage_schema.hpp')).digest('hex'), 'b3a43dae7e25d8efea564a8f6eeebfab29ea45cfe355d4ba99879e3e1ed84e86');
  const v2 = read('packages/core/src/storage_schema_v2.hpp');
  assert.match(v2, /PRAGMA user_version=2/);
  for (const name of ['brightness', 'contrast', 'highlights', 'shadows', 'saturation', 'temperature']) {
    assert.match(v2, new RegExp(`${name} INTEGER`));
    assert.match(v2, new RegExp(`${name} IS NOT NULL AND ${name} BETWEEN`));
  }
  assert.doesNotMatch(v2, /\bBLOB\b|\bJSON\b/);
  assert.match(v2, /CHECK\(\(ev IS NOT NULL\).*CASE WHEN type='pixaura.crop' THEN 4 ELSE 1 END\)/);
});
test('shared tone contracts and independent corpus execute through existing gates', () => {
  const contract = read('docs/contracts/manual-tone-color-v1.md');
  for (const term of ['milli_linear', 'milli_stops', 'milli_ratio', 'kelvin', '6504', 'FE_TONEAREST', 'FLT_MAX', 'inverse(M)', 'CANCEL', 'explicit migrate(1,2)']) assert.ok(contract.includes(term), term);
  const fixtures = read('packages/core/tests/tone_fixtures.hpp');
  assert.equal((fixtures.match(/\{"/g) ?? []).length, 940);
  assert.match(read('CMakeLists.txt'), /add_test\(NAME tone_test COMMAND tone_test\)/);
  assert.match(read('CMakeLists.txt'), /set_tests_properties\(tone_test PROPERTIES ENVIRONMENT/);
  for (const gate of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(gate).includes('tests/tone-color.test.mjs'), gate);
  assert.match(read('platforms/android/native/bridge.cpp'), /manual_tone_boundary_check/);
  assert.match(read('packages/core/swift/Tests/ManualBoundaryTests.swift'), /pixaura_manual_begin/);
  assert.match(read('packages/core/swift/Tests/StorageBoundaryTests.swift'), /pixaura_storage_migrate/);
});
test('Step 3 authorization preserves later-step and tool scope', () => {
  const roadmap = read('ROADMAP.md');
  for (let n=5;n<=8;n++) assert.match(roadmap, new RegExp(`\\| ${n} \\|[^\\n]+\\| NOT STARTED \\|`));
  assert.match(read('docs/reports/phase-3-step-3.md'), /Phase 3 Step 4 has NOT started/);
  const tone = read('packages/core/src/tone.hpp');
  for (const excluded of ['whites', 'blacks', 'tint', 'vibrance', 'curves', 'hsl', 'lut', 'sharpen', 'blur']) assert.ok(!tone.includes(`pixaura.${excluded}`));
});
