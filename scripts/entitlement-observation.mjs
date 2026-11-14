// Development evidence transport only. Not a store verifier or runtime access gate.
const fields = ['version', 'store', 'environment', 'productKind', 'event', 'observedAtMilliseconds', 'evidenceSha256'];
export function validateEntitlementObservation(input) {
  if (typeof input !== 'string' || Buffer.byteLength(input, 'utf8') > 4096) throw new Error('Invalid observation envelope');
  let value;
  try { value = JSON.parse(input); } catch { throw new Error('Invalid observation envelope'); }
  if (!value || Array.isArray(value) || typeof value !== 'object' ||
      Object.keys(value).length !== fields.length || !fields.every(k => Object.hasOwn(value, k)) ||
      value.version !== 1 || !['APPLE', 'GOOGLE'].includes(value.store) ||
      !['SANDBOX', 'PRODUCTION'].includes(value.environment) ||
      !['SUBSCRIPTION', 'LIFETIME'].includes(value.productKind) ||
      !['PENDING', 'PURCHASED', 'EXPIRED', 'REVOKED', 'RESTORED'].includes(value.event) ||
      !Number.isSafeInteger(value.observedAtMilliseconds) || value.observedAtMilliseconds < 0 ||
      typeof value.evidenceSha256 !== 'string' || !/^[a-f0-9]{64}$/.test(value.evidenceSha256)) throw new Error('Invalid observation envelope');
  return Object.freeze({ ...value, trust: 'UNTRUSTED_OBSERVATION', requiresStoreVerification: true });
}
