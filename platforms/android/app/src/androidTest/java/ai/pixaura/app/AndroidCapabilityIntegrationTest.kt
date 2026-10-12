package ai.pixaura.app

import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.AndroidObservation
import ai.pixaura.bridge.AndroidStaticFacts
import ai.pixaura.bridge.CoreProbe
import ai.pixaura.bridge.EditorFailure
import ai.pixaura.bridge.ManualEditor
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import java.io.File
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference
import android.graphics.Bitmap
import android.graphics.Color
import java.io.ByteArrayOutputStream

/** Emulator wiring/synthetic signals only. Never physical GPU/thermal evidence. */
class AndroidCapabilityIntegrationTest {
    private val app get() = InstrumentationRegistry.getInstrumentation().targetContext
    private fun fixture(): ByteArray {
        val json = InstrumentationRegistry.getInstrumentation().context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()) }
        val bytes = json.getJSONObject("png").getJSONArray("bytes")
        return ByteArray(bytes.length()) { bytes.getInt(it).toByte() }
    }
    private inline fun editor(large: Boolean = false, block: (ManualEditor, File) -> Unit) {
        val root = File(app.noBackupFilesDir, "capability-${UUID.randomUUID()}").apply { mkdir() }
        val input = if (large) {
            val bitmap = Bitmap.createBitmap(1024, 512, Bitmap.Config.ARGB_8888)
            try { bitmap.eraseColor(Color.rgb(40, 80, 120)); ByteArrayOutputStream().use { output ->
                check(bitmap.compress(Bitmap.CompressFormat.PNG, 100, output)); output.toByteArray() } }
            finally { bitmap.recycle() }
        } else fixture()
        try { input.inputStream().use { stream -> ManualEditor(root.canonicalPath, stream, input.size.toLong(), if (large) snapshot() else null).use { block(it, root) } } }
        finally { root.walkBottomUp().forEach { it.setWritable(true); it.delete() } }
    }
    private fun snapshot(sequence: Long = 1) = AndroidCapabilitySnapshot(
        AndroidStaticFacts(8, 36, 8L * 1073741824, 512L * 1048576, 2),
        AndroidObservation(1, 1, false, 2, 512L * 1048576, sequence))
    @Test fun livePressureRecoveryParametersCommitCancelAndLateCompletionFence() = editor(large = true) { editor, root ->
        var observation = snapshot().observation
        val initial = editor.executionStrategy(); assertEquals(2, initial.tier)
        assertEquals(256L * 1048576, initial.cap); assertTrue(initial.forcedCpu); assertEquals(0, initial.backend)
        val base = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(base))
        assertEquals(512, base.bitmap.width); assertEquals(256, base.bitmap.height)
        editor.select(editor.tools.single { it.name == "Exposure" }.index); editor.update(listOf(1234))
        val oldRequest = editor.request(); val arrived = CountDownLatch(1); val release = CountDownLatch(1)
        val stale = AtomicReference<ManualEditor.Candidate?>(); val failure = AtomicReference<Throwable?>()
        val worker = Thread {
            try { stale.set(editor.render(oldRequest)); arrived.countDown(); check(release.await(20, TimeUnit.SECONDS)) }
            catch (error: Throwable) { failure.set(error); arrived.countDown() }
        }
        worker.start()
        try {
            assertTrue(arrived.await(20, TimeUnit.SECONDS)); assertNull(failure.get())
            observation = observation.copy(thermal = 2, sequence = 2)
            val elevated = editor.updateCapability(observation); assertEquals(0, elevated.tier); assertEquals(720, elevated.edge)
            observation = observation.copy(thermal = 3, sequence = 3)
            val severe = editor.updateCapability(observation); assertEquals(64L * 1048576, severe.cap)
            assertTrue(severe.epoch > elevated.epoch); assertEquals(listOf(1234), editor.state().values)
            assertTrue(editor.state().active); assertEquals(1, editor.state().revisionCount)
            val latest = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(latest))
            release.countDown(); worker.join(); assertNull(failure.get()); assertFalse(editor.install(requireNotNull(stale.get())))
            assertTrue(requireNotNull(editor.displayedBitmap()).sameAs(latest.bitmap))
            observation = observation.copy(thermal = 1, memory = 4, sequence = 4)
            val critical = editor.updateCapability(observation); assertEquals(360, critical.edge); assertEquals(32L * 1048576, critical.cap)
            val reduced = requireNotNull(editor.render(editor.request())); assertEquals(360, reduced.bitmap.width)
            assertEquals(180, reduced.bitmap.height); assertTrue(editor.install(reduced))
            assertEquals(1, editor.state().revisionCount)
            observation = observation.copy(memory = 1)
            repeat(3) { observation = observation.copy(sequence = observation.sequence + 1)
                val recovered = editor.updateCapability(observation)
                assertEquals(if (it < 2) 0 else 2, recovered.tier)
                assertEquals(if (it < 2) critical.epoch else critical.epoch + 1, recovered.epoch) }
            val stable = editor.executionStrategy()
            assertEquals(stable, editor.updateCapability(observation))
            observation = observation.copy(vulkan = 1, sequence = observation.sequence + 1)
            assertTrue(editor.updateCapability(observation).forcedCpu)
            editor.action(1); assertEquals(2, editor.state().revisionCount)
            editor.select(editor.tools.single { it.name == "Exposure" }.index); editor.update(listOf(-1000))
            observation = observation.copy(constrained = true, sequence = observation.sequence + 1)
            editor.updateCapability(observation); assertTrue(editor.state().active)
            editor.action(0); assertEquals(2, editor.state().revisionCount)
            editor.close()
            ManualEditor(root.canonicalPath).use { reopened ->
                assertEquals(2, reopened.state().revisionCount)
                assertEquals(1L, reopened.configureCapability(snapshot()).epoch)
                assertEquals(512, requireNotNull(reopened.render(reopened.request())).bitmap.width)
            }
        } finally { release.countDown(); worker.join() }
    }
    @Test fun malformedJniAndSyntheticSeamCannotClaimTrustOrRaiseCeilings() = editor { editor, _ ->
        editor.configureCapability(snapshot())
        val storage = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }.get(editor) as ByteBuffer
        val update = ManualEditor::class.java.getDeclaredMethod("nativePolicyUpdate", ByteBuffer::class.java, LongArray::class.java).apply { isAccessible = true }
        val before = editor.executionStrategy()
        for (invalid in listOf(longArrayOf(), longArrayOf(1, -1, 1, 0, 2, Long.MAX_VALUE, 2),
            longArrayOf(1, 5, 1, 0, 2, 512L * 1048576, 2), longArrayOf(2, 1, 1, 0, 2, 512L * 1048576, 2),
            longArrayOf(1, 1, 1, 2, 2, 512L * 1048576, 2), longArrayOf(1, 1, 1, 0, 3, 512L * 1048576, 2))) {
            val result = update.invoke(editor, storage, invalid) as LongArray
            assertTrue(result[0] != 0L); assertEquals(before, editor.executionStrategy())
        }
        var observation = snapshot().observation
        repeat(1000) { observation = observation.copy(thermal = if (it % 2 == 0) 4 else 1, sequence = observation.sequence + 1)
            val strategy = editor.updateCapability(observation)
            assertTrue(strategy.cap <= 256L * 1048576); assertTrue(strategy.forcedCpu); assertEquals(0, strategy.backend) }
        assertEquals(1, editor.state().revisionCount)
        try { editor.updateCapability(observation.copy(sequence = 1)); fail("stale observation admitted") }
        catch (failure: EditorFailure) { assertEquals(9, failure.status) }
    }
    @Test fun actualAndroidProbeIsBoundedLocalAndUncertified() {
        val owner = (app.applicationContext as PixAuraApplication).capabilityOwner
        val snapshot = owner.sample()
        assertTrue(snapshot.facts.cpus in 0..1024); assertTrue(snapshot.facts.physicalBytes >= 0)
        assertTrue(snapshot.facts.processBudgetBytes >= 0); assertTrue(snapshot.observation.thermal in 0..4)
        assertTrue(snapshot.observation.memory in 0..4); assertTrue(CoreProbe().nativeVulkanAvailability() in 0..2)
        // No model, serial, fingerprint, analytics, certification field or persistent profile.
        println("Android local probe: api=${snapshot.facts.api}, cpus=${snapshot.facts.cpus}, ram=${snapshot.facts.physicalBytes}, budget=${snapshot.facts.processBudgetBytes}, vulkan=${snapshot.facts.vulkan}, thermal=${snapshot.observation.thermal}, pressure=${snapshot.observation.memory}; provenance UNOBSERVED, no physical certification claim")
    }
    @Test fun initialPolicyRejectsBeforeDecodeAndPreservesNoDocumentState() {
        val root = File(app.noBackupFilesDir, "initial-admission-${UUID.randomUUID()}").apply { mkdir() }
        val input = fixture(); val facts = snapshot().facts
        val initial = snapshot().copy(observation = snapshot().observation.copy(processBudgetBytes = 1048576))
        try {
            try { input.inputStream().use { ManualEditor(root.canonicalPath, it, input.size.toLong(), initial).close() }
                fail("initial insufficient budget admitted") }
            catch (failure: EditorFailure) { assertEquals(8, failure.status) }
            assertFalse(File(root, "catalog.sqlite").exists())
            input.inputStream().use { ManualEditor(root.canonicalPath, it, input.size.toLong(), AndroidCapabilitySnapshot(facts, snapshot().observation)).use { editor ->
                assertEquals(1, editor.state().revisionCount); assertEquals(2, editor.executionStrategy().tier)
            } }
        } finally { root.walkBottomUp().forEach { it.setWritable(true); it.delete() } }
    }
    @Test fun lowMidHighAndUnknownProfilesUseTheSameSharedRecoveryAndCeilings() {
        val profiles = listOf(
            AndroidStaticFacts(2, 36, 2L * 1073741824, 128L * 1048576, 1),
            AndroidStaticFacts(4, 36, 4L * 1073741824, 256L * 1048576, 2),
            AndroidStaticFacts(8, 36, 8L * 1073741824, 1024L * 1048576, 2),
            AndroidStaticFacts(0, 26, 0, 0, 0))
        profiles.forEachIndexed { index, facts -> editor { editor, _ ->
            var observation = AndroidObservation(if (index == 3) 0 else 1, 1, false, facts.vulkan, facts.processBudgetBytes, 1)
            val base = editor.configureCapability(AndroidCapabilitySnapshot(facts, observation))
            assertEquals(if (index == 3) 0 else index, base.tier)
            assertTrue(base.cap <= 256L * 1048576); assertTrue(base.forcedCpu)
            observation = observation.copy(memory = 4, sequence = 2)
            val lower = editor.updateCapability(observation); assertEquals(0, lower.tier); assertEquals(360, lower.edge)
            repeat(3) { observation = observation.copy(memory = 1, sequence = observation.sequence + 1)
                val next = editor.updateCapability(observation)
                assertTrue(next.cap <= base.cap); assertTrue(next.tier <= base.tier)
                if (index != 3 && it == 2) assertEquals(base.tier, next.tier) }
            assertEquals(1, editor.state().revisionCount)
        } }
    }
    @Test fun pressureCallbacksVsRenderCommitCancelAndOwnerDestruction() = editor { editor, _ ->
        editor.configureCapability(snapshot())
        var sequence = 1L
        repeat(32) { iteration ->
            editor.select(editor.tools.single { it.name == "Exposure" }.index); editor.update(listOf(100 + iteration))
            val before = editor.state().revisionCount; val start = CountDownLatch(1)
            val errors = AtomicReference<Throwable?>()
            val observation = snapshot().observation.copy(memory = 4, vulkan = 1, sequence = ++sequence)
            val update = Thread { try { start.await(); editor.updateCapability(observation) } catch (error: Throwable) { errors.set(error) } }
            update.start(); start.countDown(); editor.action(if (iteration % 2 == 0) 1 else 0); update.join()
            assertNull(errors.get()); assertEquals(before + if (iteration % 2 == 0) 1 else 0, editor.state().revisionCount)
            val request = editor.request()
            val pending = Thread { try { editor.render(request) } catch (error: EditorFailure) {
                if (error.status != 13) errors.set(error)
            } catch (error: Throwable) { errors.set(error) } }
            pending.start()
            editor.updateCapability(observation.copy(thermal = 3, sequence = ++sequence)); pending.join()
            assertNull(errors.get())
        }
        val done = AtomicReference<Throwable?>()
        val update = Thread { try { editor.updateCapability(snapshot().observation.copy(constrained = true, sequence = ++sequence)) }
            catch (error: IllegalStateException) { /* closed owner rejects before JNI */ }
            catch (error: Throwable) { done.set(error) } }
        update.start(); editor.close(); update.join(); assertNull(done.get())
    }
}
