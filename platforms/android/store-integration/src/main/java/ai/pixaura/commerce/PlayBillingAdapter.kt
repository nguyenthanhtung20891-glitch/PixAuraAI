package ai.pixaura.commerce

import android.app.Activity
import android.content.Context
import ai.pixaura.app.commerce.VerifiedGoogleRecord
import ai.pixaura.app.commerce.VerifiedNativeStoreHandoffV1
import com.android.billingclient.api.AcknowledgePurchaseParams
import com.android.billingclient.api.BillingClient
import com.android.billingclient.api.BillingClientStateListener
import com.android.billingclient.api.BillingFlowParams
import com.android.billingclient.api.BillingResult
import com.android.billingclient.api.PendingPurchasesParams
import com.android.billingclient.api.ProductDetails
import com.android.billingclient.api.Purchase
import com.android.billingclient.api.QueryPurchasesParams
import com.android.billingclient.api.QueryProductDetailsParams
import java.util.concurrent.atomic.AtomicBoolean
import java.security.MessageDigest

// Non-shipping SDK adapter. Billing SDK runtime/privacy admission and real products
// are external gates. Backend must return signature-verified, token-bound metadata;
// a Purchase/PURCHASED callback is NEVER a grant by itself.
class PlayBillingAdapter(context: Context, private val products: Map<String, String>,
    private val verify: (Purchase, (VerifiedGoogleRecord?) -> Unit) -> Unit,
    private val persist: (VerifiedGoogleRecord) -> Unit, private val unavailable: () -> Unit,
    private val nativeSink: ((VerifiedNativeStoreHandoffV1) -> Unit)? = null) : AutoCloseable {
    private val closed = AtomicBoolean(false)
    private val handoffGate = Any()
    private val client: BillingClient
    init {
        require(products.isNotEmpty() && products.size <= 8 && products.keys.all { it.isNotEmpty() && it.length <= 128 } && products.values.all { it in setOf(BillingClient.ProductType.SUBS, BillingClient.ProductType.INAPP) })
        client = BillingClient.newBuilder(context.applicationContext)
            .enablePendingPurchases(PendingPurchasesParams.newBuilder().enableOneTimeProducts().build())
            .enableAutoServiceReconnection()
            .setListener { result, purchases -> if (result.responseCode == BillingClient.BillingResponseCode.OK) purchases.orEmpty().forEach(::accept) }
            .build()
    }
    fun connect(ready: (Boolean) -> Unit) {
        if (closed.get()) { ready(false); return }
        client.startConnection(object : BillingClientStateListener {
            override fun onBillingSetupFinished(result: BillingResult) { synchronized(handoffGate) { if (!closed.get()) ready(result.responseCode == BillingClient.BillingResponseCode.OK) } }
            override fun onBillingServiceDisconnected() { synchronized(handoffGate) { if (!closed.get()) ready(false) } }
        })
    }
    fun queryCatalog(type: String, callback: (List<ProductDetails>?) -> Unit) {
        require(type in setOf(BillingClient.ProductType.INAPP, BillingClient.ProductType.SUBS))
        if (closed.get() || !client.isReady) { callback(null); return }
        val configured = products.filterValues { it == type }.keys
        if (configured.isEmpty()) { callback(emptyList()); return }
        val params = QueryProductDetailsParams.newBuilder().setProductList(configured.map {
            QueryProductDetailsParams.Product.newBuilder().setProductId(it).setProductType(type).build()
        }).build()
        client.queryProductDetailsAsync(params) { result, response ->
            if (!closed.get()) callback(if (result.responseCode == BillingClient.BillingResponseCode.OK) response.productDetailsList else null)
        }
    }
    fun restore() {
        if (closed.get()) return
        if (!client.isReady) { unavailable(); return }
        for (kind in listOf(BillingClient.ProductType.INAPP, BillingClient.ProductType.SUBS)) {
            client.queryPurchasesAsync(QueryPurchasesParams.newBuilder().setProductType(kind).build()) { result, purchases ->
                if (!closed.get() && result.responseCode == BillingClient.BillingResponseCode.OK) purchases.forEach(::accept)
            }
        }
    }
    fun purchase(activity: Activity, details: ProductDetails, approvedOfferToken: String? = null): BillingResult {
        check(!closed.get() && client.isReady && details.productId in products)
        require((products[details.productId] == BillingClient.ProductType.SUBS) == (approvedOfferToken != null))
        val selection = BillingFlowParams.ProductDetailsParams.newBuilder().setProductDetails(details)
        approvedOfferToken?.let { token ->
            require(details.subscriptionOfferDetails.orEmpty().any { it.offerToken == token }); selection.setOfferToken(token)
        }
        return client.launchBillingFlow(activity, BillingFlowParams.newBuilder().setProductDetailsParamsList(listOf(selection.build())).build())
    }
    private fun accept(purchase: Purchase) {
        if (closed.get() || purchase.purchaseState != Purchase.PurchaseState.PURCHASED ||
            purchase.products.size != 1 || purchase.products[0] !in products) return
        verify(purchase) verification@{ record ->
            if (!closed.get() && record != null && record.productId == purchase.products[0]) {
                val hash = MessageDigest.getInstance("SHA-256").digest(purchase.purchaseToken.toByteArray()).joinToString("") { "%02x".format(it) }
                if (record.tokenSha256 != hash || (purchase.isSuspended && record.status == "active")) return@verification
                try {
                    synchronized(handoffGate) {
                        if (closed.get()) return@verification
                        persist(record)
                        if (closed.get()) return@verification // Persistence may reentrantly retire the owner.
                        nativeSink?.invoke(VerifiedNativeStoreHandoffV1.fresh(record)) // Signature-verified object only; never Purchase or a paid UI flag.
                    }
                } catch (_: Exception) { synchronized(handoffGate) { if (!closed.get()) unavailable() }; return@verification } // No acknowledgment without durable cache.
                synchronized(handoffGate) {
                    if (!closed.get() && !purchase.isAcknowledged && record.status == "active") client.acknowledgePurchase(
                        AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchase.purchaseToken).build()) { /* store retry reconciles on restore */ }
                }
            }
        }
    }
    override fun close() { val stop = synchronized(handoffGate) { closed.compareAndSet(false, true) }; if (stop) client.endConnection() }
}
