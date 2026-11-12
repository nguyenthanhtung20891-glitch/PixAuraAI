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
  return change?.after_sha256_lf ?? predecessor;
}
