package ai.pixaura.app

import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.AndroidObservation
import ai.pixaura.bridge.AndroidStaticFacts
import ai.pixaura.bridge.EditorContextV2
import ai.pixaura.bridge.EditorFailure
import android.graphics.Bitmap
import android.os.ParcelFileDescriptor
import java.io.File
import java.io.FileOutputStream
import java.security.KeyStore
import java.util.UUID
import org.junit.Assert.*
import org.junit.Test

/** Synthetic app-private source/key namespace. No store/signing credentials or
 * physical-device qualification. Every assertion invokes the shipping native owner. */
class NativeEditorRuntimePoolV2Test {
    @org.junit.Before fun waitForPriorApplicationOwner() = awaitShippingEditorRetirement()

    @Test fun quotaResetLocalPresentationUsesNativeDayAcrossTimezonesAndDST() {
        for (locale in listOf(java.util.Locale.US, java.util.Locale.forLanguageTag("vi-VN"))) {
            assertEquals("1970-01-02 07:00 (GMT+07:00)", localQuotaReset(0, java.util.TimeZone.getTimeZone("GMT+07:00"), locale))
            assertEquals("1970-01-01 16:00 (GMT-08:00)", localQuotaReset(0, java.util.TimeZone.getTimeZone("GMT-08:00"), locale))
            val zone = java.util.TimeZone.getTimeZone("America/Los_Angeles")
            assertEquals("2024-03-09 16:00 (America/Los_Angeles)", localQuotaReset(19791, zone, locale))
            assertEquals("2024-03-10 17:00 (America/Los_Angeles)", localQuotaReset(19792, zone, locale))
        }
        assertNull(localQuotaReset(-1)); assertNull(localQuotaReset(Long.MAX_VALUE))
    }
    @Test fun freeQuotaPresentationTracksNativeDeliveryAndManualRecovery() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val app = instrumentation.targetContext.applicationContext as PixAuraApplication
        val token = "free-ui-${UUID.randomUUID()}"
        val root = File(app.noBackupFilesDir, token); assertTrue(root.mkdir())
        val pool = NativeEditorRuntimePoolV2(app, File(root, "runtime"), token)
        val store = androidx.lifecycle.ViewModelStore()
        lateinit var model: ManualEditorModel
        fun main(work: () -> Unit) = instrumentation.runOnMainSync { work() }
        fun waitFor(predicate: () -> Boolean) {
            val deadline = android.os.SystemClock.uptimeMillis() + 20_000
            while (!predicate() && android.os.SystemClock.uptimeMillis() < deadline) android.os.SystemClock.sleep(10)
            assertTrue("native-backed model state did not arrive", predicate())
        }
        val input = File(root, "source.png")
        val image = Bitmap.createBitmap(16, 12, Bitmap.Config.ARGB_8888)
        try { image.eraseColor(0xff708090.toInt()); FileOutputStream(input).use { assertTrue(image.compress(Bitmap.CompressFormat.PNG, 100, it)) } } finally { image.recycle() }
        var primaryFailure: Throwable? = null
        try {
            main { model = ManualEditorModel(app, pool); store.put("fixture", model); model.importFile(input) }
            waitFor { model.state.value.core?.revisionCount == 1 && !model.state.value.busy }
            assertEquals(0, model.state.value.commercial?.access); assertEquals(-1L, model.state.value.commercial?.trialStart)
            main { model.setAutoMode(true) }; waitFor { model.autoProposal.value.choices.size == 11 }
            main { model.chooseAuto(0) }
            for (delivered in 1..3) {
                main { model.requestAuto() }; waitFor { model.autoProposal.value.status == AutoStatus.REVIEW_READY && !model.autoProposal.value.inFlight }
                assertEquals(delivered, model.state.value.commercial?.ai)
                assertEquals(1, model.state.value.core?.revisionCount); assertEquals(-1L, model.state.value.commercial?.trialStart)
                main { model.cancelAuto(); model.interrupt() }; waitFor { !model.state.value.busy }
            }
            main { model.requestAuto() }; waitFor { model.autoProposal.value.status == AutoStatus.FAILED && !model.autoProposal.value.inFlight }
            val denied = "Daily AI proposal limit reached. Manual editing remains available."
            assertEquals(denied, model.autoProposal.value.error); assertNull(model.autoProposal.value.review)
            main { model.setAssistMode(true) }; waitFor { model.assistance.value.choices.size == 11 }
            main { model.chooseAdvice(0); model.requestAdvice() }; waitFor { model.assistance.value.status == AssistStatus.FAILED && !model.assistance.value.inFlight }
            assertEquals(denied, model.assistance.value.error); assertNull(model.assistance.value.advice)
            main { model.setAssistMode(false); model.select(4) }; waitFor { model.state.value.core?.active == true && !model.state.value.busy }
            main { model.update(listOf(1000)); model.action(1) }; waitFor { model.state.value.core?.revisionCount == 2 && !model.state.value.busy }
            assertEquals(1, model.state.value.commercial?.access); assertEquals(3, model.state.value.commercial?.ai)
        } catch (failure: Throwable) { primaryFailure = failure; throw failure }
        finally {
            try {
                main { store.clear() }; awaitNativeEditorRetirement(pool); pool.close()
                assertTrue(root.deleteRecursively())
                for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(app.noBackupFilesDir, token + suffix).delete()
                KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry("${app.packageName}.$token")
            } catch (cleanup: Throwable) {
                val original = primaryFailure
                if (original != null) original.addSuppressed(cleanup) else throw cleanup
            }
        }
    }
    @Test fun sharedOwnerClosesIdleAndKeyLossPreservesRecovery() {
        val app = InstrumentationRegistry.getInstrumentation().targetContext
        val token = "pool-test-${UUID.randomUUID()}"
        val root = File(app.noBackupFilesDir, token); assertTrue(root.mkdir())
        val project = File(root, "project"); assertTrue(project.mkdir())
        val input = File(root, "source.png")
        val image = Bitmap.createBitmap(16, 12, Bitmap.Config.ARGB_8888)
        try { image.eraseColor(0xff708090.toInt()); FileOutputStream(input).use { assertTrue(image.compress(Bitmap.CompressFormat.PNG, 100, it)) } } finally { image.recycle() }
        val snapshot = AndroidCapabilitySnapshot(AndroidStaticFacts(8, 36, 8L shl 30, 512L shl 20, 2), AndroidObservation(1, 1, false, 2, 512L shl 20, 1))
        val pool = NativeEditorRuntimePoolV2(app, File(root, "runtime"), token)
        try {
            val first = pool.acquire(snapshot); val second = pool.acquire(snapshot)
            assertSame(first.runtime, second.runtime)
            val oldHandle = first.runtime.native
            val owner = ParcelFileDescriptor.open(input, ParcelFileDescriptor.MODE_READ_ONLY).use { EditorContextV2(first.runtime, project, it, input.length()) }
            try {
                second.close(); owner.select(4); owner.update(listOf(1000)); owner.applyFromHuman()
                assertEquals(2, owner.state().revisionCount)
            } finally { owner.close(); first.close() }
            pool.acquire(snapshot).use { next -> assertNotEquals(oldHandle, next.runtime.native) }
            val ledger = File(root, "runtime/commercial-v2.sqlite"); val before = ledger.readBytes()
            val key = File(app.noBackupFilesDir, "$token.key"); assertTrue(key.delete())
            pool.acquire(snapshot).use { recovery ->
                try { recovery.runtime.commercialState(); fail("key loss granted commercial access") } catch (error: EditorFailure) { assertEquals(3003, error.status) }
                EditorContextV2(recovery.runtime, project).use { saved ->
                    assertEquals(2, saved.state().revisionCount); assertTrue(saved.state().recoveryOnly)
                    try { saved.select(4); fail("key loss granted new editing") } catch (error: EditorFailure) { assertEquals(3003, error.status) }
                    val output = File(root, "recovery.png"); assertTrue(output.createNewFile())
                    ParcelFileDescriptor.open(output, ParcelFileDescriptor.MODE_READ_WRITE).use { assertTrue(saved.exportSavedObserved(it, 2, 0).bytes > 0) }
                }
            }
            assertArrayEquals(before, ledger.readBytes()); assertFalse(key.exists())
            assertTrue(KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.containsAlias("${app.packageName}.$token"))
        } finally {
            pool.close(); assertTrue(root.deleteRecursively())
            for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(app.noBackupFilesDir, token + suffix).delete()
            KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry("${app.packageName}.$token")
        }
    }
}
