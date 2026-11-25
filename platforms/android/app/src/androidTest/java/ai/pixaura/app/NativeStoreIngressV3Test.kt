package ai.pixaura.app

import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.app.commerce.GoogleNativeStoreOwnerV3
import ai.pixaura.app.commerce.GoogleNativeStoreVerifierV3
import ai.pixaura.app.commerce.ProtectedCommercialCache
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.AndroidStaticFacts
import ai.pixaura.bridge.AndroidObservation
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import java.io.File
import java.security.KeyPairGenerator
import java.security.KeyStore
import java.security.MessageDigest
import java.security.Signature
import java.security.spec.ECGenParameterSpec
import java.util.Base64
import java.util.UUID

/** Real P-256 synthetic signature/JNI/native/Keystore tests, NOT Google Play
 * sandbox transactions, backend admission or physical-device qualification. */
class NativeStoreIngressV3Test {
    @Before fun retirePriorOwner()=awaitShippingEditorRetirement()
    @Test fun signedNativeDeliveryCacheRepairRevocationAndReopenKeepOneAuthority() {
        val context=InstrumentationRegistry.getInstrumentation().targetContext
        val token="store-v3-${UUID.randomUUID()}"
        val root=File(context.noBackupFilesDir,token);assertTrue(root.mkdir())
        val keys=KeyPairGenerator.getInstance("EC").apply{initialize(ECGenParameterSpec("secp256r1"))}.generateKeyPair()
        val product="test.synthetic.lifetime"
        val verifier=GoogleNativeStoreVerifierV3(keys.public,context.packageName,mapOf(product to "lifetime"),"SANDBOX")
        val pool=NativeEditorRuntimePoolV2(context,File(root,"runtime"),token,verifier)
        val snapshot=AndroidCapabilitySnapshot(AndroidStaticFacts(8,36,8L shl 30,512L shl 20,2),AndroidObservation(1,1,false,2,512L shl 20,1))
        val owner=GoogleNativeStoreOwnerV3(context,verifier,pool)
        val scope="${context.packageName}\n1\n${verifier.configuration.keyIdentity}\n$product"
        val cacheName="store-v3."+digest(scope.toByteArray(Charsets.UTF_8))
        val cacheFile=File(context.noBackupFilesDir,"$cacheName.cache")
        val cache=ProtectedCommercialCache(context,cacheName)
        fun payload(nonce:String,status:String,at:Long)=JSONObject().put("version",1).put("store","GOOGLE").put("environment","SANDBOX")
            .put("packageName",context.packageName).put("productId",product).put("kind","lifetime").put("status",status)
            .put("expires",0).put("issuedAt",at).put("nonce",nonce).put("tokenSha256","b".repeat(64)).toString()
        fun sign(body:String)=Base64.getEncoder().encodeToString(Signature.getInstance("SHA256withECDSA").apply{initSign(keys.private);update(body.toByteArray(Charsets.UTF_8))}.sign())
        var lease:NativeEditorRuntimePoolV2.Lease?=null
        try {
            lease=pool.acquire(snapshot);assertEquals(0,lease.runtime.commercialState().access)
            val request=owner.begin(snapshot);val body=payload(request.nonce,"active",2000000000)
            val receipt=request.accept(body,sign(body))
            assertEquals(product,receipt.product);assertEquals(64,receipt.identity.length)
            assertEquals(3,lease.runtime.commercialState().access);assertEquals(-1L,lease.runtime.commercialState().trialStart)
            assertEquals("active",cache.read(keys.public,mapOf(product to "lifetime"),"SANDBOX").single().status)
            val original=cacheFile.readBytes()
            val failed=owner.begin(snapshot)
            try{failed.accept(payload(failed.nonce,"active",2000000000),"invalid");fail("unsigned proof accepted")}catch(_:Exception){}
            assertEquals(3,lease.runtime.commercialState().access)
            val cancelled=owner.begin(snapshot);cancelled.close()
            try{val p=payload(cancelled.nonce,"active",2000000000);cancelled.accept(p,sign(p));fail("cancelled request accepted")}catch(_:IllegalStateException){}
            cacheFile.writeBytes(byteArrayOf(1,2,3))
            val repair=owner.begin(snapshot);val repairBody=payload(repair.nonce,"active",2000000000)
            val repaired=repair.accept(repairBody,sign(repairBody));assertTrue(repaired.replay);assertEquals(receipt.identity,repaired.identity)
            val latest=cacheFile.readBytes()
            // Old ciphertext remains decryptable: corruption repair preserved the
            // existing app-cache AES key rather than resetting it.
            cacheFile.writeBytes(original)
            assertEquals("active",cache.read(keys.public,mapOf(product to "lifetime"),"SANDBOX").single().status)
            cacheFile.writeBytes(latest)
            val revoke=owner.begin(snapshot);val revoked=payload(revoke.nonce,"revoked",2000000001)
            revoke.accept(revoked,sign(revoked));assertEquals(0,lease.runtime.commercialState().access)
            cacheFile.writeBytes(original) // A valid older cache is never native authority.
            lease.close();lease=null
            lease=pool.acquire(snapshot);assertEquals(0,lease.runtime.commercialState().access);assertEquals(-1L,lease.runtime.commercialState().trialStart)
            val replay=owner.begin(snapshot)
            try{val old=payload(replay.nonce,"active",2000000001);replay.accept(old,sign(old));fail("same-generation revocation reversal accepted")}catch(_:Exception){}
            assertEquals(0,lease.runtime.commercialState().access)
            val authoritative=owner.begin(snapshot);val newer=payload(authoritative.nonce,"active",2000000002)
            authoritative.accept(newer,sign(newer));assertEquals(3,lease.runtime.commercialState().access)
            assertEquals(-1L,lease.runtime.commercialState().trialStart)
        } finally {
            lease?.close();pool.close();cache.clear();root.deleteRecursively()
            File(context.noBackupFilesDir,"$token.key").delete();File(context.noBackupFilesDir,"$token.lock").delete()
            KeyStore.getInstance("AndroidKeyStore").apply{load(null);deleteEntry("${context.packageName}.$token")}
        }
    }
    @Test fun nativeStoreChangesRefreshTheOpenModelWithoutPublishingOrClearingItsGesture() {
        val instrumentation=InstrumentationRegistry.getInstrumentation()
        val app=instrumentation.targetContext.applicationContext as PixAuraApplication
        val token="store-model-v3-${UUID.randomUUID()}";val root=File(app.noBackupFilesDir,token);assertTrue(root.mkdir())
        val keys=KeyPairGenerator.getInstance("EC").apply{initialize(ECGenParameterSpec("secp256r1"))}.generateKeyPair()
        val product="test.synthetic.model-lifetime"
        val verifier=GoogleNativeStoreVerifierV3(keys.public,app.packageName,mapOf(product to "lifetime"),"SANDBOX")
        val pool=NativeEditorRuntimePoolV2(app,File(root,"runtime"),token,verifier)
        val service=GoogleNativeStoreOwnerV3(app,verifier,pool)
        val source=File(root,"source.png");val bitmap=android.graphics.Bitmap.createBitmap(16,12,android.graphics.Bitmap.Config.ARGB_8888)
        try{bitmap.eraseColor(0xff708090.toInt());java.io.FileOutputStream(source).use{assertTrue(bitmap.compress(android.graphics.Bitmap.CompressFormat.PNG,100,it))}}finally{bitmap.recycle()}
        val store=androidx.lifecycle.ViewModelStore();lateinit var model:ManualEditorModel
        fun main(action:()->Unit)=instrumentation.runOnMainSync{action()}
        fun waitFor(predicate:()->Boolean){val end=android.os.SystemClock.uptimeMillis()+20000;while(!predicate()&&android.os.SystemClock.uptimeMillis()<end)android.os.SystemClock.sleep(10);assertTrue("native-backed commercial state did not arrive",predicate())}
        fun deliver(status:String,at:Long){
            service.begin(app.capabilityOwner.sample()).use{request->
                val body=JSONObject().put("version",1).put("store","GOOGLE").put("environment","SANDBOX").put("packageName",app.packageName)
                    .put("productId",product).put("kind","lifetime").put("status",status).put("expires",0).put("issuedAt",at)
                    .put("nonce",request.nonce).put("tokenSha256","c".repeat(64)).toString()
                val signature=Base64.getEncoder().encodeToString(Signature.getInstance("SHA256withECDSA").apply{initSign(keys.private);update(body.toByteArray(Charsets.UTF_8))}.sign())
                request.accept(body,signature)
            }
        }
        var primary:Throwable?=null
        try{
            main{model=ManualEditorModel(app,pool);store.put("model",model);model.importFile(source)}
            waitFor{model.state.value.core?.revisionCount==1&&!model.state.value.busy}
            assertEquals(0,model.state.value.commercial?.access);assertEquals(-1L,model.state.value.commercial?.trialStart)
            deliver("active",2000000000);waitFor{model.state.value.commercial?.access==3}
            assertEquals(1,model.state.value.core?.revisionCount);assertEquals(-1L,model.state.value.commercial?.trialStart)
            main{model.select(4)};waitFor{model.state.value.core?.active==true&&!model.state.value.busy}
            deliver("revoked",2000000001);waitFor{model.state.value.commercial?.access==0}
            assertTrue(model.state.value.core?.active==true);assertEquals(1,model.state.value.core?.revisionCount)
            assertEquals(-1L,model.state.value.commercial?.trialStart);assertFalse(model.state.value.busy)
        }catch(error:Throwable){primary=error;throw error}
        finally{
            main{store.clear()}
            try{awaitNativeEditorRetirement(pool);pool.close()}catch(cleanup:Throwable){if(primary!=null)primary.addSuppressed(cleanup)else throw cleanup}
            val scope="${app.packageName}\n1\n${verifier.configuration.keyIdentity}\n$product"
            ProtectedCommercialCache(app,"store-v3."+digest(scope.toByteArray(Charsets.UTF_8))).clear()
            root.deleteRecursively();File(app.noBackupFilesDir,"$token.key").delete();File(app.noBackupFilesDir,"$token.lock").delete()
            KeyStore.getInstance("AndroidKeyStore").apply{load(null);deleteEntry("${app.packageName}.$token")}
        }
    }
    private fun digest(bytes:ByteArray)=MessageDigest.getInstance("SHA-256").digest(bytes).joinToString(""){"%02x".format(it.toInt() and 255)}
}
