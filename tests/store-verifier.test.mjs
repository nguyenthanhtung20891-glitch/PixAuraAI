import test from 'node:test';
import assert from 'node:assert/strict';
import { generateKeyPairSync, verify, createHash } from 'node:crypto';
import { verifyGooglePurchase } from '../services/store-verifier/verify.mjs';
// Synthetic protocol tests; these keys/responses are NOT store sandbox evidence.
const key = generateKeyPairSync('ec', { namedCurve: 'prime256v1' });
const config = { packageName: 'synthetic.fixture', products: { 'fixture.monthly': 'subscription', 'fixture.lifetime': 'lifetime' } };
const request = { kind: 'lifetime', nonce: 'a'.repeat(32), productId: 'fixture.lifetime', purchaseToken: 'synthetic-token' };
const body = r => JSON.stringify(r);
const deps = response => ({ revocationStatus: async () => 'not_revoked', accessToken: async () => 'synthetic-oauth', signingKey: key.privateKey.export({ type: 'pkcs8', format: 'pem' }), nowSeconds: () => 2000000000, fetcher: async (url, options) => { assert.ok(url.startsWith('https://androidpublisher.googleapis.com/')); assert.equal(options.redirect, 'error'); assert.ok(url.includes('?fields=')); return new Response(body({ kind: "androidpublisher#productPurchase", purchaseTimeMillis: "1999990000000", acknowledgementState: 0, consumptionState: 0, purchaseType: 0, ...response })); } });
test('signed store-metadata verdict binds product/token/nonce and never echoes token', async () => {
  const result = await verifyGooglePurchase(body(request), config, deps({ purchaseState: 0 }));
  assert.ok(verify('sha256', Buffer.from(result.payload), key.publicKey, Buffer.from(result.signature, 'base64')));
  const v = JSON.parse(result.payload); assert.equal(v.status, 'active'); assert.equal(v.environment, 'SANDBOX'); assert.equal(v.nonce, request.nonce); assert.equal(v.tokenSha256, createHash('sha256').update(request.purchaseToken).digest('hex'));
  assert.ok(!result.payload.includes(request.purchaseToken)); assert.ok(!verify('sha256', Buffer.from(result.payload + ' '), key.publicKey, Buffer.from(result.signature, 'base64')));
});
test('pending/revocation/expiry/hold and renewal cancellation remain distinct', async () => {
  for (const [purchaseState, status] of [[1, 'revoked'], [2, 'pending']]) assert.equal(JSON.parse((await verifyGooglePurchase(body(request), config, deps({ purchaseState }))).payload).status, status);
  const r = { ...request, kind: 'subscription', productId: 'fixture.monthly' };
  for (const [subscriptionState, status] of [['SUBSCRIPTION_STATE_CANCELED', 'active'], ['SUBSCRIPTION_STATE_EXPIRED', 'expired'], ['SUBSCRIPTION_STATE_ON_HOLD', 'inactive'], ['SUBSCRIPTION_STATE_PAUSED', 'inactive']]) {
    const raw = { subscriptionState, lineItems: [{ productId: r.productId, expiryTime: new Date(2000000100 * 1000).toISOString() }] };
    assert.equal(JSON.parse((await verifyGooglePurchase(body(r), config, deps(raw))).payload).status, status);
  }
});
test('subscription revoke evidence overrides expiry and missing authoritative channel fails closed', async () => {
  const r = { ...request, kind: 'subscription', productId: 'fixture.monthly' };
  const raw = { subscriptionState: 'SUBSCRIPTION_STATE_ACTIVE', lineItems: [{ productId: r.productId, expiryTime: new Date(2000000100 * 1000).toISOString() }] };
  const fractional = { ...raw, lineItems: [{ productId: r.productId, expiryTime: new Date(2000000100123).toISOString() }] };
  const precise = await verifyGooglePurchase(body(r), config, deps(fractional));
  assert.equal(JSON.parse(precise.payload).expires, 2000000100);
  const revoked = await verifyGooglePurchase(body(r), config, { ...deps(raw), revocationStatus: async () => 'revoked' });
  assert.equal(JSON.parse(revoked.payload).status, 'revoked');
  await assert.rejects(verifyGooglePurchase(body(r), config, { ...deps(raw), revocationStatus: async () => 'unknown' }));
});
test('unapproved products, malformed/oversized payloads, forged data and server failures fail closed', async () => {
  let requests = 0; const d = { ...deps({}), fetcher: async () => { requests++; throw Error('secret synthetic-token'); } };
  for (const r of [{ ...request, image: 'private' }, { ...request, productId: 'unknown' }, { ...request, purchaseToken: 'a\nb' }, { ...request, nonce: 'bad' }]) await assert.rejects(verifyGooglePurchase(body(r), config, d), /^Error: Store verification unavailable$/);
  assert.equal(requests, 0);
  await assert.rejects(verifyGooglePurchase('x'.repeat(8193), config, d));
  await assert.rejects(verifyGooglePurchase(body(request), config, d), e => !e.message.includes('token'));
  await assert.rejects(verifyGooglePurchase(body(request), config, deps({ purchaseState: 0, purchaseTimeMillis: 'bad' })));
  await assert.rejects(verifyGooglePurchase(body(request), config, deps({ purchaseState: 0, consumptionState: 1 })));
  await assert.rejects(verifyGooglePurchase(body(request), config, deps({ purchaseState: '0' }))); // type must be integer
});
