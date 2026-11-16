import fs from 'node:fs';
import assert from 'node:assert/strict';

// Additive predecessor check: original historical records remain immutable.
// This changes expected fingerprints only; behavioral tests read actual source.
export function phase11Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v1.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 predecessor');
  return phase11SuccessorHash(path, change?.after_sha256_lf ?? predecessor);
}

// Follow-up implementation evidence is additive; v1 stays frozen at f585c24.
export function phase11SuccessorHash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v2.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 follow-up predecessor');
  return phase11PostV2Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Failure-state localization successor; both earlier manifests stay immutable.
export function phase11PostV2Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v3.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 localization predecessor');
  return phase11PostV3Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Physical controller build correction; all previous manifests remain immutable.
export function phase11PostV3Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v4.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 controller predecessor');
  return phase11PostV4Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Queue-retirement regression successor; no prior source receipt is rewritten.
export function phase11PostV4Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v5.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 retirement predecessor');
  return phase11PostV5Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Physical-device framework resolution successor; prior records stay immutable.
export function phase11PostV5Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v6.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 device loader predecessor');
  return phase11PostV6Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Physical fixture root uses the existing POSIX canonical-path contract.
export function phase11PostV6Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v7.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 physical fixture predecessor');
  return phase11PostV7Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Admitted-device measurement harness successor; production policy is unchanged.
export function phase11PostV7Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v8.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 device measurement predecessor');
  return phase11PostV8Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Failure diagnostics must preserve intentional application termination.
export function phase11PostV8Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v9.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase 11 UI diagnostics predecessor');
  return phase11PostV9Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Remaining Phase11 layout/security evidence; all prior receipts immutable.
export function phase11PostV9Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v10.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 remaining engineering predecessor');
  return phase11PostV10Hash(path, change?.after_sha256_lf ?? predecessor);
}

// CI controller boot-readiness correction; prior source manifests are immutable.
export function phase11PostV10Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v12.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 controller predecessor');
  return phase11PostV12Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Approved G8/G10 direction; existing ABI/schema/numerical/hardware evidence preserved.
export function phase11PostV12Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v13.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 G8/G10 predecessor');
  return phase11PostV13Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Test-only strict compiler portability; v13 evidence remains immutable.
export function phase11PostV13Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v14.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 compiler predecessor');
  return phase11PostV14Hash(path, change?.after_sha256_lf ?? predecessor);
}

export function phase11PostV14Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v15.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 tile compiler predecessor');
  return phase11PostV15Hash(path, change?.after_sha256_lf ?? predecessor);
}

export function phase11PostV15Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v16.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 exit diagnostics predecessor');
  return phase11PostV16Hash(path, change?.after_sha256_lf ?? predecessor);
}

export function phase11PostV16Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v17.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 explicit exit predecessor');
  return phase11PostV17Hash(path, change?.after_sha256_lf ?? predecessor);
}

export function phase11PostV17Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v18.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 bounded integration predecessor');
  return phase11PostV18Hash(path, change?.after_sha256_lf ?? predecessor);
}

// Private C codec MSVC layout successor; v18 failure/evidence remain immutable.
export function phase11PostV18Hash(path, predecessor) {
  const record = JSON.parse(fs.readFileSync('docs/reports/phase-11-source-v19.json', 'utf8'));
  const change = record.changes[path];
  if (change) assert.equal(change.before_sha256_lf, predecessor, path + ' Phase11 codec layout predecessor');
  return change?.after_sha256_lf ?? predecessor;
}
