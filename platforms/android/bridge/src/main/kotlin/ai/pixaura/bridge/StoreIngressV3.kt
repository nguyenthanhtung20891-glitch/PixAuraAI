package ai.pixaura.bridge

import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.Collections
import java.util.TreeMap

/** Private application configuration. Never load a verifier/key/catalog from a
 * purchase response, UI input or cache. The configured adapter verifies actual
 * store/backend signatures; this bridge does not provide a permissive verifier. */
internal class StoreConfigurationV3(val application: String, val keyIdentity: String,
    val store: Int, val environment: Int, products: Map<String, Int>) {
    val products: Map<String, Int> = Collections.unmodifiableMap(TreeMap(products))
    fun encoded(): ByteArray {
        require(products.size in 1..8)
        val fields = listOf("PIXAURA_STORE_CONFIGURATION_V3", store.toString(), environment.toString(), application, keyIdentity,
            products.size.toString()) + products.flatMap { listOf(it.key, it.value.toString()) }
        require(fields.all { it.isNotEmpty() && '\n' !in it && '\u0000' !in it })
        return (fields.joinToString("\n") + "\n").toByteArray(Charsets.UTF_8).also { require(it.size <= 2048) }
    }
}
internal data class StoreVerifiedMetadataV3(val product: String, val transactionDigest: String, val verifiedKeyIdentity: String,
    val kind: Int, val status: Int, val signedSeconds: Long, val signedMillisecond: Int, val expires: Long)
internal interface StoreVerifierV3 {
    val configuration: StoreConfigurationV3
    /** origin1 fresh requires the native nonce; origin2 verifies cached signed
     * proof without creating a new freshness/UTC claim. No native reentrancy. */
    fun verifySignedProof(proof: ByteArray, origin: Int, nativeNonce: String): StoreVerifiedMetadataV3
}
internal class StoreAcknowledgmentV3 private constructor(val product: String, val identity: String, val proofDigest: String, val replay: Boolean) {
    companion object {
        fun commit(runtime: EditorRuntimeV2, proof: ByteArray, nonce: String): StoreAcknowledgmentV3 {
            require(proof.size in 1..16384)
            val out=ByteBuffer.allocateDirect(276).order(ByteOrder.nativeOrder())
            val status=NativeEditorV2.storeAccept(runtime.native,proof,nonce.toByteArray(Charsets.UTF_8),out)
            if(status!=0)throw EditorFailure(status,"Verified store ingress")
            runtime.storeDidCommitV3()
            return fromNative(out)
        }
        private fun fromNative(bytes: ByteBuffer): StoreAcknowledgmentV3 {
            check(bytes.getInt(0) == 3 && bytes.getInt(4) == 276)
            fun text(at: Int, count: Int): String {
                val copy = bytes.duplicate(); copy.position(at); val out = ByteArray(count); copy.get(out)
                return out.takeWhile { it != 0.toByte() }.toByteArray().toString(Charsets.UTF_8)
            }
            return StoreAcknowledgmentV3(text(16,129),text(145,65),text(210,65),bytes.getInt(8)!=0)
        }
    }
}
/** Native retains precisely this callback owner, releasing it with the shared
 * runtime. This object must not retain that runtime and create an ownership cycle. */
internal class StoreNativeCallbackV3(private val verifier: StoreVerifierV3) {
    val configuration = verifier.configuration
    @Suppress("unused") // Invoked through the explicit JNI method signature.
    fun verifyForNativeV3(proof: ByteArray, origin: Int, nonce: ByteArray): Array<String> {
        require(proof.size in 1..16384 && origin in 1..2 && nonce.size <= 32)
        val value = verifier.verifySignedProof(proof, origin, nonce.toString(Charsets.UTF_8))
        return arrayOf(value.product, value.transactionDigest, value.verifiedKeyIdentity, value.kind.toString(), value.status.toString(),
            value.signedSeconds.toString(),value.signedMillisecond.toString(),value.expires.toString())
    }
}
internal fun EditorRuntimeV2.configureStoreV3(verifier: StoreVerifierV3) {
    val callback = StoreNativeCallbackV3(verifier)
    val status = NativeEditorV2.storeBind(native, callback.configuration.encoded(), callback)
    if (status != 0) throw EditorFailure(status, "Verified store configuration")
    storeDidCommitV3()
}
internal fun EditorRuntimeV2.beginStoreRequestV3(): String {
    val out = ByteBuffer.allocateDirect(33).order(ByteOrder.nativeOrder())
    val status = NativeEditorV2.storeBegin(native,out)
    if (status != 0) throw EditorFailure(status,"Verified store request")
    val bytes=ByteArray(32);out.get(bytes);return bytes.toString(Charsets.UTF_8)
}
internal fun EditorRuntimeV2.cancelStoreRequestV3(nonce: String) {
    val status=NativeEditorV2.storeCancel(native,nonce.toByteArray(Charsets.UTF_8))
    if(status!=0)throw EditorFailure(status,"Cancel verified store request")
}
internal fun EditorRuntimeV2.acceptStoreProofV3(proof: ByteArray, nonce: String): StoreAcknowledgmentV3 =
    StoreAcknowledgmentV3.commit(this,proof,nonce)
