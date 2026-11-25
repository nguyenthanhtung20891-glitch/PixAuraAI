package ai.pixaura.app.commerce

import org.json.JSONObject
import java.nio.charset.StandardCharsets
import java.security.AlgorithmParameters
import java.security.MessageDigest
import java.security.KeyFactory
import java.security.spec.X509EncodedKeySpec
import java.security.spec.ECGenParameterSpec
import java.security.spec.ECParameterSpec
import java.security.PublicKey
import java.security.Signature
import java.security.interfaces.ECPublicKey
import java.util.Base64

// Authenticated metadata only. Does not grant native edits or enforce quotas.
class VerifiedGoogleRecord private constructor(val payload: String, val signature: String, val productId: String, val status: String, val issuedAt: Long, val tokenSha256: String,
    val verifierKeySha256: String, val packageName: String, val environment: String, val kind: String, val expires: Long, val nonce: String) {
    companion object {
        internal fun canonicalVerificationKey(key: PublicKey): ECPublicKey {
            require(key is ECPublicKey)
            val parameters = AlgorithmParameters.getInstance("EC").apply { init(ECGenParameterSpec("secp256r1")) }.getParameterSpec(ECParameterSpec::class.java)
            require(key.params.curve == parameters.curve && key.params.generator == parameters.generator &&
                key.params.order == parameters.order && key.params.cofactor == parameters.cofactor)
            val material = requireNotNull(key.encoded); require(material.isNotEmpty() && material.size <= 4096)
            val encodedKey = material.copyOf()
            val canonicalKey = KeyFactory.getInstance("EC").generatePublic(X509EncodedKeySpec(encodedKey)) as ECPublicKey
            require(canonicalKey.w == key.w && canonicalKey.params.curve == parameters.curve &&
                canonicalKey.params.generator == parameters.generator && canonicalKey.params.order == parameters.order && canonicalKey.params.cofactor == parameters.cofactor)
            return canonicalKey
        }
        fun verify(payload: String, signature: String, key: PublicKey, packageName: String,
                   products: Map<String, String>, environment: String,
                   nonce: String, tokenSha256: String, minimumIssuedAt: Long = 0): VerifiedGoogleRecord {
            require(payload.toByteArray(StandardCharsets.UTF_8).size <= 4096 && signature.length <= 256)
            val canonicalKey = canonicalVerificationKey(key)
            // Verify and fingerprint the SAME defensively decoded key. A custom
            // PublicKey object cannot advertise one identity and verify another.
            val keyDigest = MessageDigest.getInstance("SHA-256").digest(canonicalKey.encoded).joinToString("") { "%02x".format(it.toInt() and 255) }
            val checker = Signature.getInstance("SHA256withECDSA")
            checker.initVerify(canonicalKey); checker.update(payload.toByteArray(StandardCharsets.UTF_8))
            require(checker.verify(Base64.getDecoder().decode(signature)))
            val j = JSONObject(payload)
            val fields = setOf("version", "store", "environment", "packageName", "productId", "kind", "status", "expires", "issuedAt", "nonce", "tokenSha256")
            require(j.keys().asSequence().toSet() == fields)
            fun string(name: String): String { val v = j.get(name); require(v is String); return v }
            fun number(name: String): Long { val v = j.get(name); require(v is Int || v is Long); return (v as Number).toLong() }
            require(number("version") == 1L && string("store") == "GOOGLE")
            require(environment in setOf("SANDBOX", "PRODUCTION") && string("environment") == environment)
            require(string("packageName") == packageName && products[string("productId")] == string("kind"))
            require(string("kind") in setOf("subscription", "lifetime") && string("status") in setOf("active", "expired", "revoked", "pending", "inactive"))
            require(nonce.matches(Regex("[a-f0-9]{32}")) && string("nonce") == nonce)
            require(tokenSha256.matches(Regex("[a-f0-9]{64}")) && string("tokenSha256") == tokenSha256)
            require(number("issuedAt") in minimumIssuedAt..8640000000000L && number("expires") in 0..8640000000000L)
            require(string("kind") != "lifetime" || number("expires") == 0L)
            return VerifiedGoogleRecord(payload, signature, string("productId"), string("status"), number("issuedAt"), string("tokenSha256"), keyDigest, packageName, environment, string("kind"), number("expires"), nonce)
        }
    }
}
