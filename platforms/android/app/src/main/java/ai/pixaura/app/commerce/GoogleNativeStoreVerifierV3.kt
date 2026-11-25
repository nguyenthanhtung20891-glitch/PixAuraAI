package ai.pixaura.app.commerce

import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import java.security.KeyFactory
import java.security.spec.X509EncodedKeySpec
import java.util.Base64

import ai.pixaura.bridge.StoreConfigurationV3
import ai.pixaura.bridge.StoreVerifiedMetadataV3
import ai.pixaura.bridge.StoreVerifierV3
import java.security.MessageDigest
import java.security.PublicKey
import java.util.Collections
import java.util.TreeMap
import org.json.JSONArray
import org.json.JSONObject

/** Optional metadata-only verifier boundary. No Billing SDK, network request,
 * backend endpoint or product configuration is invented/activated here. The
 * actual approved deployment supplies the immutable key/catalog before binding. */
internal class GoogleNativeStoreVerifierV3(key: PublicKey, application: String,
    products: Map<String,String>, private val environment: String) : StoreVerifierV3 {
    private val key = VerifiedGoogleRecord.canonicalVerificationKey(key)
    private val products = Collections.unmodifiableMap(TreeMap(products))
    override val configuration: StoreConfigurationV3
    init {
        require(environment in setOf("SANDBOX","PRODUCTION") && this.products.size in 1..8)
        require(this.products.values.all { it in setOf("subscription","lifetime") })
        configuration = StoreConfigurationV3(application,digest(this.key.encoded),2,if(environment=="SANDBOX")1 else 2,
            this.products.mapValues { if(it.value=="subscription")1 else 2 })
    }
    internal fun verifiedRecord(proof: ByteArray, origin: Int, nativeNonce: String): VerifiedGoogleRecord {
        require(proof.size in 1..16384 && origin in 1..2)
        val envelope = JSONArray(proof.decodeToString(throwOnInvalidSequence=true))
        require(envelope.length()==2 && envelope.get(0) is String && envelope.get(1) is String)
        val payload=envelope.getString(0); val signature=envelope.getString(1)
        val metadata=JSONObject(payload)
        val expected=if(origin==1)nativeNonce else metadata.getString("nonce")
        require(origin==1 || nativeNonce.isEmpty())
        val record=VerifiedGoogleRecord.verify(payload,signature,key,configuration.application,products,environment,
            expected,metadata.getString("tokenSha256"))
        require(record.verifierKeySha256==configuration.keyIdentity && record.status in setOf("active","expired","revoked"))
        return record
    }
    override fun verifySignedProof(proof: ByteArray, origin: Int, nativeNonce: String): StoreVerifiedMetadataV3 {
        val record=verifiedRecord(proof,origin,nativeNonce)
        val identity="PIXAURA_GOOGLE_TRANSACTION_V3\n${configuration.application}\n$environment\n${record.tokenSha256}"
        return StoreVerifiedMetadataV3(record.productId,digest(identity.toByteArray(Charsets.UTF_8)),record.verifierKeySha256,
            if(record.kind=="subscription")1 else 2,when(record.status){"active"->1;"expired"->2;else->3},record.issuedAt,0,record.expires)
    }
    companion object {
        fun fromConfiguredApplication(context:Context):GoogleNativeStoreVerifierV3? {
            val info=if(Build.VERSION.SDK_INT>=33)context.packageManager.getApplicationInfo(context.packageName,PackageManager.ApplicationInfoFlags.of(PackageManager.GET_META_DATA.toLong()))
                else { @Suppress("DEPRECATION") context.packageManager.getApplicationInfo(context.packageName,PackageManager.GET_META_DATA) }
            val raw=info.metaData?.getString("ai.pixaura.store.v3") ?: return null
            require(raw.toByteArray(Charsets.UTF_8).size<=4096)
            val value=JSONObject(raw)
            require(value.keys().asSequence().toSet()==setOf("version","environment","publicKeySpki","products") && value.get("version")==3)
            val material=Base64.getDecoder().decode(value.getString("publicKeySpki"));require(material.size in 1..4096)
            val key=KeyFactory.getInstance("EC").generatePublic(X509EncodedKeySpec(material))
            val configured=value.getJSONObject("products");val products=configured.keys().asSequence().associateWith { id ->
                val kind=configured.get(id);require(kind is String);kind
            }
            return GoogleNativeStoreVerifierV3(key,context.packageName,products,value.getString("environment"))
        }
        fun envelope(payload:String,signature:String):ByteArray = JSONArray().put(payload).put(signature).toString().toByteArray(Charsets.UTF_8)
            .also { require(it.size in 1..16384) }
        private fun digest(bytes:ByteArray)=MessageDigest.getInstance("SHA-256").digest(bytes).joinToString(""){"%02x".format(it.toInt() and 255)}
    }
}
