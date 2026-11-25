package ai.pixaura.bridge

import android.graphics.Bitmap
import android.os.ParcelFileDescriptor
import java.io.File
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.concurrent.atomic.AtomicBoolean
import org.json.JSONArray
import org.json.JSONObject

internal object NativeEditorV2 {
    init { System.loadLibrary("pixaura_core"); System.loadLibrary("pixaura_bridge") }
    external fun runtimeCreate(path: ByteArray, key: ByteArray, facts: LongArray?, observation: LongArray?, output: ByteBuffer): Int
    external fun runtimeClose(owner: Long): Int
    external fun runtimeRecovery(path: ByteArray, facts: LongArray?, observation: LongArray?, output: ByteBuffer): Int
    external fun runtimePolicy(owner: Long, observation: LongArray?): LongArray?
    external fun commercial(owner: Long, output: ByteBuffer): Int
    external fun storeBind(owner: Long, configuration: ByteArray, verifier: StoreNativeCallbackV3): Int
    external fun storeBegin(owner: Long, output: ByteBuffer): Int
    external fun storeCancel(owner: Long, nonce: ByteArray): Int
    external fun storeAccept(owner: Long, proof: ByteArray, nonce: ByteArray, output: ByteBuffer): Int
    external fun open(runtime: Long, path: ByteArray, fd: Int, bytes: Long, migrate: Boolean, output: ByteBuffer): Int
    external fun editIdentity(owner: Long, output: ByteBuffer): Int
    external fun state(owner: Long, output: ByteBuffer): Int
    external fun action(owner: Long, kind: Int, value: Int): Int
    external fun update(owner: Long, values: IntArray): Int
    external fun preview(owner: Long, width: Int, height: Int, kind: Int, reference: ByteArray?, output: ByteBuffer): Int
    external fun history(owner: Long, output: ByteBuffer): Int
    external fun agentBytes(owner: Long, mode: Int, input: ByteArray?, output: ByteBuffer): Int
    external fun planBegin(owner: Long, input: ByteArray, output: ByteBuffer): Int
    external fun frame(owner: Long, frame: Long, kind: Int, output: ByteBuffer?): Int
    external fun exportTask(owner: Long, kind: Int, ticket: Long, format: Int, quality: Int, fd: Int, output: ByteBuffer?): Int
    external fun exportObserved(owner: Long, format: Int, quality: Int, fd: Int, output: ByteBuffer): Int
    external fun export(owner: Long, format: Int, quality: Int, fd: Int, output: ByteBuffer): Int
    external fun inspectReview(owner: Long, id: ByteArray, output: ByteBuffer): Int
    external fun reviewCommand(owner: Long, id: ByteArray, command: Int, action: Int, parameter: Int, value: Int, destination: Int, output: ByteBuffer): Int
    external fun advisory(owner: Long, kind: Int, choice: Int, input: ByteArray?, content: ByteArray?, output: ByteBuffer): Int
    external fun content(owner: Long, id: ByteArray, operations: Boolean, output: ByteBuffer): Int
    external fun pending(owner: Long, output: ByteBuffer): Int
    external fun reviewPreview(owner: Long, kind: Int, ticket: Long, id: ByteArray?, width: Int, height: Int, output: ByteBuffer?): Int
    external fun requestTicket(owner: Long, kind: Int, ticket: Long, choice: Int, output: ByteBuffer?): Int
    external fun humanApply(owner: Long, id: ByteArray): Int
    external fun humanApplyReceipt(owner: Long, id: ByteArray, output: ByteBuffer): Int
    external fun cancelAdvisory(owner: Long, id: ByteArray): Int
}
private fun record(size: Int) = ByteBuffer.allocateDirect(size).order(ByteOrder.nativeOrder())
private fun checkedV2(status: Int, boundary: String) { if (status != 0) throw EditorFailure(status, boundary) }
private fun ByteBuffer.textAt(offset: Int, size: Int): String {
    val bytes = ByteArray(size); val copy = duplicate(); copy.position(offset); copy.get(bytes)
    return bytes.takeWhile { it != 0.toByte() }.toByteArray().toString(Charsets.UTF_8)
}

/** Observation of an actual native COMMIT; never an input granting Apply. */
internal data class NativeExportObservationV2(val width: Int, val height: Int, val format: Int,
    val bytes: Long, val epoch: Long, val source: String, val document: String, val revision: String)

internal data class NativePublicationV2(val kind: Int, val epoch: Long,
    val logicalUtc: Long, val clockUncertain: Boolean, val authenticatedUtc: Boolean,
    val event: String, val source: String, val project: String, val document: String,
    val parent: String, val revision: String, val digest: String)

data class CommercialStateV2(val access: Int, val clockUncertain: Boolean,
    val photos: Int, val ai: Int, val pending: Int, val day: Long, val logicalUtc: Long,
    val trialStart: Long, val trialEnd: Long)

/** One app-owned runtime, shared by all documents. Native selects the CPU budget
 * from trusted OS observations; this bridge never grants a tier or paid access.
 * Call on platform workers. Close documents and frames before close(). */
internal class EditorRuntimeV2 private constructor(root: File, protectedInstallationKey: ByteArray?,
    capability: AndroidCapabilitySnapshot?, recoveryOnly: Boolean) : AutoCloseable {
    constructor(root: File, protectedInstallationKey: ByteArray, capability: AndroidCapabilitySnapshot? = null) : this(root, protectedInstallationKey, capability, false)
    companion object {
        fun recovery(root: File, capability: AndroidCapabilitySnapshot? = null) = EditorRuntimeV2(root, null, capability, true)
    }
    val native: Long
    private val closed = AtomicBoolean(false)
    private val storeObserversV3 = java.util.concurrent.CopyOnWriteArrayList<() -> Unit>()
    private val policyGate = Any()
    private val retirementObserver = java.util.concurrent.atomic.AtomicReference<(() -> Unit)?>(null)
    internal fun observeFrameRetirement(callback: () -> Unit) { check(retirementObserver.compareAndSet(null, callback)) }
    internal fun frameRetired() { retirementObserver.get()?.invoke() }
    private var observationSequence = capability?.observation?.sequence ?: 0L
    init {
        val out = record(8)
        val path = root.canonicalPath.toByteArray(Charsets.UTF_8)
        val status = if (recoveryOnly) NativeEditorV2.runtimeRecovery(path, capability?.facts?.nativeWords(), capability?.observation?.nativeWords(), out)
            else NativeEditorV2.runtimeCreate(path, requireNotNull(protectedInstallationKey), capability?.facts?.nativeWords(), capability?.observation?.nativeWords(), out)
        checkedV2(status, if (recoveryOnly) "Recovery owner" else "Commercial owner")
        native = out.getLong(0)
    }
    fun strategy(): ExecutionStrategy = ExecutionStrategy.fromNative(checkNotNull(NativeEditorV2.runtimePolicy(native, null)))
    fun updateCapability(observation: AndroidObservation): ExecutionStrategy = synchronized(policyGate) {
        if (observation.sequence <= observationSequence) return@synchronized strategy()
        val current = ExecutionStrategy.fromNative(checkNotNull(NativeEditorV2.runtimePolicy(native, observation.nativeWords())))
        observationSequence = observation.sequence; current
    }
    fun observeStoreChangesV3(observer: () -> Unit): AutoCloseable = synchronized(storeObserversV3) {
        check(!closed.get() && storeObserversV3.size < 8); storeObserversV3.add(observer)
        AutoCloseable { storeObserversV3.remove(observer) }
    }
    fun storeDidCommitV3() {
        for(observer in storeObserversV3)try{observer()}catch(_:Exception){/* UI observation cannot invalidate a committed store fact. */}
    }
    fun commercialState(): CommercialStateV2 {
        val out = record(64); checkedV2(NativeEditorV2.commercial(native, out), "Commercial state")
        check(out.getInt(0) == 2 && out.getInt(4) == 64)
        return CommercialStateV2(out.getInt(8), out.getInt(12) != 0, out.getInt(16), out.getInt(20), out.getInt(24),
            out.getLong(32), out.getLong(40), out.getLong(48), out.getLong(56))
    }
    override fun close() {
        if (!closed.compareAndSet(false, true)) return
        val status = NativeEditorV2.runtimeClose(native)
        if (status != 0) { closed.set(false); checkedV2(status, "Commercial close") }
        storeObserversV3.clear()
    }
}
internal data class EditorStateV2(val revision: String, val revisionCount: Int, val selected: Int,
    val epoch: Long, val generation: Long, val pending: Boolean, val canUndo: Boolean,
    val canRedo: Boolean, val recoveryOnly: Boolean, val commercialUnavailable: Boolean, val values: List<Int>)
internal data class NativeReviewParameterV2(val name: String, val unit: String, val raw: Int, val minimum: Int, val maximum: Int, val neutral: Int)
internal data class NativeReviewActionV2(val index: Int, val toolIndex: Int, val toolId: String, val label: String,
    val operationVersion: Int, val parameterVersion: Int, val geometry: Boolean, val parameters: List<NativeReviewParameterV2>)
internal data class NativeReviewInspectionV2(val providerId: String, val epoch: Long, val warnings: Int, val actions: List<NativeReviewActionV2>)
internal data class AdvisoryV2(val id: String, val delivery: String, val choice: Int, val tool: Int, val epoch: Long)

/** Explicit worker-owned frames recycle only after their caller retires every
 * consumer. App UI transfer is one-way: Bitmap references then own the native
 * consumer reservation through a bounded phantom queue, never early recycle. */
internal enum class FrameCurrencyV2 { CURRENT, BUSY, STALE, UNAVAILABLE }

internal class EditorFrameV2(private val runtime: EditorRuntimeV2, private val editor: Long, private val handle: Long,
    bitmap: Bitmap, val sourceWidth: Int, val sourceHeight: Int) : AutoCloseable {
    private val closed = AtomicBoolean(false)
    private val pixels = Any()
    private var owned: Bitmap? = bitmap
    private var displayed: java.lang.ref.WeakReference<Bitmap>? = null
    val bitmap: Bitmap get() = synchronized(pixels) { owned ?: displayed?.get() ?: throw EditorFailure(13, "Retired image") }
    val isCurrent: Boolean get() = !closed.get() && NativeEditorV2.frame(editor, handle, 1, null) == 0
    internal fun currency(wait: Boolean): FrameCurrencyV2 {
        if (closed.get()) return FrameCurrencyV2.STALE
        val out = record(4)
        if (NativeEditorV2.frame(editor, handle, if (wait) 4 else 3, out) != 0) return FrameCurrencyV2.UNAVAILABLE
        return when (out.getInt(0)) { 0 -> FrameCurrencyV2.CURRENT; 1 -> FrameCurrencyV2.BUSY; 2 -> FrameCurrencyV2.STALE; else -> FrameCurrencyV2.UNAVAILABLE }
    }
    fun transferToDisplay(retired: () -> Unit): Bitmap = synchronized(pixels) {
        if (closed.get()) throw EditorFailure(13, "Retired image")
        val value = owned ?: return@synchronized displayed?.get() ?: throw EditorFailure(13, "Retired image")
        val weak = java.lang.ref.WeakReference(value)
        BitmapLifetimeV2.register(value, runtime, editor, handle) { closed.set(true); retired() }
        displayed = weak; owned = null; value
    }
    override fun close() = synchronized(pixels) {
        if (closed.compareAndSet(false, true) && displayed == null) {
            owned?.recycle(); owned = null
            checkedV2(NativeEditorV2.frame(editor, handle, 2, null), "Preview release")
            runtime.frameRetired()
        }
        Unit
    }
}

/** Versioned synchronous bridge. Native owns all publication and commercial
 * semantics. UI must serialize check/install and stop -> join -> close lifecycle.
 * No v1 handle, restored proposal, quota or paid state grants Human Apply. */
internal class EditorContextV2(private val runtime: EditorRuntimeV2, root: File,
    input: ParcelFileDescriptor? = null, bytes: Long = 0, explicitlyMigrateSchema3: Boolean = false) : AutoCloseable {
    private var native: Long
    private val lock = Any()
    init {
        val out = record(8)
        checkedV2(NativeEditorV2.open(runtime.native, root.canonicalPath.toByteArray(Charsets.UTF_8),
            input?.fd ?: -1, bytes, explicitlyMigrateSchema3, out), "Open")
        native = out.getLong(0)
    }
    private fun owner(): Long = synchronized(lock) { if (native == 0L) throw EditorFailure(13, "Closed editor"); native }
    fun state(): EditorStateV2 {
        val out = record(112); checkedV2(NativeEditorV2.state(owner(), out), "State")
        check(out.getInt(0) == 2 && out.getInt(4) == 112)
        val count = out.getInt(52); check(count in 0..4)
        return EditorStateV2(out.textAt(72, 33), out.getInt(8), out.getInt(12), out.getLong(16), out.getLong(24),
            out.getInt(32) != 0, out.getInt(36) != 0, out.getInt(40) != 0, out.getInt(44) != 0,
            out.getInt(48) != 0, (0 until count).map { out.getInt(56 + it * 4) })
    }
    fun editIdentity(): String {
        val out = record(33); checkedV2(NativeEditorV2.editIdentity(owner(), out), "Gesture identity")
        return out.textAt(0, 33)
    }
    fun history(): JSONObject {
        val output = record(1048576); val size = NativeEditorV2.history(owner(), output)
        if (size < 0) throw EditorFailure(-size, "Ordinary history")
        check(size <= 1048576); val bytes = ByteArray(size).also { output.get(it) }
        return JSONObject(bytes.toString(Charsets.UTF_8)).also { check(it.getInt("version") == 2 && it.getJSONArray("revisions").length() <= 4096) }
    }
    private fun action(kind: Int, value: Int = 0) = checkedV2(NativeEditorV2.action(owner(), kind, value), "Edit/history")
    fun select(tool: Int) = action(6, tool)
    fun update(values: List<Int>) = checkedV2(NativeEditorV2.update(owner(), values.toIntArray()), "Parameter")
    fun applyFromHuman() = action(1)
    fun cancel() = action(0)
    fun undo() = action(2)
    fun redo() = action(3)
    fun preview(width: Int = 512, height: Int = 512, original: Boolean = false, advice: AdvisoryV2? = null, revision: String? = null): EditorFrameV2 {
        require(advice == null || revision == null)
        val owner = owner(); val out = record(40)
        val edge = try { runtime.strategy().edge } catch (_: EditorFailure) { 720 }
        checkedV2(NativeEditorV2.preview(owner, minOf(width, edge), minOf(height, edge), if (advice != null) 2 else if (revision != null) 3 else if (original) 1 else 0,
            (advice?.id ?: revision)?.toByteArray(Charsets.UTF_8), out), "Preview")
        return decodeFrame(out, owner)
    }
    private fun decodeFrame(out: ByteBuffer, owner: Long): EditorFrameV2 {
        val frame = out.getLong(32)
        try {
            check(out.getInt(0) == 2 && out.getInt(4) == 40)
            val w = out.getInt(8); val h = out.getInt(12); val bytes = out.getLong(24)
            check(w in 1..1024 && h in 1..1024 && bytes == w.toLong() * h * 4)
            val pixels = record(bytes.toInt()); checkedV2(NativeEditorV2.frame(owner, frame, 0, pixels), "Preview copy")
            val argb = IntArray(w * h) { i -> val at = i * 4
                ((pixels.get(at + 3).toInt() and 255) shl 24) or ((pixels.get(at).toInt() and 255) shl 16) or
                    ((pixels.get(at + 1).toInt() and 255) shl 8) or (pixels.get(at + 2).toInt() and 255) }
            val bitmap = Bitmap.createBitmap(argb, w, h, Bitmap.Config.ARGB_8888)
            return EditorFrameV2(runtime, owner, frame, bitmap, out.getInt(16), out.getInt(20))
        } catch (failure: Throwable) { NativeEditorV2.frame(owner, frame, 2, null); throw failure }
    }
    inner class ReviewPreview internal constructor(private val editor: Long, private val ticket: Long) : AutoCloseable {
        private val closed = AtomicBoolean(false)
        fun cancel() { check(!closed.get()); checkedV2(NativeEditorV2.reviewPreview(editor, 2, ticket, null, 0, 0, null), "Cancel review preview") }
        fun render(): ReviewPair {
            if (closed.get()) throw EditorFailure(13, "Review preview cancelled")
            val out = record(80)
            checkedV2(NativeEditorV2.reviewPreview(editor, 1, ticket, null, 0, 0, out), "Review preview")
            val beforeRecord = out.duplicate().order(ByteOrder.nativeOrder())
            val afterRecord = out.duplicate().apply { position(40) }.slice().order(ByteOrder.nativeOrder())
            val before = try { decodeFrame(beforeRecord, editor) }
                catch (error: Throwable) { NativeEditorV2.frame(editor, afterRecord.getLong(32), 2, null); throw error }
            val after = try { decodeFrame(afterRecord, editor) }
                catch (error: Throwable) { before.close(); throw error }
            return ReviewPair(this, before, after)
        }
        override fun close() { if (closed.compareAndSet(false, true)) NativeEditorV2.reviewPreview(editor, 3, ticket, null, 0, 0, null) }
    }
    class ReviewPair internal constructor(private val task: ReviewPreview, val before: EditorFrameV2, val after: EditorFrameV2) : AutoCloseable {
        val isCurrent: Boolean get() = before.isCurrent && after.isCurrent
        override fun close() { before.close(); after.close() }
    }
    fun beginReviewPreview(advice: AdvisoryV2, width: Int = 512, height: Int = 512): ReviewPreview {
        val current = owner(); val out = record(8)
        val edge = try { runtime.strategy().edge } catch (_: EditorFailure) { 720 }
        checkedV2(NativeEditorV2.reviewPreview(current, 0, 0, advice.id.toByteArray(Charsets.UTF_8), minOf(width, edge), minOf(height, edge), out), "Begin review preview")
        return ReviewPreview(current, out.getLong(0))
    }
    private fun advisory(kind: Int, choice: Int = 0, input: String? = null, content: ByteArray? = null): AdvisoryV2 {
        val out = record(128)
        checkedV2(NativeEditorV2.advisory(owner(), kind, choice, input?.toByteArray(Charsets.UTF_8), content, out), "AI proposal")
        check(out.getInt(0) == 2 && out.getInt(4) == 128)
        return AdvisoryV2(out.textAt(24, 33), out.textAt(57, 65), out.getInt(8), out.getInt(12), out.getLong(16))
    }
    fun agentBytes(mode: Int, input: ByteArray? = null): ByteArray {
        val output = record(8192); val size = NativeEditorV2.agentBytes(owner(), mode, input, output)
        if (size < 0) throw EditorFailure(-size, "Agent data")
        if (size !in 1..8192) throw EditorFailure(14, "Agent data")
        return ByteArray(size).also { output.get(it) }
    }
    fun beginPlanCandidate(plan: ByteArray): AdvisoryRequest {
        val current = owner(); val output = record(8)
        checkedV2(NativeEditorV2.planBegin(current, plan, output), "Begin Plan candidate")
        return AdvisoryRequest(current, output.getLong(0))
    }
    class AdvisoryRequest internal constructor(internal val editor: Long, internal val ticket: Long)
    fun beginAdvisory(choice: Int): AdvisoryRequest {
        val current = owner(); val out = record(8)
        checkedV2(NativeEditorV2.requestTicket(current, 0, 0, choice, out), "Begin AI proposal")
        return AdvisoryRequest(current, out.getLong(0))
    }
    fun cancelAdvisoryRequest(request: AdvisoryRequest) {
        val current = owner(); checkedV2(if (current == request.editor) 0 else 9, "AI request owner")
        checkedV2(NativeEditorV2.requestTicket(current, 1, request.ticket, 0, null), "Cancel AI request")
    }
    fun runAdvisory(request: AdvisoryRequest): AdvisoryV2 {
        val current = owner(); checkedV2(if (current == request.editor) 0 else 9, "AI request owner")
        val out = record(128)
        checkedV2(NativeEditorV2.requestTicket(current, 2, request.ticket, 0, out), "AI proposal")
        check(out.getInt(0) == 2 && out.getInt(4) == 128)
        return AdvisoryV2(out.textAt(24, 33), out.textAt(57, 65), out.getInt(8), out.getInt(12), out.getLong(16))
    }
    fun requestAdvisory(choice: Int) = advisory(0, choice)
    fun restoreAdvisory(delivery: String) = advisory(1, input = delivery)
    fun pendingAdvisories(): List<String> {
        val output = record(8192); val size = NativeEditorV2.pending(owner(), output)
        if (size < 0) throw EditorFailure(-size, "Advisory inbox")
        check(size <= 8192); val bytes = ByteArray(size).also { output.get(it) }
        val tokens = JSONArray(bytes.toString(Charsets.UTF_8)); check(tokens.length() <= 64)
        return (0 until tokens.length()).map { tokens.getString(it) }
    }
    fun inspectReview(advice: AdvisoryV2): NativeReviewInspectionV2 {
        val out = record(4640)
        checkedV2(NativeEditorV2.inspectReview(owner(), advice.id.toByteArray(Charsets.UTF_8), out), "Native review inspection")
        check(out.getInt(0) == 2 && out.getInt(4) == 4640 && out.getInt(8) in 0..8 && out.getInt(12) in 0..3)
        check(out.getLong(16) == advice.epoch && out.textAt(24, 33) == advice.id)
        val actions = (0 until out.getInt(8)).map { index ->
            val at = 96 + index * 568
            check(out.getInt(at) == index && out.getInt(at + 4) in 0..10 && out.getInt(at + 8) == 1 && out.getInt(at + 12) == 1)
            check(out.getInt(at + 16) in 1..4 && out.getInt(at + 20) in 0..1)
            val parameters = (0 until out.getInt(at + 16)).map { parameter ->
                val p = at + 120 + parameter * 112
                NativeReviewParameterV2(out.textAt(p, 32), out.textAt(p + 32, 64), out.getInt(p + 96), out.getInt(p + 100), out.getInt(p + 104), out.getInt(p + 108))
            }
            NativeReviewActionV2(index, out.getInt(at + 4), out.textAt(at + 24, 32), out.textAt(at + 56, 64),
                out.getInt(at + 8), out.getInt(at + 12), out.getInt(at + 20) == 1, parameters)
        }
        return NativeReviewInspectionV2(out.textAt(64, 32), out.getLong(16), out.getInt(12), actions)
    }
    fun editReviewCommand(advice: AdvisoryV2, command: Int, action: Int = 0, parameter: Int = 0, value: Int = 0, destination: Int = 0): AdvisoryV2 {
        val out = record(128)
        checkedV2(NativeEditorV2.reviewCommand(owner(), advice.id.toByteArray(Charsets.UTF_8), command, action, parameter, value, destination, out), "Native review edit")
        check(out.getInt(0) == 2 && out.getInt(4) == 128)
        return AdvisoryV2(out.textAt(24, 33), out.textAt(57, 65), out.getInt(8), out.getInt(12), out.getLong(16))
    }
    fun editAdvisory(advice: AdvisoryV2, operations: ByteArray) = advisory(2, input = advice.id, content = operations)
    fun restoreReview(advice: AdvisoryV2) = advisory(3, input = advice.id)
    fun advisoryContent(advice: AdvisoryV2, operations: Boolean): ByteArray {
        val output = record(8192); val size = NativeEditorV2.content(owner(), advice.id.toByteArray(Charsets.UTF_8), operations, output)
        if (size < 0) throw EditorFailure(-size, "Inspect advisory")
        check(size <= 8192); return ByteArray(size).also { output.get(it) }
    }
    fun cancelAdvisory(advice: AdvisoryV2) = checkedV2(NativeEditorV2.cancelAdvisory(owner(), advice.id.toByteArray(Charsets.UTF_8)), "Cancel advisory")
    fun applyAdvisoryReceiptFromHuman(advice: AdvisoryV2): NativePublicationV2 {
        val out = record(336)
        checkedV2(NativeEditorV2.humanApplyReceipt(owner(), advice.id.toByteArray(Charsets.UTF_8), out), "Human AI Apply")
        check(out.getInt(0) == 2 && out.getInt(4) == 336)
        return NativePublicationV2(out.getInt(8), out.getLong(16), out.getLong(24),
            out.getInt(32) != 0, out.getInt(36) != 0, out.textAt(40, 33),
            out.textAt(73, 65), out.textAt(138, 33), out.textAt(171, 33),
            out.textAt(204, 33), out.textAt(237, 33), out.textAt(270, 65))
    }
    fun applyAdvisoryFromHuman(advice: AdvisoryV2) { applyAdvisoryReceiptFromHuman(advice) }
    inner class ExportTask internal constructor(private val editor: Long, private val ticket: Long) : AutoCloseable {
        private val closed = AtomicBoolean(false)
        fun cancel() { check(!closed.get()); checkedV2(NativeEditorV2.exportTask(editor, 2, ticket, 0, 0, -1, null), "Cancel export") }
        fun run(staging: ParcelFileDescriptor): NativeExportObservationV2 {
            check(!closed.get()); val out = record(176)
            checkedV2(NativeEditorV2.exportTask(editor, 1, ticket, 0, 0, staging.fd, out), "Export")
            check(out.getInt(0) == 2 && out.getInt(4) == 176 && out.getLong(24) > 0)
            return NativeExportObservationV2(out.getInt(8), out.getInt(12), out.getInt(16), out.getLong(24),
                out.getLong(32), out.textAt(40, 65), out.textAt(105, 33), out.textAt(138, 33))
        }
        override fun close() { if (closed.compareAndSet(false, true)) NativeEditorV2.exportTask(editor, 3, ticket, 0, 0, -1, null) }
    }
    fun beginExport(format: Int, quality: Int = 90): ExportTask {
        val current = owner(); val out = record(8)
        checkedV2(NativeEditorV2.exportTask(current, 0, 0, format, quality, -1, out), "Begin export")
        return ExportTask(current, out.getLong(0))
    }
    fun exportSavedObserved(staging: ParcelFileDescriptor, format: Int, quality: Int = 90): NativeExportObservationV2 {
        val out = record(176)
        checkedV2(NativeEditorV2.exportObserved(owner(), format, quality, staging.fd, out), "Export")
        check(out.getInt(0) == 2 && out.getInt(4) == 176 && out.getInt(16) == format && out.getLong(24) > 0)
        return NativeExportObservationV2(out.getInt(8), out.getInt(12), out.getInt(16), out.getLong(24),
            out.getLong(32), out.textAt(40, 65), out.textAt(105, 33), out.textAt(138, 33))
    }
    fun exportSaved(staging: ParcelFileDescriptor, format: Int, quality: Int = 90): Long {
        val output = record(8); checkedV2(NativeEditorV2.export(owner(), format, quality, staging.fd, output), "Export")
        return output.getLong(0)
    }
    fun stop() { val h = synchronized(lock) { native }; if (h != 0L) checkedV2(NativeEditorV2.action(h, 4, 0), "Stop") }
    override fun close() {
        val h = synchronized(lock) { val value = native; native = 0; value }
        if (h != 0L) { NativeEditorV2.action(h, 4, 0); checkedV2(NativeEditorV2.action(h, 5, 0), "Close") }
    }
}
