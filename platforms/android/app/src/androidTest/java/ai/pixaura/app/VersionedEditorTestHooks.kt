package ai.pixaura.app

import ai.pixaura.bridge.ManualEditor

/** Debug instrumentation only. Versioned handles are never treated as v1 direct
 * buffers. Both paths exercise their real provider/validator/publication code. */
internal object VersionedEditorTestHooks {
    private external fun nativeControl(owner: Long, family: Int, command: Int, value: Int): Int
    private fun context(editor: ManualEditor): Any? = ManualEditor::class.java.getDeclaredField("versioned").apply { isAccessible = true }.get(editor)
    fun control(editor: ManualEditor, family: Int, command: Int, value: Int): Int? {
        val owner = context(editor) ?: return null
        val handle = owner.javaClass.getDeclaredField("native").apply { isAccessible = true }.getLong(owner)
        return nativeControl(handle, family, command, value)
    }
    fun staleComparisonCopy(editor: ManualEditor, comparison: ManualEditor.ReviewComparison): Int? {
        val owner = context(editor) ?: return null
        val pair = comparison.javaClass.getDeclaredField("v2Pair").apply { isAccessible = true }.get(comparison)
        val before = pair.javaClass.getDeclaredMethod("getBefore").apply { isAccessible = true }.invoke(pair)
        val handle = before.javaClass.getDeclaredField("handle").apply { isAccessible = true }.getLong(before)
        val native = owner.javaClass.getDeclaredField("native").apply { isAccessible = true }.getLong(owner)
        val output = java.nio.ByteBuffer.allocateDirect(comparison.before.width * comparison.before.height * 4)
        for (i in 0 until output.capacity()) output.put(i, 0x5a.toByte())
        val bridge = Class.forName("ai.pixaura.bridge.NativeEditorV2")
        val target = bridge.getDeclaredField("INSTANCE").get(null)
        val copy = bridge.getDeclaredMethod("frame", java.lang.Long.TYPE, java.lang.Long.TYPE, Integer.TYPE, java.nio.ByteBuffer::class.java)
        val status = copy.invoke(target, native, handle, 0, output) as Int
        org.junit.Assert.assertEquals(9, status)
        for (i in 0 until output.capacity()) org.junit.Assert.assertEquals(0x5a.toByte(), output.get(i))
        return status
    }
    fun candidate(editor: ManualEditor, bytes: ByteArray): ManualEditor.AutoProposal? {
        val owner = context(editor) ?: return null
        val begin = owner.javaClass.getDeclaredMethod("beginPlanCandidate", ByteArray::class.java).apply { isAccessible = true }
        val task = begin.invoke(owner, bytes)
        val run = owner.javaClass.declaredMethods.single { it.name == "runAdvisory" }.apply { isAccessible = true }
        val advice = run.invoke(owner, task)
        val present = ManualEditor::class.java.declaredMethods.single { it.name == "v2Proposal" }.apply { isAccessible = true }
        return present.invoke(editor, advice) as ManualEditor.AutoProposal
    }
}

/** Independent native-owner tests cannot coexist with a prior activity's live
 * display reservations. Await real resource retirement; never force native
 * release or recycle a bitmap that still has a consumer. */
internal fun awaitShippingEditorRetirement() {
    val app = androidx.test.platform.app.InstrumentationRegistry.getInstrumentation().targetContext.applicationContext as PixAuraApplication
    awaitNativeEditorRetirement(app.editorRuntimes)
}
internal fun awaitNativeEditorRetirement(pool: NativeEditorRuntimePoolV2) {
    val runtime = NativeEditorRuntimePoolV2::class.java.getDeclaredField("runtime").apply { isAccessible = true }
    val leases = NativeEditorRuntimePoolV2::class.java.getDeclaredField("leases").apply { isAccessible = true }
    val gate = checkNotNull(NativeEditorRuntimePoolV2::class.java.getDeclaredField("gate").apply { isAccessible = true }.get(pool))
    val deadline = android.os.SystemClock.elapsedRealtime() + 5000
    while (android.os.SystemClock.elapsedRealtime() < deadline) {
        if (synchronized(gate) { leases.getInt(pool) == 0 && runtime.get(pool) == null }) return
        System.gc(); System.runFinalization(); Thread.sleep(20)
    }
    org.junit.Assert.fail("Previous editor/display resources did not retire; refusing a second native runtime")
}
