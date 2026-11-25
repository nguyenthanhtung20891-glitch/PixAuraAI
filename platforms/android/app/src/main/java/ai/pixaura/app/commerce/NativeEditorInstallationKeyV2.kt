package ai.pixaura.app.commerce

import android.content.Context
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.AtomicFile
import java.io.File
import java.io.RandomAccessFile
import java.nio.file.Files
import java.security.KeyStore
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

/** App-owned native receipt secret, distinct from store/signing credentials.
 * Never resets an unreadable existing value. Key loss is a recovery-only state,
 * not evidence authorizing a new trial, entitlement, edit or receipt. */
internal class NativeEditorInstallationKeyV2(context: Context, namespace: String = "native-receipt-v2") {
    private val root = context.noBackupFilesDir.canonicalFile
    private val alias: String
    private val file: AtomicFile
    private val lockFile: File
    init {
        require(namespace.matches(Regex("[a-zA-Z0-9.-]{1,100}")))
        alias = "${context.packageName}.$namespace"
        file = AtomicFile(File(root, "$namespace.key"))
        lockFile = File(root, "$namespace.lock")
    }
    fun readOrCreate(allowCreate: Boolean = true): ByteArray = synchronized(processLock) {
        require(!Files.isSymbolicLink(lockFile.toPath()))
        RandomAccessFile(lockFile, "rw").channel.use { channel ->
            channel.lock().use {
                val backup = File(file.baseFile.path + ".bak")
                val pending = File(file.baseFile.path + ".new")
                for (path in listOf(file.baseFile, backup, pending)) require(!Files.isSymbolicLink(path.toPath()))
                val store = KeyStore.getInstance("AndroidKeyStore").apply { load(null) }
                if (file.baseFile.exists() || backup.exists()) {
                    val key = store.getKey(alias, null) as? SecretKey ?: error("Native receipt key unavailable")
                    // AtomicFile restores a committed backup before opening. Never
                    // replace that installation secret after an interrupted write.
                    val bytes = file.openRead().use { input ->
                        val data = ByteArray(60); var count = 0
                        while (count < data.size) { val n = input.read(data, count, data.size - count); require(n > 0); count += n }
                        require(input.read() == -1); data
                    }
                    val cipher = Cipher.getInstance("AES/GCM/NoPadding")
                    cipher.init(Cipher.DECRYPT_MODE, key, GCMParameterSpec(128, bytes.copyOfRange(0, 12)))
                    cipher.updateAAD(alias.toByteArray(Charsets.UTF_8))
                    return@synchronized cipher.doFinal(bytes.copyOfRange(12, bytes.size)).also { require(it.size == 32) }
                }
                require(allowCreate) { "Native receipt key is unavailable; recovery remains available" }
                require(!pending.exists()) { "Native receipt key creation was interrupted" }
                require(!store.containsAlias(alias)) { "Native receipt key file is unavailable" }
                val key = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore").run {
                    init(KeyGenParameterSpec.Builder(alias, KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
                        .setBlockModes(KeyProperties.BLOCK_MODE_GCM).setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE).build())
                    generateKey()
                }
                val secret = ByteArray(32).also { SecureRandom().nextBytes(it) }
                try {
                    val cipher = Cipher.getInstance("AES/GCM/NoPadding")
                    cipher.init(Cipher.ENCRYPT_MODE, key); require(cipher.iv.size == 12)
                    cipher.updateAAD(alias.toByteArray(Charsets.UTF_8))
                    val bytes = cipher.iv + cipher.doFinal(secret); require(bytes.size == 60)
                    val output = file.startWrite()
                    try { output.write(bytes); file.finishWrite(output) }
                    catch (error: Exception) { file.failWrite(output); throw error }
                    return@synchronized secret
                } catch (error: Exception) { secret.fill(0); throw error }
            }
        }
        @Suppress("UNREACHABLE_CODE") error("Native receipt key unavailable")
    }
    private companion object { val processLock = Any() }
}
