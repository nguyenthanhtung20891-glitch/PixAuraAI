package ai.pixaura.app.commerce

import java.security.PublicKey

// Metadata verified under the configured verifier/catalog, not a native access grant.
// Native bridge must bind its configured verifier/owner and independently project policy.
class VerifiedNativeStoreHandoffV1 private constructor(val record: VerifiedGoogleRecord, val origin: Origin) {
    enum class Origin { SIGNATURE_VERIFIED_METADATA, PROTECTED_CACHE }
    companion object {
        internal fun fresh(record: VerifiedGoogleRecord) = VerifiedNativeStoreHandoffV1(record, Origin.SIGNATURE_VERIFIED_METADATA)
        internal fun cached(record: VerifiedGoogleRecord) = VerifiedNativeStoreHandoffV1(record, Origin.PROTECTED_CACHE)
    }
}
class GoogleProtectedCacheReplayV1(private val cache: ProtectedCommercialCache, private val key: PublicKey,
    private val products: Map<String, String>, private val environment: String,
    private val sink: (VerifiedNativeStoreHandoffV1) -> Unit) : AutoCloseable {
    private val gate = Any()
    private var closed = false
    fun replay() = synchronized(gate) {
        check(!closed)
        cache.read(key, products, environment).forEach { if (closed) return@synchronized; sink(VerifiedNativeStoreHandoffV1.cached(it)) }
    }
    override fun close() = synchronized(gate) { closed = true }
}
