package ai.pixaura.app.commerce

import android.content.Context
import ai.pixaura.app.NativeEditorRuntimePoolV2
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.StoreAcknowledgmentV3
import ai.pixaura.bridge.acceptStoreProofV3
import ai.pixaura.bridge.beginStoreRequestV3
import ai.pixaura.bridge.cancelStoreRequestV3
import java.security.MessageDigest
import java.util.concurrent.atomic.AtomicBoolean

/** App-owned metadata-only lifecycle. Uses the SAME pool as editing; never
 * opens another native publisher/SQLite owner. Real backend/store configuration
 * remains external. No networking, billing activation or purchases occur here. */
internal class GoogleNativeStoreOwnerV3(context: Context,private val verifier: GoogleNativeStoreVerifierV3,
    private val runtimes: NativeEditorRuntimePoolV2) {
    private val context=context.applicationContext
    init{require(verifier.configuration.application==this.context.packageName)}
    fun begin(snapshot: AndroidCapabilitySnapshot): Request {
        val lease=runtimes.acquire(snapshot)
        try{return Request(lease,lease.runtime.beginStoreRequestV3())}
        catch(error:Throwable){try{lease.close()}catch(cleanup:Throwable){error.addSuppressed(cleanup)};throw error}
    }
    inner class Request internal constructor(private val lease: NativeEditorRuntimePoolV2.Lease,val nonce:String): AutoCloseable {
        private val finished=AtomicBoolean(false)
        fun accept(payload:String,signature:String):StoreAcknowledgmentV3 {
            check(finished.compareAndSet(false,true))
            var failure:Throwable?=null
            try{
                val proof=GoogleNativeStoreVerifierV3.envelope(payload,signature)
                val receipt=lease.runtime.acceptStoreProofV3(proof,nonce)
                val record=verifier.verifiedRecord(proof,1,nonce)
                val scope="${verifier.configuration.application}\n${verifier.configuration.environment}\n${verifier.configuration.keyIdentity}\n${record.productId}"
                // Per-product isolation preserves an unrelated verified lifetime
                // if one cache is damaged. Native journal remains the authority.
                val cache=ProtectedCommercialCache(context,"store-v3."+digest(scope.toByteArray(Charsets.UTF_8)))
                cache.repairAfterNativeCommit(record,receipt,digest(proof))
                return receipt
            }catch(error:Throwable){failure=error;throw error}
            finally{
                try{lease.runtime.cancelStoreRequestV3(nonce)}catch(cleanup:Throwable){failure?.addSuppressed(cleanup)}
                try{lease.close()}catch(cleanup:Throwable){if(failure!=null)failure.addSuppressed(cleanup)else throw cleanup}
            }
        }
        override fun close(){
            if(finished.compareAndSet(false,true))try{lease.runtime.cancelStoreRequestV3(nonce)}finally{lease.close()}
        }
    }
    private fun digest(bytes:ByteArray)=MessageDigest.getInstance("SHA-256").digest(bytes).joinToString(""){"%02x".format(it.toInt() and 255)}
}
