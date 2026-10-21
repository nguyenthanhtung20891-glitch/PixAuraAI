package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.UUID
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger
import java.nio.ByteBuffer
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test

// Instrumentation-only access. Lifetime is protected by the paused execution;
// tests resume then join it before closing/destroying the owner.
class AgentPlatformHooks {
    private external fun nativeControl(context: ByteBuffer, command: Int, point: Int): Int
    fun control(editor: ManualEditor, command: Int, point: Int = 0): Int {
        val field = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }
        return nativeControl(field.get(editor) as ByteBuffer, command, point)
    }
}

class AgentPlatformTest {
    private val hooks = AgentPlatformHooks()
    private fun fixture(body: (ManualEditor, File) -> Unit) {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val root = File(instrumentation.targetContext.noBackupFilesDir, "agent-platform-${UUID.randomUUID()}").apply { mkdir() }
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use {
            JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes")
        }
        val bytes = ByteArray(values.length()) { values.getInt(it).toByte() }
        val snapshot = AndroidCapabilitySnapshot(AndroidStaticFacts(8, 36, 8L shl 30, 512L shl 20, 2),
            AndroidObservation(1, 1, false, 2, 512L shl 20, 1))
        try { bytes.inputStream().use { ManualEditor(root.canonicalPath, it, bytes.size.toLong(), snapshot).use { editor -> body(editor, root) } } }
        finally { root.deleteRecursively() }
    }
    private fun plan(editor: ManualEditor, value: Int = 100): ManualEditor.AgentPlan {
        val binding = String(editor.agentBinding(), Charsets.UTF_8).trimEnd('\n')
        val output = editor.referenceAgentPlan(("{\"binding\":$binding,\"goal_id\":\"adjust.exposure\",\"parameters\":{\"milli_ev\":$value},\"structured_intent_version\":1}\n").toByteArray())
        val expected = "{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":$value},\"tool_id\":\"pixaura.exposure\"}],\"binding\":$binding,\"plan_version\":1}\n"
        assertArrayEquals(expected.toByteArray(), output.canonicalBytes())
        return output
    }
    private fun status(expected: Int, body: () -> Unit) {
        try { body(); fail("expected native status $expected") } catch (failure: EditorFailure) { assertEquals(expected, failure.status) }
    }
    @Test fun humanApprovalExactPlanReplayRevocationAndSharedHistory() = fixture { editor, root ->
        val original = editor.state().revisionId; val p = plan(editor)
        assertArrayEquals(p.canonicalBytes(), editor.validateAgentPlan(p.canonicalBytes()).canonicalBytes())
        status(15) { editor.executeAgentPlan(p) }
        editor.approveAgentPlanFromHuman(p).use { approval ->
            assertEquals(1, editor.state().revisionCount)
            status(15) { editor.executeAgentPlan(plan(editor, 200), approval) }
            val receipt = editor.executeAgentPlan(p, approval)
            assertEquals(original, receipt.sourceRevision); assertEquals(64, receipt.planSHA256.length)
            assertEquals(2, editor.state().revisionCount)
            status(15) { editor.executeAgentPlan(p, approval) }
            editor.action(2); assertEquals(original, editor.state().revisionId)
            editor.action(3); assertEquals(receipt.resultingRevision, editor.state().revisionId)
        }
        val fresh = plan(editor)
        val revoked = editor.approveAgentPlanFromHuman(fresh)
        editor.revokeAgentApproval(revoked); status(15) { editor.executeAgentPlan(fresh, revoked) }; revoked.close(); revoked.close()
        editor.select(1); editor.update(listOf(100)); editor.action(1); assertEquals(3, editor.state().revisionCount)
        editor.close(); editor.close()
        ManualEditor(root.canonicalPath).use { assertEquals(3, it.state().revisionCount) }
        status(13) { editor.executeAgentPlan(p) }
    }
    @Test fun malformedAndCrossContextFailClosed() = fixture { editor, _ ->
        for (bytes in listOf(byteArrayOf(), byteArrayOf(0xff.toByte()), "{}\n".toByteArray(), ByteArray(8193))) {
            try { editor.validateAgentPlan(bytes); fail("malformed plan accepted") } catch (_: EditorFailure) { }
            try { editor.referenceAgentPlan(bytes); fail("malformed intent accepted") } catch (_: EditorFailure) { }
        }
        val p = plan(editor)
        val binding = String(editor.agentBinding(), Charsets.UTF_8).trimEnd('\n')
        val prose = "{\"binding\":$binding,\"goal\":\"make this photo brighter\",\"intent_version\":1,\"scope\":\"document\",\"type\":\"edit\"}\n"
        status(5) { editor.referenceAgentPlan(prose.toByteArray()) }
        editor.approveAgentPlanFromHuman(p).use { approval -> fixture { other, _ ->
            status(9) { other.validateAgentPlan(p.canonicalBytes()) }
            status(3) { other.executeAgentPlan(plan(other), approval) }
        } }
        assertEquals(1, editor.state().revisionCount)
    }
    @Test fun actualJniRejectsInvalidDirectStorageEnumsAndLengths() = fixture { editor, _ ->
        val field = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }
        val context = field.get(editor) as ByteBuffer
        val method = ManualEditor::class.java.getDeclaredMethod("nativeAgentBytes", ByteBuffer::class.java,
            Integer.TYPE, ByteArray::class.java, ByteBuffer::class.java).apply { isAccessible = true }
        fun call(c: ByteBuffer, mode: Int, bytes: ByteArray?, out: ByteBuffer) = method.invoke(editor, c, mode, bytes, out) as Int
        assertEquals(1, call(ByteBuffer.allocate(context.capacity()), 1, null, ByteBuffer.allocateDirect(8200)))
        assertEquals(1, call(context, 1, null, ByteBuffer.allocateDirect(8199)))
        assertEquals(1, call(context, 99, null, ByteBuffer.allocateDirect(8200)))
        assertEquals(8, call(context, 2, ByteArray(8193), ByteBuffer.allocateDirect(8200)))
        assertEquals(1, editor.state().revisionCount)
    }
    @Test fun realJniCancellationBusyManualUndoReleaseAndStopFences() {
        for (mode in 0..6) fixture { editor, _ ->
            if (mode == 2) { editor.select(1); editor.update(listOf(50)); editor.action(1) }
            val before = editor.state().revisionCount; val p = plan(editor)
            val a = editor.approveAgentPlanFromHuman(p); val b = editor.approveAgentPlanFromHuman(p)
            try {
                assertEquals(0, hooks.control(editor, 0, if (mode == 5) 3 else 1))
                val result = AtomicInteger(-100); val done = CountDownLatch(1)
                val worker = Thread { try { editor.executeAgentPlan(p, a); result.set(0) } catch (e: EditorFailure) { result.set(e.status) } finally { done.countDown() } }
                worker.start()
                val deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(10)
                while (hooks.control(editor, 1) != 1 && System.nanoTime() < deadline) Thread.sleep(1)
                assertEquals(1, hooks.control(editor, 1))
                status(8) { editor.executeAgentPlan(p, b) }
                when (mode) {
                    0, 5 -> editor.cancelAgentExecution()
                    1 -> { editor.select(1); editor.update(listOf(50)); editor.action(1) }
                    2 -> { editor.action(2); editor.action(3) }
                    3 -> a.close()
                }
                val closer = if (mode == 4) Thread { editor.close() }.also { it.start() } else null
                if (mode == 4) {
                    val until = System.nanoTime() + TimeUnit.SECONDS.toNanos(10)
                    while (System.nanoTime() < until) { try { editor.agentBinding() } catch (e: EditorFailure) { assertEquals(13, e.status); break }; Thread.sleep(1) }
                }
                assertEquals(0, hooks.control(editor, 2))
                assertTrue(done.await(10, TimeUnit.SECONDS)); worker.join(); closer?.join(10000)
                if (mode != 4) {
                    assertEquals(if (mode == 0 || mode == 5) 13 else if (mode == 3 || mode == 6) 0 else 9, result.get())
                    if (mode == 6) editor.cancelAgentExecution()
                    assertEquals(before + if (mode == 1 || mode == 3 || mode == 6) 1 else 0, editor.state().revisionCount)
                    if (mode == 0) { status(15) { editor.executeAgentPlan(p, a) }; editor.executeAgentPlan(p, b) }
                } else { assertFalse(requireNotNull(closer).isAlive); assertEquals(13, result.get()) }
            } finally { a.close(); b.close() }
        }
    }
}
