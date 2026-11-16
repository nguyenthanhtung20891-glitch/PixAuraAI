package ai.pixaura.app.commerce

import ai.pixaura.commerce.PlayBillingAdapter
import ai.pixaura.commerce.MetadataVerificationClient
import androidx.test.platform.app.InstrumentationRegistry
import com.android.billingclient.api.BillingClient
import org.junit.Assert.*
import org.junit.Test

// Real SDK runtime construction in test APK without transport/location modules.
// No connection, catalog query, purchase or authenticated store evidence is claimed.
class PlaySdkConstructionTest {
    @Test fun metadataClientRejectsUnsafeEndpointsAndClosesWithoutAnyPurchaseOrNetwork() {
        val key = java.security.KeyPairGenerator.getInstance("EC").apply { initialize(java.security.spec.ECGenParameterSpec("secp256r1")) }.generateKeyPair().public
        for (endpoint in listOf("http://synthetic.invalid/verify", "https://a@synthetic.invalid/verify", "https://synthetic.invalid/verify?q=x", "https://synthetic.invalid/verify#fragment")) {
            try { MetadataVerificationClient(java.net.URL(endpoint), key, "synthetic.fixture", mapOf("synthetic.not-queried" to "lifetime"), "SANDBOX"); fail("unsafe endpoint") } catch (_: IllegalArgumentException) { }
        }
        val client = MetadataVerificationClient(java.net.URL("https://synthetic.invalid/verify"), key, "synthetic.fixture", mapOf("synthetic.not-queried" to "lifetime"), "SANDBOX")
        client.close(); client.close() // .invalid is a reserved test name; no request is made.
    }
    @Test fun syntheticVerificationFailureCannotCallDestroyedNativeOwner() {
        // Development-only Purchase JSON + synthetic signing key. No connection,
        // checkout, Google transaction or authenticated sandbox outcome is claimed.
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val purchase = com.android.billingclient.api.Purchase(org.json.JSONObject()
            .put("productId", "synthetic.fixture").put("productIds", org.json.JSONArray().put("synthetic.fixture"))
            .put("purchaseToken", "synthetic-token-not-a-purchase").put("token", "synthetic-token-not-a-purchase")
            .put("purchaseState", 0).put("acknowledged", true).toString(), "synthetic-not-a-store-signature")
        assertEquals(com.android.billingclient.api.Purchase.PurchaseState.PURCHASED, purchase.purchaseState)
        val key = java.security.KeyPairGenerator.getInstance("EC").apply { initialize(java.security.spec.ECGenParameterSpec("secp256r1")) }.generateKeyPair()
        val hash = java.security.MessageDigest.getInstance("SHA-256").digest(purchase.purchaseToken.toByteArray()).joinToString("") { "%02x".format(it) }
        val nonce = "a".repeat(32)
        val payload = org.json.JSONObject().put("version", 1).put("store", "GOOGLE").put("environment", "SANDBOX")
            .put("packageName", context.packageName).put("productId", "synthetic.fixture").put("kind", "lifetime")
            .put("status", "active").put("expires", 0).put("issuedAt", 2000000000L).put("nonce", nonce).put("tokenSha256", hash).toString()
        val signature = java.util.Base64.getEncoder().encodeToString(java.security.Signature.getInstance("SHA256withECDSA").apply { initSign(key.private); update(payload.toByteArray()) }.sign())
        val record = VerifiedGoogleRecord.verify(payload, signature, key.public, context.packageName, mapOf("synthetic.fixture" to "lifetime"), "SANDBOX", nonce, hash)
        var unavailable = 0; var delivered = 0
        lateinit var adapter: PlayBillingAdapter
        adapter = PlayBillingAdapter(context, mapOf("synthetic.fixture" to BillingClient.ProductType.INAPP),
            verify = { _, callback -> callback(record) }, persist = { adapter.close(); error("synthetic cache failure after close") },
            unavailable = { unavailable++ }, nativeSink = { delivered++ })
        try {
            val accept = PlayBillingAdapter::class.java.getDeclaredMethod("accept", com.android.billingclient.api.Purchase::class.java).apply { isAccessible = true }
            accept.invoke(adapter, purchase)
            assertEquals(0, delivered); assertEquals("Closed adapter must not notify a destroyed native/UI owner", 0, unavailable)
        } finally { adapter.close() }
    }
    @Test fun syntheticSuccessfulPersistenceCannotCallDestroyedNativeOwner() {
        // Development-only Purchase JSON + synthetic signing key. No connection,
        // checkout, Google transaction or authenticated sandbox outcome is claimed.
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val purchase = com.android.billingclient.api.Purchase(org.json.JSONObject()
            .put("productId", "synthetic.fixture").put("productIds", org.json.JSONArray().put("synthetic.fixture"))
            .put("purchaseToken", "synthetic-token-not-a-purchase").put("token", "synthetic-token-not-a-purchase")
            .put("purchaseState", 0).put("acknowledged", true).toString(), "synthetic-not-a-store-signature")
        assertEquals(com.android.billingclient.api.Purchase.PurchaseState.PURCHASED, purchase.purchaseState)
        val key = java.security.KeyPairGenerator.getInstance("EC").apply { initialize(java.security.spec.ECGenParameterSpec("secp256r1")) }.generateKeyPair()
        val hash = java.security.MessageDigest.getInstance("SHA-256").digest(purchase.purchaseToken.toByteArray()).joinToString("") { "%02x".format(it) }
        val nonce = "a".repeat(32)
        val payload = org.json.JSONObject().put("version", 1).put("store", "GOOGLE").put("environment", "SANDBOX")
            .put("packageName", context.packageName).put("productId", "synthetic.fixture").put("kind", "lifetime")
            .put("status", "active").put("expires", 0).put("issuedAt", 2000000000L).put("nonce", nonce).put("tokenSha256", hash).toString()
        val signature = java.util.Base64.getEncoder().encodeToString(java.security.Signature.getInstance("SHA256withECDSA").apply { initSign(key.private); update(payload.toByteArray()) }.sign())
        val record = VerifiedGoogleRecord.verify(payload, signature, key.public, context.packageName, mapOf("synthetic.fixture" to "lifetime"), "SANDBOX", nonce, hash)
        var unavailable = 0; var delivered = 0
        lateinit var adapter: PlayBillingAdapter
        adapter = PlayBillingAdapter(context, mapOf("synthetic.fixture" to BillingClient.ProductType.INAPP),
            verify = { _, callback -> callback(record) }, persist = { adapter.close() },
            unavailable = { unavailable++ }, nativeSink = { delivered++ })
        try {
            val accept = PlayBillingAdapter::class.java.getDeclaredMethod("accept", com.android.billingclient.api.Purchase::class.java).apply { isAccessible = true }
            accept.invoke(adapter, purchase)
            assertEquals(0, delivered); assertEquals("Closed adapter must not notify a destroyed native/UI owner", 0, unavailable)
        } finally { adapter.close() }
    }
    @Test fun sdkOwnerConstructsAndClosesWithoutTelemetryRuntimeOrPurchase() {
        try { Class.forName("com.google.android.datatransport.runtime.TransportRuntime"); fail("transport runtime unexpectedly admitted") } catch (_: ClassNotFoundException) { }
        try { Class.forName("com.google.android.gms.location.LocationServices"); fail("location runtime unexpectedly admitted") } catch (_: ClassNotFoundException) { }
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        var verificationCalled = false
        val owner = PlayBillingAdapter(context, mapOf("synthetic.not-queried" to BillingClient.ProductType.INAPP),
            verify = { _, callback -> verificationCalled = true; callback(null) }, persist = { fail("unverified grant") }, unavailable = { })
        owner.close(); owner.close(); assertFalse(verificationCalled)
    }
}
