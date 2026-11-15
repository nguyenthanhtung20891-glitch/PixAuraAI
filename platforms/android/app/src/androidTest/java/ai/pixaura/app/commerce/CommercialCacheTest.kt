package ai.pixaura.app.commerce

import androidx.test.platform.app.InstrumentationRegistry
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import java.io.File
import java.security.KeyPair
import java.security.KeyPairGenerator
import java.security.Signature
import java.security.spec.ECGenParameterSpec
import java.util.Base64
import java.util.UUID

// Synthetic cryptographic/cache tests, NOT Google Play sandbox transactions.
class CommercialCacheTest {
    private val context = InstrumentationRegistry.getInstrumentation().targetContext
    private val products = mapOf("synthetic.fixture" to "lifetime", "synthetic.monthly" to "subscription")
    private val nonce = "a".repeat(32)
    private val token = "b".repeat(64)
    private fun keys(): KeyPair = KeyPairGenerator.getInstance("EC").apply { initialize(ECGenParameterSpec("secp256r1")) }.generateKeyPair()
    private fun payload(time: Long = 2000000000) = JSONObject().put("version", 1).put("store", "GOOGLE").put("environment", "SANDBOX")
        .put("packageName", context.packageName).put("productId", "synthetic.fixture").put("kind", "lifetime").put("status", "active")
        .put("expires", 0).put("issuedAt", time).put("nonce", nonce).put("tokenSha256", token).toString()
    private fun signed(k: KeyPair, payload: String): String = Base64.getEncoder().encodeToString(Signature.getInstance("SHA256withECDSA").apply { initSign(k.private); update(payload.toByteArray()) }.sign())
    private fun record(k: KeyPair, time: Long = 2000000000): VerifiedGoogleRecord { val p = payload(time); return VerifiedGoogleRecord.verify(p, signed(k, p), k.public, context.packageName, products, "SANDBOX", nonce, token) }
    @Test fun signatureBindingAndUnverifiedInputsFailClosed() {
        val k = keys(); val p = payload(); val signature = signed(k, p)
        assertEquals("active", record(k).status)
        for (changed in listOf(p + " ", p.replace("SANDBOX", "PRODUCTION"), p.replace(nonce, "c".repeat(32)))) {
            try { VerifiedGoogleRecord.verify(changed, signature, k.public, context.packageName, products, "SANDBOX", nonce, token); fail("tampered grant") } catch (_: Exception) { }
        }
        try { VerifiedGoogleRecord.verify(p, signature, keys().public, context.packageName, products, "SANDBOX", nonce, token); fail("wrong key") } catch (_: Exception) { }
        try { VerifiedGoogleRecord.verify(p, signature, k.public, context.packageName, products, "SANDBOX", nonce, "c".repeat(64)); fail("wrong transaction") } catch (_: Exception) { }
    }
    @Test fun keystoreCacheRejectsCorruptionAndStaleFactsAndRecoversOnlyFromVerifiedFact() {
        val k = keys(); val namespace = "commercial-test-" + UUID.randomUUID().toString(); val cache = ProtectedCommercialCache(context, namespace)
        try {
            assertTrue(cache.read(k.public, products, "SANDBOX").isEmpty()); cache.write(record(k))
            assertEquals("active", cache.read(k.public, products, "SANDBOX").single().status)
            val pending = JSONObject(payload(2000000001)).put("status", "pending").toString()
            cache.write(VerifiedGoogleRecord.verify(pending, signed(k, pending), k.public, context.packageName, products, "SANDBOX", nonce, token))
            assertEquals("active", cache.read(k.public, products, "SANDBOX").single().status)
            try { cache.write(record(k, 1999999999)); fail("stale rollback") } catch (_: Exception) { }
            val path = File(context.noBackupFilesDir, "$namespace.cache"); val bytes = path.readBytes(); bytes[bytes.lastIndex] = (bytes.last().toInt() xor 1).toByte(); path.writeBytes(bytes)
            try { cache.read(k.public, products, "SANDBOX"); fail("corrupt cache") } catch (_: Exception) { }
            cache.repairAfterStoreVerification(record(k, 2000000001)); assertEquals(2000000001L, cache.read(k.public, products, "SANDBOX").single().issuedAt)
            val monthly = JSONObject(payload(2000000001)).put("productId", "synthetic.monthly").put("kind", "subscription").put("expires", 2000000100L).toString()
            cache.write(VerifiedGoogleRecord.verify(monthly, signed(k, monthly), k.public, context.packageName, products, "SANDBOX", nonce, token))
            assertEquals(2, cache.read(k.public, products, "SANDBOX").size)
            assertTrue(cache.read(k.public, products, "SANDBOX").any { it.productId == "synthetic.fixture" && it.status == "active" })
            val store = java.security.KeyStore.getInstance("AndroidKeyStore"); store.load(null); store.deleteEntry(context.packageName + "." + namespace)
            try { cache.read(k.public, products, "SANDBOX"); fail("lost key") } catch (_: Exception) { }
            cache.repairAfterStoreVerification(record(k, 2000000002)); assertEquals(2000000002L, cache.read(k.public, products, "SANDBOX").single().issuedAt)
        } finally { cache.clear() }
    }
}
