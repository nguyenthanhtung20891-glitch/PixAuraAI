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
    @Test fun verifiedRecordRetainsConfiguredVerifierAndSignedScopeForNativeBinding() {
        val key = keys(); val result = record(key)
        val digest = java.security.MessageDigest.getInstance("SHA-256").digest(key.public.encoded).joinToString("") { "%02x".format(it.toInt() and 255) }
        assertEquals(digest, result.verifierKeySha256); assertEquals(context.packageName, result.packageName)
        assertEquals("SANDBOX", result.environment); assertEquals("lifetime", result.kind)
        assertEquals(0L, result.expires); assertEquals(nonce, result.nonce)
        assertNotEquals(record(keys()).verifierKeySha256, result.verifierKeySha256)
    }
    @Test fun bitCountAloneDoesNotAdmitDifferentEcParameters() {
        val key = keys(); val real = key.public as java.security.interfaces.ECPublicKey
        val wrong = object : java.security.interfaces.ECPublicKey {
            override fun getAlgorithm(): String = real.algorithm
            override fun getFormat(): String = real.format
            override fun getEncoded(): ByteArray = real.encoded
            override fun getW(): java.security.spec.ECPoint = real.w
            override fun getParams(): java.security.spec.ECParameterSpec = java.security.spec.ECParameterSpec(real.params.curve, real.params.generator, real.params.order, real.params.cofactor + 1)
        }
        val text = payload()
        try { VerifiedGoogleRecord.verify(text, signed(key, text), wrong, context.packageName, products, "SANDBOX", nonce, token); fail("same-bit-count different curve admitted") } catch (_: IllegalArgumentException) { }
    }
    @Test fun keyFingerprintAndSignatureCannotUseDifferentPublicKeys() {
        val advertised = keys(); val signing = keys()
        val real = signing.public as java.security.interfaces.ECPublicKey
        val inconsistent = object : java.security.interfaces.ECPublicKey {
            override fun getAlgorithm(): String = real.algorithm
            override fun getFormat(): String = real.format
            override fun getEncoded(): ByteArray = advertised.public.encoded
            override fun getW(): java.security.spec.ECPoint = real.w
            override fun getParams(): java.security.spec.ECParameterSpec = real.params
        }
        val text = payload()
        try { VerifiedGoogleRecord.verify(text, signed(signing, text), inconsistent, context.packageName, products, "SANDBOX", nonce, token); fail("fingerprinted identity differed from signature key") } catch (_: IllegalArgumentException) { }
    }
    @Test fun actualOsClockHasStableLocalBootAndMonotonicSamples() {
        val source = AndroidCommercialClockSourceV1(context)
        val first = source.sample(); val second = source.sample()
        assertTrue(first.utcSeconds >= 0 && first.uptimeSeconds >= 0)
        assertTrue(second.uptimeSeconds >= first.uptimeSeconds)
        assertEquals(first.bootToken, second.bootToken)
        assertTrue(first.bootToken.matches(Regex("[a-f0-9]{32}")))
    }
    @Test fun signatureBindingAndUnverifiedInputsFailClosed() {
        val k = keys(); val p = payload(); val signature = signed(k, p)
        assertEquals("active", record(k).status)
        for (changed in listOf(p + " ", p.replace("SANDBOX", "PRODUCTION"), p.replace(nonce, "c".repeat(32)))) {
            try { VerifiedGoogleRecord.verify(changed, signature, k.public, context.packageName, products, "SANDBOX", nonce, token); fail("tampered grant") } catch (_: Exception) { }
        }
        try { VerifiedGoogleRecord.verify(p, signature, keys().public, context.packageName, products, "SANDBOX", nonce, token); fail("wrong key") } catch (_: Exception) { }
        try { VerifiedGoogleRecord.verify(p, signature, k.public, context.packageName, products, "SANDBOX", nonce, "c".repeat(64)); fail("wrong transaction") } catch (_: Exception) { }
    }
    @Test fun signedCacheReplayLabelsEvidenceAndStopsBeforeOwnerTeardown() {
        val k = keys(); val namespace = "native-replay-test-" + UUID.randomUUID().toString(); val cache = ProtectedCommercialCache(context, namespace)
        val events = mutableListOf<VerifiedNativeStoreHandoffV1>()
        val replay = GoogleProtectedCacheReplayV1(cache, k.public, products, "SANDBOX") { events.add(it) }
        try {
            cache.write(record(k)); replay.replay()
            assertEquals(1, events.size); assertEquals(VerifiedNativeStoreHandoffV1.Origin.PROTECTED_CACHE, events.single().origin)
            assertEquals("synthetic.fixture", events.single().record.productId)
            replay.close()
            try { replay.replay(); fail("closed native handoff") } catch (_: IllegalStateException) { }
            assertEquals(1, events.size)
        } finally { replay.close(); cache.clear() }
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
