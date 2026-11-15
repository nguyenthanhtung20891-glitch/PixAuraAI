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
