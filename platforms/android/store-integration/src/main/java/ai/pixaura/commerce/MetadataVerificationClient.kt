package ai.pixaura.commerce

import ai.pixaura.app.commerce.VerifiedGoogleRecord
import com.android.billingclient.api.Purchase
import org.json.JSONObject
import java.net.URL
import java.nio.charset.StandardCharsets
import java.security.MessageDigest
import java.security.PublicKey
import java.security.SecureRandom
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.ArrayBlockingQueue
import java.util.concurrent.TimeUnit
import java.util.concurrent.RejectedExecutionException
import java.util.concurrent.atomic.AtomicBoolean
import javax.net.ssl.HttpsURLConnection

// Non-shipping metadata client: operator supplies legitimate HTTPS endpoint,
// exact catalog/environment and pinned verifier key. No default URL/account/ID.
// Default platform TLS, no redirects, no logs, no images/usage/voice/history.
class MetadataVerificationClient(private val endpoint: URL, private val key: PublicKey,
    private val packageName: String, private val products: Map<String, String>,
    private val environment: String) : AutoCloseable {
    private val stopped = AtomicBoolean(false)
    private val callbacks = Any()
    private val worker = ThreadPoolExecutor(1, 1, 0, TimeUnit.MILLISECONDS, ArrayBlockingQueue(8))
    private val active = java.util.concurrent.atomic.AtomicReference<HttpsURLConnection?>()
    init {
        require(endpoint.protocol == "https" && endpoint.host.isNotEmpty() && endpoint.userInfo == null && endpoint.query == null && endpoint.ref == null)
        require(environment in setOf("SANDBOX", "PRODUCTION") && products.isNotEmpty() && products.size <= 8)
    }
    fun verify(purchase: Purchase, result: (VerifiedGoogleRecord?) -> Unit) {
        if (stopped.get()) return
        require(purchase.products.size == 1 && purchase.purchaseToken.length in 1..4096)
        val id = purchase.products.single(); val kind = products[id] ?: error("Store product unconfigured")
        try { worker.execute {
            if (stopped.get()) return@execute
            var connection: HttpsURLConnection? = null
            val value = try {
                val bytes = ByteArray(16); SecureRandom().nextBytes(bytes)
                val nonce = bytes.joinToString("") { "%02x".format(it) }
                val hash = MessageDigest.getInstance("SHA-256").digest(purchase.purchaseToken.toByteArray()).joinToString("") { "%02x".format(it) }
                val request = JSONObject().put("productId", id).put("kind", kind).put("purchaseToken", purchase.purchaseToken).put("nonce", nonce).toString().toByteArray(StandardCharsets.UTF_8)
                require(request.size <= 8192)
                connection = endpoint.openConnection() as HttpsURLConnection
                active.set(connection)
                check(!stopped.get())
                connection.instanceFollowRedirects = false; connection.connectTimeout = 10000; connection.readTimeout = 10000
                connection.requestMethod = "POST"; connection.doOutput = true
                connection.setRequestProperty("Content-Type", "application/json"); connection.setRequestProperty("Cache-Control", "no-store")
                connection.setFixedLengthStreamingMode(request.size)
                connection.outputStream.use { it.write(request) }
                require(connection.responseCode == 200)
                val response = connection.inputStream.use { stream ->
                    val output = java.io.ByteArrayOutputStream(); val buffer = ByteArray(1024)
                    while (true) { val count = stream.read(buffer); if (count < 0) break; require(output.size() + count <= 16384); output.write(buffer, 0, count) }
                    JSONObject(output.toString("UTF-8"))
                }
                require(response.keys().asSequence().toSet() == setOf("payload", "signature"))
                VerifiedGoogleRecord.verify(response.getString("payload"), response.getString("signature"), key, packageName, products, environment, nonce, hash)
            } catch (_: Exception) { null } finally { active.compareAndSet(connection, null); connection?.disconnect() }
            synchronized(callbacks) { if (!stopped.get()) result(value) }
        } } catch (_: RejectedExecutionException) { synchronized(callbacks) { if (!stopped.get()) result(null) } }
    }
    override fun close() {
        val close = synchronized(callbacks) { stopped.compareAndSet(false, true) }
        if (close) { active.getAndSet(null)?.disconnect(); worker.shutdownNow() }
    }
}
