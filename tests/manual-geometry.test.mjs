import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const read = p => fs.readFileSync(new URL(`../${p}`, import.meta.url), 'utf8');
test('geometry manual integration reuses shared history and preview boundaries', () => {
  const api = read('packages/core/src/document_api.cpp');
  assert.match(api, /snapshots.size\(\) \+ registry.gestures.size\(\) < max_handles/);
  assert.match(api, /descriptor->category == "geometry"/);
  assert.match(api, /trial->commit\(snapshot/);
  const preview = read('packages/core/src/manual_geometry_preview.cpp');
  for (const name of ['pixaura_working_evaluate_cancel', 'pixaura_preview_render_interactive', 'pixaura_manual_geometry_current', 'pixaura_decode_release']) assert.ok(preview.includes(name));
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) assert.ok(read(file).includes('src/manual_geometry_preview.cpp'));
  for (const file of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml', 'scripts/check-apple.sh']) assert.ok(read(file).includes('tests/manual-geometry.test.mjs'));
});
test('geometry scope and remaining frozen steps stay bounded', () => {
  const roadmap = read('ROADMAP.md');
  for (let n=7;n<=8;n++) assert.match(roadmap, new RegExp(`\\| ${n} \\|[^\\n]+\\| NOT STARTED \\|`));
  assert.match(roadmap, /\| 3 \| Tone & Color Tools \| (IN PROGRESS|COMPLETE \/ FULL PASS) \|/);
  const contract = read('docs/contracts/manual-geometry-v1.md');
  for (const term of ['PROPOSED / unimplemented', '0..999999', '1..1000000', '0..3', 'No clamp', '64 owned handles', 'STALE_BASE', '1,001 updates', 'both', 'PRV1']) assert.ok(contract.includes(term));
});
