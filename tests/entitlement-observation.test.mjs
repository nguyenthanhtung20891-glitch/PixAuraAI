import test from 'node:test';
import assert from 'node:assert/strict';
import { validateEntitlementObservation as validate } from '../scripts/entitlement-observation.mjs';
const seed = { version: 1, store: 'APPLE', environment: 'SANDBOX', productKind: 'LIFETIME', event: 'PURCHASED', observedAtMilliseconds: 1, evidenceSha256: 'a'.repeat(64) };
test('purchase and restore observations never become verified entitlements', () => {
  for (const store of ['APPLE','GOOGLE']) for (const environment of ['SANDBOX','PRODUCTION']) for (const event of ['PENDING','PURCHASED','EXPIRED','REVOKED','RESTORED']) {
    const result = validate(JSON.stringify({ ...seed, store, environment, event }));
    assert.equal(result.trust, 'UNTRUSTED_OBSERVATION'); assert.equal(result.requiresStoreVerification, true); assert.ok(Object.isFrozen(result));
    assert.equal(Object.hasOwn(result, 'canEdit'), false);
  }
});
test('observation envelope rejects forged trust, secrets and unknown fields', () => {
  for (const field of ['verified','canEdit','purchaseToken','signedTransaction','__proto__']) {
    const input = JSON.stringify(seed).slice(0,-1) + ',"' + field + '":true}';
    assert.throws(() => validate(input), /^Error: Invalid observation envelope$/);
  }
});
test('malformed and over-budget observation payloads fail without echoing content', () => {
  for (const input of ['', 'null','[]','{}', '{broken', 'x'.repeat(4097), 1, null]) assert.throws(() => validate(input), /^Error: Invalid observation envelope$/);
  for (const patch of [{version:2},{store:'OTHER'},{environment:'LOCAL'},{observedAtMilliseconds:-1},{observedAtMilliseconds:1.5},{observedAtMilliseconds:2**53},{evidenceSha256:'not-a-verifier'},{event:'VERIFIED'}]) assert.throws(() => validate(JSON.stringify({...seed,...patch})), /^Error: Invalid observation envelope$/);
});
