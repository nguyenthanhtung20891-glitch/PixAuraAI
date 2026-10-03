import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import { limits, parseManifest, validateManifest, validateOperation, serialize, replay, transition }
  from './support/document-contract.mjs';

const golden = fs.readFileSync(new URL('./fixtures/image-document-v1.json', import.meta.url), 'utf8');
const fixture = () => parseManifest(golden);
const id = suffix => suffix.padStart(32, '0');
const command = (session, kind, extra = {}) => ({ kind, expected_revision_id: session.document.current_revision_id,
  expected_session_id: session.session_id, expected_generation: session.generation, ...extra });
const session = () => ({ document: fixture(), session_id: id('500'), generation: 0 });
const exposure = value => ({ id: id('13'), type: 'pixaura.exposure', operation_version: 1,
  parameter_version: 1, parameters: { milli_ev: value } });

test('contract golden bytes are deterministic and survive key-order normalization/reload', () => {
  assert.equal(serialize(fixture()), golden);
  const reversed = Object.fromEntries(Object.entries(fixture()).reverse());
  assert.equal(serialize(reversed), golden);
  assert.equal(serialize(parseManifest(serialize(reversed))), golden);
});

test('replay preserves explicit ordering and full stacks do not duplicate ancestor operations', () => {
  assert.deepEqual(replay(fixture()).map(value => value.type), ['pixaura.exposure', 'pixaura.rotate']);
  const state = session();
  const next = transition(state, command(state, 'commit', { revision_id: id('103'), actor: 'manual', plan_id: null,
    operations: [exposure(-1000)], stack: [id('13'), id('12')] }));
  assert.deepEqual(replay(next.document).map(value => value.parameters), [{ milli_ev: -1000 }, { quarter_turns: 1 }]);
  assert.deepEqual(replay(next.document, id('102')).map(value => value.parameters), [{ milli_ev: 1250 }, { quarter_turns: 1 }]);
  assert.equal(serialize(state.document), golden);
  const reordered = structuredClone(next.document);
  reordered.revisions.at(-1).stack.reverse();
  assert.deepEqual(replay(reordered).map(value => value.type), ['pixaura.rotate', 'pixaura.exposure']);
  assert.notEqual(serialize(reordered), serialize(next.document));
});

test('undo/redo and branching preserve history, original descriptor and reloadable redo path', () => {
  let state = session();
  state = transition(state, command(state, 'undo'));
  assert.equal(state.document.current_revision_id, id('101'));
  assert.deepEqual(state.document.redo, [id('102')]);
  assert.deepEqual(parseManifest(serialize(state.document)).redo, [id('102')]);
  state = transition(state, command(state, 'undo'));
  assert.deepEqual(state.document.redo, [id('101'), id('102')]);
  assert.throws(() => transition(state, command(state, 'undo')), /NO_HISTORY/);
  state = transition(state, command(state, 'redo'));
  state = transition(state, command(state, 'commit', { revision_id: id('103'), actor: 'manual', plan_id: null,
    operations: [exposure(0)], stack: [id('11'), id('13')] }));
  assert.deepEqual(state.document.redo, []);
  assert.equal(state.document.revisions.length, 4);
  assert.deepEqual(replay(state.document, id('102')).map(value => value.id), [id('11'), id('12')]);
  assert.deepEqual(state.document.source, fixture().source);
  assert.throws(() => transition(state, command(state, 'redo')), /NO_HISTORY/);
  state = transition(state, command(state, 'checkout', { revision_id: id('102') }));
  assert.equal(state.document.current_revision_id, id('102'));
  assert.deepEqual(state.document.redo, []);
});

test('invalid and stale commands leave inputs intact; generation rejects revision ABA', () => {
  const initial = session();
  const captured = command(initial, 'commit', { revision_id: id('103'), actor: 'manual', plan_id: null,
    operations: [exposure(5001)], stack: [id('13')] });
  assert.throws(() => transition(initial, captured), /INVALID_PARAMETERS/);
  assert.equal(serialize(initial.document), golden);
  let moved = transition(initial, command(initial, 'undo'));
  assert.throws(() => transition(moved, captured), /STALE_BASE/);
  const reopened = { ...initial, session_id: id('501') };
  assert.throws(() => transition(reopened, captured), /STALE_BASE/);
  const exhausted = { ...initial, generation: Number.MAX_SAFE_INTEGER };
  assert.throws(() => transition(exhausted, command(exhausted, 'undo')), /RESOURCE_LIMIT/);
  moved = transition(moved, command(moved, 'redo'));
  assert.equal(moved.document.current_revision_id, captured.expected_revision_id);
  assert.throws(() => transition(moved, captured), /STALE_BASE/);
  const rewritten = command(initial, 'commit', { revision_id: id('103'), actor: 'manual', plan_id: null,
    operations: [{ ...exposure(0), id: id('11') }], stack: [id('11')] });
  assert.throws(() => transition(initial, rewritten), /INVALID_PROJECT/);
  assert.equal(serialize(initial.document), golden);
});

test('operation tuple and exact parameter validation never clamps or skips future tools', () => {
  for (const value of [-5000, 0, 5000]) assert.doesNotThrow(() => validateOperation(exposure(value)));
  for (const value of [-5001, 5001, 1.5, NaN, Infinity, -0, '1000']) {
    assert.throws(() => validateOperation(exposure(value)), /INVALID_PARAMETERS/);
  }
  assert.throws(() => validateOperation({ ...exposure(0), parameters: { milli_ev: 0, extra: 1 } }), /INVALID_PARAMETERS/);
  for (const changed of [{ type: 'pixaura.contrast' }, { operation_version: 2 }, { parameter_version: 2 }]) {
    assert.throws(() => validateOperation({ ...exposure(0), ...changed }), /UNSUPPORTED_OPERATION/);
  }
  const crop = { ...exposure(0), type: 'pixaura.crop', parameters: { x_ppm: 0, y_ppm: 0, width_ppm: 1000000, height_ppm: 1000000 } };
  assert.doesNotThrow(() => validateOperation(crop));
  assert.throws(() => validateOperation({ ...crop, parameters: { ...crop.parameters, x_ppm: 1 } }), /INVALID_PARAMETERS/);
  assert.throws(() => validateOperation({ ...crop, parameters: { ...crop.parameters, width_ppm: 0 } }), /INVALID_PARAMETERS/);
  assert.throws(() => validateOperation({ ...exposure(0), type: 'pixaura.rotate', parameters: { quarter_turns: 4 } }), /INVALID_PARAMETERS/);
});

test('project rejects unknown schema/history operations, dangling references, cycles and malformed redo', () => {
  const mutations = [
    d => { d.extra = 1; }, d => { d.source.path = '../original.png'; },
    d => { d.current_revision_id = id('999'); }, d => { d.revisions[1].parent_id = id('102'); },
    d => { d.revisions[1].parent_id = d.revisions[1].id; },
    d => { d.revisions[2].id = d.revisions[1].id; }, d => { d.operations.push(d.operations[0]); },
    d => { d.revisions[2].stack.push(id('999')); }, d => { d.revisions[2].stack.push(id('11')); },
    d => { d.redo = [id('101')]; }, d => { d.revisions[1].actor = 'import'; },
    d => { d.revisions[1].actor = 'ai'; }, d => { d.revisions[1].plan_id = id('777'); },
    d => { d.operations.push(exposure(0)); }, d => { d.source.metadata.orientation = 0; },
    d => { d.source.metadata.codec = 'jpeg'; }, d => { d.source.sha256 = '../asset'; },
  ];
  for (const mutate of mutations) { const document = fixture(); mutate(document); assert.throws(() => validateManifest(document)); }
  const future = fixture(); future.schema_version = 2;
  assert.throws(() => parseManifest(JSON.stringify(future)), /UNSUPPORTED_SCHEMA/);
  const unknownHistory = fixture(); unknownHistory.operations[0].operation_version = 2;
  unknownHistory.current_revision_id = id('100');
  assert.throws(() => validateManifest(unknownHistory), /UNSUPPORTED_OPERATION/);
});

test('approved AI batch uses the same operation stack; no candidate is accepted in manifest', () => {
  const state = session();
  const approved = transition(state, command(state, 'commit', { revision_id: id('103'), actor: 'ai', plan_id: id('777'),
    operations: [exposure(-100)], stack: [id('11'), id('12'), id('13')] }));
  assert.equal(approved.document.revisions.at(-1).actor, 'ai');
  assert.deepEqual(replay(approved.document).at(-1).parameters, { milli_ev: -100 });
  const candidate = fixture(); candidate.revisions[1].actor = 'candidate';
  assert.throws(() => validateManifest(candidate), /INVALID_PROJECT/);
});

test('bounded token walk rejects corruption, duplicates, invalid UTF-8 and noninteger JSON', () => {
  for (const text of [golden.slice(0, -2), golden + '{}', '{"a":1,"a":2}',
    '{"a":1,"\\u0061":2}', '{"a":1e2}', '{"a":1.0}', '{"a":01}', '{"a":-0}',
    '{"a":9007199254740992}', '{"a":"\\ud800"}', '{"a":"\\udc00"}', '\ufeff' + golden,
    '{"a":null,}', '{"a":"\u0001"}']) assert.throws(() => parseManifest(text), /INVALID_PROJECT/);
  assert.throws(() => parseManifest(Uint8Array.of(0xff)), /INVALID_PROJECT/);
  assert.throws(() => parseManifest(' '.repeat(limits.bytes + 1)), /RESOURCE_LIMIT/);
  assert.throws(() => parseManifest('['.repeat(limits.depth + 1) + '0' + ']'.repeat(limits.depth + 1)), /RESOURCE_LIMIT/);
  assert.throws(() => parseManifest(JSON.stringify({ text: 'x'.repeat(limits.string + 1) })), /RESOURCE_LIMIT/);
});

test('document admission caps metadata/counts without pruning existing history', () => {
  const state = fixture(); const before = serialize(state);
  const oversized = structuredClone(state); oversized.source.metadata.width = 65535; oversized.source.metadata.height = 65535;
  assert.throws(() => validateManifest(oversized), /RESOURCE_LIMIT/);
  const tooMany = fixture(); tooMany.operations = Array(limits.operations + 1).fill(exposure(0));
  assert.throws(() => validateManifest(tooMany), /RESOURCE_LIMIT/);
  const tooLong = fixture(); tooLong.revisions[2].stack = Array(limits.stack + 1).fill(id('11'));
  assert.throws(() => validateManifest(tooLong), /RESOURCE_LIMIT/);
  assert.equal(serialize(state), before);
});
