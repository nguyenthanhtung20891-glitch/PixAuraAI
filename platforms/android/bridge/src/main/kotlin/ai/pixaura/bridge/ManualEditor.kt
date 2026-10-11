package ai.pixaura.bridge

import android.graphics.Bitmap
import java.io.InputStream
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock
import org.json.JSONObject

data class ManualParameter(val name: String, val unit: String, val minimum: Int,
    val maximum: Int, val default: Int, val step: Int)
data class ManualTool(val index: Int, val id: String, val name: String, val category: String,
    val parameters: List<ManualParameter>)
data class ManualState(val active: Boolean, val approvalPending: Boolean, val canUndo: Boolean,
    val canRedo: Boolean, val revisionCount: Int, val revisionId: String, val selected: Int, val editId: String, val gestureState: Int, val values: List<Int>)
class EditorFailure(val status: Int, val boundary: String) : Exception("$boundary failed (core status $status)")

/** Existing PRV1 owner pattern: short lifecycle/check+display gate, detached
 * Bitmap copies, bounded synchronous worker admission, stop/join/destroy.
 * Construct and call IO/render/approval on application workers, never UI thread.
 */
class ManualEditor(root: String, input: InputStream? = null, bytes: Long = 0) : AutoCloseable {
    companion object { init { System.loadLibrary("pixaura_core"); System.loadLibrary("pixaura_bridge") } }
    private val identity = UUID.randomUUID().toString().replace("-", "")
    private val gate = ReentrantLock()
    private val idle = gate.newCondition()
    private val storage = ByteBuffer.allocateDirect(nativeStorageSize())
    private var closed = false
    private var active = 0
    private var displayed: Bitmap? = null
    private var committed: Bitmap? = null
    private var requestedGeneration = 0L
    class Request internal constructor(internal val owner: String, internal val bytes: ByteBuffer, internal val approved: Boolean, internal val generation: Long)
    class Candidate internal constructor(val request: Request, val bitmap: Bitmap)
    val tools: List<ManualTool>
    init {
        checked(nativeOpen(storage, root.toByteArray(Charsets.UTF_8),
            identity.toByteArray(Charsets.US_ASCII), input, bytes), "Open")
        tools = try {
        val json = JSONObject(String(checkNotNull(CoreProbe().nativeManualRegistry()), Charsets.UTF_8)).getJSONArray("tools")
        (0 until json.length()).map { i ->
            val t = json.getJSONObject(i); val p = t.getJSONArray("parameters")
            ManualTool(i, t.getString("tool_id"), t.getString("name"), t.getString("category"),
                (0 until p.length()).map { j -> val v = p.getJSONObject(j)
                    ManualParameter(v.getString("name"), v.getString("unit"), v.getInt("minimum"),
                        v.getInt("maximum"), v.getInt("default"), v.getInt("step")) })
        }
        } catch (failure: Throwable) { nativeStop(storage); nativeDestroy(storage); throw failure }
    }
    private fun checked(status: Int, boundary: String) { if (status != 0) throw EditorFailure(status, boundary) }
    fun state(): ManualState = gate.withLock {
        check(!closed)
        val j = JSONObject(checkNotNull(nativeState(storage)))
        val v = j.getJSONArray("values")
        ManualState(j.getBoolean("active"), j.getBoolean("approval_pending"), j.getBoolean("can_undo"),
            j.getBoolean("can_redo"), j.getInt("revision_count"), j.getString("revision_id"), j.getInt("selected"), j.getString("edit_id"), j.getInt("gesture_state"),
            (0 until v.length()).map { v.getInt(it) })
    }
    fun select(index: Int) = gate.withLock { check(!closed); requestedGeneration = 0; displayed = committed; checked(nativeSelect(storage, index), "Tool") }
    fun update(values: List<Int>, editId: String? = null) = gate.withLock { check(!closed)
        if (editId != null && state().editId != editId) throw EditorFailure(9, "Parameter")
        checked(nativeUpdate(storage, values.toIntArray()), "Parameter"); requestedGeneration = 0 }
    fun action(action: Int) = gate.withLock { check(!closed)
        val status = nativeAction(storage, action)
        if (status == 0 || status == 9) { requestedGeneration = 0; if (action == 0 || action == 4 || status == 9) displayed = committed }
        checked(status, if (action == 1) "Commit" else "History/cancel") }
    fun request(): Request = gate.withLock {
        check(!closed); val request = ByteBuffer.allocateDirect(nativeRequestSize())
        checked(nativeRequest(storage, request), "Preview"); requestedGeneration = nativeGeneration(request)
        Request(identity, request, !state().active, requestedGeneration)
    }
    fun render(request: Request): Candidate? {
        gate.withLock { if (closed || request.owner != identity || active >= 1) return null; active++ }
        try {
            val pixels = nativeRender(storage, request.bytes) ?: throw EditorFailure(8, "Preview")
            checked(pixels[0], "Preview")
            return Candidate(request, Bitmap.createBitmap(pixels, 3, pixels[1], pixels[1], pixels[2], Bitmap.Config.ARGB_8888))
        } finally { gate.withLock { active--; idle.signalAll() } }
    }
    fun install(candidate: Candidate, replace: (Bitmap) -> Unit = {}): Boolean = gate.withLock {
        if (closed || candidate.request.owner != identity || nativeCurrent(storage, candidate.request.bytes) != 0) false
        else { displayed = candidate.bitmap; if (candidate.request.approved) committed = candidate.bitmap; replace(candidate.bitmap); true }
    }
    fun displayedBitmap(): Bitmap? = gate.withLock { displayed }
    fun isRequested(request: Request): Boolean = gate.withLock { !closed && request.owner == identity && request.generation == requestedGeneration }
    override fun close() = gate.withLock {
        if (!closed) {
            closed = true; checked(nativeStop(storage), "Close")
            while (active != 0) idle.awaitUninterruptibly()
            checked(nativeDestroy(storage), "Close")
        }
    }
    private external fun nativeStorageSize(): Int
    private external fun nativeRequestSize(): Int
    private external fun nativeGeneration(request: ByteBuffer): Long
    private external fun nativeOpen(storage: ByteBuffer, root: ByteArray, identity: ByteArray, input: InputStream?, bytes: Long): Int
    private external fun nativeSelect(storage: ByteBuffer, index: Int): Int
    private external fun nativeUpdate(storage: ByteBuffer, values: IntArray): Int
    private external fun nativeAction(storage: ByteBuffer, action: Int): Int
    private external fun nativeState(storage: ByteBuffer): String?
    private external fun nativeRequest(storage: ByteBuffer, request: ByteBuffer): Int
    private external fun nativeRender(storage: ByteBuffer, request: ByteBuffer): IntArray?
    private external fun nativeCurrent(storage: ByteBuffer, request: ByteBuffer): Int
    private external fun nativeStop(storage: ByteBuffer): Int
    private external fun nativeDestroy(storage: ByteBuffer): Int
}
