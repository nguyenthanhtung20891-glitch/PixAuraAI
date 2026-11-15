// Metadata-only verification component. No HTTP listener/deployment, accounts,
// logging, database, photos, quota history or analytics. Operator supplies config,
// OAuth provider and P-256 signing key; no credentials/default product IDs here.
import { createHash, createPrivateKey, sign } from 'node:crypto';
const deny = () => { throw new Error('Store verification unavailable'); };
async function boundedJson(response) {
  if (!response.ok) return deny();
  const reader = response.body.getReader(); let bytes = 0; const chunks = [];
  try { for (;;) { const { done, value } = await reader.read(); if (done) break; bytes += value.length; if (bytes > 65536) return deny(); chunks.push(value); } }
  finally { await reader.cancel().catch(() => {}); }
  try { return JSON.parse(Buffer.concat(chunks).toString('utf8')); } catch { return deny(); }
}
export async function verifyGooglePurchase(input, config, { accessToken, signingKey, fetcher = fetch, revocationStatus = async () => 'unknown', nowSeconds = () => Math.floor(Date.now() / 1000) }) {
  try {
    if (typeof input !== 'string' || Buffer.byteLength(input) > 8192) return deny();
    const r = JSON.parse(input);
    if (!r || Array.isArray(r) || Object.keys(r).sort().join() !== 'kind,nonce,productId,purchaseToken' ||
        !['subscription', 'lifetime'].includes(r.kind) || !/^[a-f0-9]{32}$/.test(r.nonce) ||
        typeof r.purchaseToken !== 'string' || r.purchaseToken.length < 1 || r.purchaseToken.length > 4096 || /[\x00-\x20\x7f]/.test(r.purchaseToken) ||
        typeof r.productId !== 'string' || !/^[a-zA-Z0-9._-]{1,128}$/.test(r.productId) ||
        config.products?.[r.productId] !== r.kind || !/^[a-zA-Z][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9_]*)+$/.test(config.packageName)) return deny();
    const token = await accessToken();
    if (typeof token !== 'string' || token.length > 4096 || /[\r\n]/.test(token) || !token) return deny();
    const base = 'https://androidpublisher.googleapis.com/androidpublisher/v3/applications/' + encodeURIComponent(config.packageName) + '/purchases/';
    const url = r.kind === 'lifetime' ? base + 'products/' + encodeURIComponent(r.productId) + '/tokens/' + encodeURIComponent(r.purchaseToken) : base + 'subscriptionsv2/tokens/' + encodeURIComponent(r.purchaseToken);
    const fields = r.kind === 'lifetime' ? 'kind,purchaseState,purchaseTimeMillis,acknowledgementState,consumptionState,purchaseType,quantity' : 'subscriptionState,lineItems(productId,expiryTime),testPurchase';
    const raw = await boundedJson(await fetcher(url + '?fields=' + encodeURIComponent(fields), { headers: { Authorization: 'Bearer ' + token, 'Cache-Control': 'no-store' }, redirect: 'error', signal: AbortSignal.timeout(10000) }));
    let status, expiry = 0;
    if (r.kind === 'lifetime') {
      if (!Number.isInteger(raw.purchaseState) || raw.kind !== 'androidpublisher#productPurchase') return deny();
      if (raw.purchaseState === 0 && (!/^[0-9]{1,16}$/.test(raw.purchaseTimeMillis) || (!Number.isSafeInteger(Number(raw.purchaseTimeMillis)) || Number(raw.purchaseTimeMillis) <= 0 || Number(raw.purchaseTimeMillis) > 8640000000000000) ||
          ![0, 1].includes(raw.acknowledgementState) || raw.consumptionState !== 0 || (raw.quantity !== undefined && raw.quantity !== 1))) return deny();
      status = ({ 0: 'active', 1: 'revoked', 2: 'pending' })[raw.purchaseState];
    } else {
      const items = raw.lineItems?.filter(x => x.productId === r.productId);
      if (!items?.length) return deny();
      expiry = Math.max(...items.map(x => Math.floor(Date.parse(x.expiryTime) / 1000)));
      if (!Number.isSafeInteger(expiry) || expiry < 0 || expiry > 8640000000000) return deny();
      status = ({ SUBSCRIPTION_STATE_ACTIVE: 'active', SUBSCRIPTION_STATE_IN_GRACE_PERIOD: 'active', SUBSCRIPTION_STATE_CANCELED: 'active', SUBSCRIPTION_STATE_EXPIRED: 'expired', SUBSCRIPTION_STATE_PENDING: 'pending', SUBSCRIPTION_STATE_PAUSED: 'inactive', SUBSCRIPTION_STATE_ON_HOLD: 'inactive' })[raw.subscriptionState];
    }
    if (r.kind === 'subscription') {
      // A GET showing expired alone cannot distinguish natural expiry from an
      // authoritative revoke/refund. Production needs authenticated store-event
      // reconciliation; absent/unknown evidence fails closed, never adds grace.
      const revoked = await revocationStatus(createHash('sha256').update(r.purchaseToken).digest('hex'));
      if (revoked === 'revoked') status = 'revoked';
      else if (revoked !== 'not_revoked') return deny();
    }
    if (!status) return deny();
    const issuedAt = nowSeconds(); if (!Number.isSafeInteger(issuedAt) || issuedAt < 0 || issuedAt > 8640000000000) return deny();
    if (r.kind === 'subscription' && status === 'active' && expiry <= issuedAt) status = 'expired';
    const environment = r.kind === 'lifetime' ? (raw.purchaseType === 0 ? 'SANDBOX' : 'PRODUCTION') : (raw.testPurchase ? 'SANDBOX' : 'PRODUCTION');
    const payload = JSON.stringify({ version: 1, store: 'GOOGLE', environment, packageName: config.packageName, productId: r.productId, kind: r.kind, status, expires: expiry, issuedAt, nonce: r.nonce, tokenSha256: createHash('sha256').update(r.purchaseToken).digest('hex') });
    const key = createPrivateKey(signingKey);
    if (key.asymmetricKeyType !== 'ec' || key.asymmetricKeyDetails?.namedCurve !== 'prime256v1') return deny();
    return Object.freeze({ payload, signature: sign('sha256', Buffer.from(payload), key).toString('base64') });
  } catch { return deny(); } // Never echo tokens, store response bodies or credentials.
}
