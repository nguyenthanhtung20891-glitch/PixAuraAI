package ai.pixaura.app.commerce

import android.content.Context
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.AtomicFile
import org.json.JSONObject
import org.json.JSONArray
import java.io.File
import java.nio.charset.StandardCharsets
import java.nio.file.Files
import java.security.KeyStore
import java.security.PublicKey
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

// Background-worker component, dormant until store/admission configuration exists.
// AES-GCM is corruption/tamper resistance, not a claim of perfect offline rollback defense.
class ProtectedCommercialCache(context: Context, namespace: String = "commercial-v1") {
    private val packageName = context.packageName
    private val alias: String
    private val file: AtomicFile
    private val aad: ByteArray
    init {
        require(namespace.matches(Regex("[a-zA-Z0-9.-]{1,100}")))
        alias = "$packageName.$namespace"
        val root = context.noBackupFilesDir.canonicalFile
        val path = File(root, "$namespace.cache")
        require(!Files.isSymbolicLink(path.toPath()))
        file = AtomicFile(path); aad = alias.toByteArray(StandardCharsets.UTF_8)
    }
    @Synchronized fun write(record: VerifiedGoogleRecord) {
        if (record.status == "pending") return // Pending never erases a prior verified entitlement.
        val records = if (file.baseFile.exists()) envelopes().toMutableList() else mutableListOf()
        val previous = records.firstOrNull { JSONObject(it.getString("payload")).getString("productId") == record.productId }
        if (previous != null) {
            val old = JSONObject(previous.getString("payload")); require(record.issuedAt >= old.getLong("issuedAt"))
            require(!(record.issuedAt == old.getLong("issuedAt") && old.getString("tokenSha256") == record.tokenSha256 && old.getString("status") in setOf("revoked", "inactive") && record.status == "active"))
        }
        records.removeAll { JSONObject(it.getString("payload")).getString("productId") == record.productId }
        records.add(JSONObject().put("payload", record.payload).put("signature", record.signature))
        require(records.size <= 8)
        val data = JSONObject().put("version", 1).put("records", JSONArray(records)).toString().toByteArray(StandardCharsets.UTF_8)
        require(data.size <= 65536)
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, key()); cipher.updateAAD(aad)
        val bytes = cipher.iv + cipher.doFinal(data)
        val stream = file.startWrite()
        try { stream.write(bytes); file.finishWrite(stream) } catch (e: Exception) { file.failWrite(stream); throw e }
    }
    @Synchronized fun read(key: PublicKey, products: Map<String, String>, environment: String): List<VerifiedGoogleRecord> {
        if (!file.baseFile.exists()) return emptyList()
        return envelopes().map { j ->
            val payload = j.getString("payload"); val parsed = JSONObject(payload)
            VerifiedGoogleRecord.verify(payload, j.getString("signature"), key, packageName, products, environment, parsed.getString("nonce"), parsed.getString("tokenSha256"))
        }
    }
    // Valid cached lifetime facts are not erased by another product's refresh.
    // Unreadable/key-lost state can recover only from a signature-verified current fact.
    @Synchronized fun repairAfterStoreVerification(record: VerifiedGoogleRecord) {
        val damaged = if (file.baseFile.exists()) try { envelopes(); false } catch (_: Exception) { true } else false
        if (damaged) clear()
        write(record)
    }
    private fun envelopes(): List<JSONObject> {
        require(!Files.isSymbolicLink(file.baseFile.toPath()) && file.baseFile.length() in 28..65564)
        val bytes = file.openRead().use { stream ->
            val output = java.io.ByteArrayOutputStream(); val buffer = ByteArray(1024)
            while (true) { val n = stream.read(buffer); if (n < 0) break; require(output.size() + n <= 65564); output.write(buffer, 0, n) }
            output.toByteArray()
        }; require(bytes.size in 28..65564)
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.DECRYPT_MODE, existingKey(), GCMParameterSpec(128, bytes.copyOfRange(0, 12))); cipher.updateAAD(aad)
        val root = JSONObject(String(cipher.doFinal(bytes.copyOfRange(12, bytes.size)), StandardCharsets.UTF_8))
        require(root.keys().asSequence().toSet() == setOf("version", "records") && root.get("version") == 1)
        val rows = root.getJSONArray("records"); require(rows.length() <= 8)
        val values = (0 until rows.length()).map { rows.getJSONObject(it) }
        require(values.all { it.keys().asSequence().toSet() == setOf("payload", "signature") })
        require(values.map { JSONObject(it.getString("payload")).getString("productId") }.toSet().size == values.size)
        return values
    }
    @Synchronized fun clear() {
        file.delete(); val store = KeyStore.getInstance("AndroidKeyStore"); store.load(null); store.deleteEntry(alias)
    }
    private fun existingKey(): SecretKey {
        val store = KeyStore.getInstance("AndroidKeyStore"); store.load(null)
        return store.getKey(alias, null) as? SecretKey ?: error("Commercial cache unavailable; recovery remains available")
    }
    private fun key(): SecretKey {
        val store = KeyStore.getInstance("AndroidKeyStore"); store.load(null)
        (store.getKey(alias, null) as? SecretKey)?.let { return it }
        val generator = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore")
        generator.init(KeyGenParameterSpec.Builder(alias, KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
            .setBlockModes(KeyProperties.BLOCK_MODE_GCM).setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
            .setRandomizedEncryptionRequired(true).build())
        return generator.generateKey()
    }
}
