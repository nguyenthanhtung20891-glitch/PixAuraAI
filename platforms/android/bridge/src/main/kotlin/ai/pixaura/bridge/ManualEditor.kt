package ai.pixaura.bridge

import android.graphics.Bitmap
import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
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
class ManualEditor internal constructor(root: String, input: InputStream?, bytes: Long, initialCapability: AndroidCapabilitySnapshot?) : AutoCloseable {
    constructor(root: String, input: InputStream? = null, bytes: Long = 0) : this(root, input, bytes, null)
    companion object { init { System.loadLibrary("pixaura_core"); System.loadLibrary("pixaura_bridge") } }
    private val identity = UUID.randomUUID().toString().replace("-", "")
    private val gate = ReentrantLock()
    private val idle = gate.newCondition()
    private val storage = ByteBuffer.allocateDirect(nativeStorageSize())
    private var closed = false
    private var active = 0
    private var agentCalls = 0
    private var assistCalls = 0
    private var comparison: ReviewComparison? = null
    private var applyingReview: AutoReview? = null
    private var applyingApproval: AgentApproval? = null
    private var cancellingReview = false
    private var attemptedAutoSerial: Long? = null
    private var stopped = false
    private var destroyed = false
    private var displayed: Bitmap? = null
    private var committed: Bitmap? = null
    private var requestedGeneration = 0L
    class Request internal constructor(internal val owner: String, internal val bytes: ByteBuffer, internal val approved: Boolean, internal val generation: Long)
    class Candidate internal constructor(val request: Request, val bitmap: Bitmap)
    val tools: List<ManualTool>
    init {
        checked(nativeOpen(storage, root.toByteArray(Charsets.UTF_8),
            identity.toByteArray(Charsets.US_ASCII), input, bytes, initialCapability?.facts?.nativeWords(),
            initialCapability?.observation?.nativeWords()), "Open")
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
    internal fun configureCapability(snapshot: AndroidCapabilitySnapshot): ExecutionStrategy = gate.withLock {
        check(!closed)
        ExecutionStrategy.fromNative(checkNotNull(nativePolicyConfigure(storage, snapshot.facts.nativeWords(), snapshot.observation.nativeWords())))
    }
    internal fun updateCapability(observation: AndroidObservation): ExecutionStrategy = gate.withLock {
        check(!closed)
        ExecutionStrategy.fromNative(checkNotNull(nativePolicyUpdate(storage, observation.nativeWords())))
    }
    internal fun executionStrategy(): ExecutionStrategy = gate.withLock {
        check(!closed); ExecutionStrategy.fromNative(checkNotNull(nativePolicyQuery(storage)))
    }
    internal fun hasRequestedPreview(): Boolean = gate.withLock { !closed && requestedGeneration != 0L }
    fun isRequested(request: Request): Boolean = gate.withLock { !closed && request.owner == identity && request.generation == requestedGeneration }
    /** Immutable untrusted proposal bytes; the core revalidates at every authority gate. */
    class AgentPlan internal constructor(bytes: ByteArray) {
        private val canonical = bytes.copyOf()
        fun canonicalBytes(): ByteArray = canonical.copyOf()
    }
    class AgentApproval internal constructor(internal val owner: ManualEditor) : AutoCloseable {
        internal val token: ByteBuffer = ByteBuffer.allocateDirect(56)
        internal var owned = false
        override fun close() = owner.releaseAgentApproval(this)
    }
    class AgentReceipt internal constructor() {
        internal val buffer: ByteBuffer = ByteBuffer.allocateDirect(144)
        private fun text(offset: Int, length: Int): String {
            val bytes = ByteArray(length); val copy = buffer.duplicate(); copy.position(offset); copy.get(bytes)
            return String(bytes, Charsets.US_ASCII)
        }
        val planSHA256: String get() = text(16, 64)
        val sourceRevision: String get() = text(80, 32)
        val resultingRevision: String get() = text(112, 32)
        internal fun validate() {
            val b = buffer.duplicate().order(ByteOrder.nativeOrder())
            if (b.getInt(0) != 1 || b.getInt(4) != 144 || b.getInt(8) != 1 || b.getInt(12) != 0 ||
                listOf(planSHA256, sourceRevision, resultingRevision).any { s -> s.any { it !in '0'..'9' && it !in 'a'..'f' } } || sourceRevision == resultingRevision)
                throw EditorFailure(14, "Agent receipt")
        }
    }
    private fun agentBytes(mode: Int, input: ByteArray?): ByteArray = gate.withLock {
        checked(if (closed) 13 else 0, "Agent")
        val output = ByteBuffer.allocateDirect(8200).order(ByteOrder.nativeOrder())
        checked(nativeAgentBytes(storage, mode, input, output), "Agent")
        val length = output.getLong(0)
        checked(if (length in 1..8192) 0 else 14, "Agent output")
        ByteArray(length.toInt()).also { output.position(8); output.get(it) }
    }
    fun agentBinding(): ByteArray = agentBytes(1, null)
    fun referenceAgentPlan(canonicalStructuredIntent: ByteArray): AgentPlan =
        AgentPlan(agentBytes(3, canonicalStructuredIntent))
    fun validateAgentPlan(canonicalPlan: ByteArray): AgentPlan = AgentPlan(agentBytes(2, canonicalPlan))
    /** Trusted application boundary: invoke ONLY for explicit human confirmation.
     * Never give an editor/approval capability to planner or model code. */
    fun approveAgentPlanFromHuman(plan: AgentPlan): AgentApproval = gate.withLock {
        checked(if (closed) 13 else 0, "Agent approval")
        val approval = AgentApproval(this)
        checked(nativeAgentApprove(storage, plan.canonicalBytes(), approval.token), "Agent approval")
        approval.owned = true; approval
    }
    fun revokeAgentApproval(approval: AgentApproval) = gate.withLock {
        checked(if (closed) 13 else nativeAgentTerminal(storage, approval.token, false), "Agent revocation")
    }
    private fun releaseAgentApproval(approval: AgentApproval) = gate.withLock {
        if (approval.owned) {
            if (!closed) checked(nativeAgentTerminal(storage, approval.token, true), "Agent release")
            approval.owned = false
        }
    }
    fun executeAgentPlan(plan: AgentPlan, approval: AgentApproval? = null): AgentReceipt {
        // Allocate transport output before native publication; no JNI allocation
        // or Java callback is needed after the core commits.
        val result = AgentReceipt()
        val bytes = plan.canonicalBytes()
        gate.withLock {
            checked(if (closed) 13 else if (agentCalls >= 8) 8 else 0, "Agent execution")
            agentCalls++
        }
        try {
            checked(nativeAgentExecute(storage, bytes, approval?.token, result.buffer), "Agent execution")
            result.validate()
            gate.withLock { requestedGeneration = 0 }
            return result
        } finally { gate.withLock { agentCalls--; idle.signalAll() } }
    }
    fun cancelAgentExecution() = gate.withLock {
        checked(if (closed) 13 else nativeAgentCancel(storage), "Agent cancellation")
    }
    class AssistChoice internal constructor(val index: Int, val toolIndex: Int, val values: List<Int>)
    class AssistAdvice internal constructor(internal val owner: ManualEditor, internal val packet: ByteBuffer, val choice: AssistChoice)
    private fun assistChoice(packet: ByteBuffer): AssistChoice {
        val data = packet.duplicate().order(ByteOrder.nativeOrder())
        val count = data.getInt(16)
        checked(if (data.getInt(0) == 1 && data.getInt(4) == nativeAssistSize() && count in 1..4) 0 else 14, "Advice")
        return AssistChoice(data.getInt(8), data.getInt(12), List(count) { data.getInt(24 + it * 4) })
    }
    fun adviceChoices(): List<AssistChoice> = gate.withLock {
        checked(if (closed) 13 else 0, "Advice")
        val count = nativeAssistCount(); checked(if (count in 1..11) 0 else 14, "Advice")
        List(count) { index -> val packet = ByteBuffer.allocateDirect(nativeAssistSize())
            checked(nativeAssistChoice(index, packet), "Advice"); assistChoice(packet) }
    }
    fun requestAdvice(index: Int): AssistAdvice {
        val packet = ByteBuffer.allocateDirect(nativeAssistSize())
        gate.withLock { checked(if (closed) 13 else if (assistCalls != 0 || applyingReview != null || cancellingReview) 8 else 0, "Advice"); assistCalls++ }
        try {
            checked(nativeAssistRequest(storage, index, packet), "Advice")
            return AssistAdvice(this, packet.asReadOnlyBuffer(), assistChoice(packet))
        } finally { gate.withLock { assistCalls--; idle.signalAll() } }
    }
    fun inspectAdvice(advice: AssistAdvice): AssistChoice = gate.withLock {
        checked(if (closed) 13 else if (advice.owner !== this) 9 else nativeAssistCurrent(storage, advice.packet), "Advice")
        advice.choice
    }
    fun cancelAdvice() = gate.withLock { checked(if (closed) 13 else nativeAssistCancel(storage), "Advice") }
    class AutoProposal internal constructor(internal val owner: ManualEditor, internal val packet: ByteBuffer,
        val actions: List<AssistChoice>, val proposalId: String, val providerId: String)
    fun requestAuto(index: Int): AutoProposal {
        val packet = ByteBuffer.allocateDirect(nativeAutoSize()).order(ByteOrder.nativeOrder())
        gate.withLock { checked(if (closed) 13 else if (assistCalls != 0 || applyingReview != null || cancellingReview) 8 else 0, "Auto proposal"); assistCalls++ }
        try {
            checked(nativeAutoRequest(storage, index, packet), "Auto proposal")
            val count = packet.getInt(8)
            checked(if (packet.getInt(0) == 1 && packet.getInt(4) == 328 && count in 1..8) 0 else 14, "Auto proposal")
            val actions = List(count) { action -> val at = 136 + action * 24; val n = packet.getInt(at + 4)
                checked(if (n in 1..4) 0 else 14, "Auto proposal")
                AssistChoice(action, packet.getInt(at), List(n) { packet.getInt(at + 8 + it * 4) }) }
            fun id(at: Int): String = ByteArray(32) { packet.get(at + it) }.takeWhile { it != 0.toByte() }.toByteArray().toString(Charsets.US_ASCII)
            return AutoProposal(this, packet.asReadOnlyBuffer(), actions, id(72), id(104))
        } finally { gate.withLock { assistCalls--; idle.signalAll() } }
    }
    fun inspectAuto(proposal: AutoProposal): ByteArray = gate.withLock {
        checked(if (closed) 13 else if (proposal.owner !== this) 9 else 0, "Auto proposal")
        val bytes = ByteBuffer.allocateDirect(8192); val size = nativeAutoPlan(storage, proposal.packet, bytes)
        checked(if (size < 0) -size else if (size in 1..8192) 0 else 14, "Auto proposal")
        ByteArray(size).also { bytes.get(it) }
    }
    fun cancelAuto() = gate.withLock { checked(if (closed) 13 else nativeAutoCancel(storage), "Auto proposal") }
    /** Presentation owns this immutable reviewed plan, never an approval token. */
    class AutoReview internal constructor(internal val owner: ManualEditor, internal val proposal: AutoProposal,
        internal val plan: AgentPlan, val sourceRevision: String) {
        internal var attempted = false
        internal var abandoned = false
        fun canonicalBytes(): ByteArray = plan.canonicalBytes()
    }
    fun prepareAutoReview(proposal: AutoProposal): AutoReview = gate.withLock {
        checked(if (closed) 13 else if (proposal.owner !== this) 9 else 0, "Auto review")
        checked(if (applyingReview != null || cancellingReview) 8 else 0, "Auto review")
        checked(if (proposal.packet.duplicate().order(ByteOrder.nativeOrder()).getLong(56) == attemptedAutoSerial) 15 else 0, "Auto review")
        val plan = validateAgentPlan(inspectAuto(proposal))
        AutoReview(this, proposal, plan, state().revisionId)
    }
    /** ONLY the explicit review Apply action may invoke this trusted boundary. */
    fun applyAutoFromHuman(review: AutoReview, onExecuting: () -> Unit = {}): AgentReceipt {
        val approval = gate.withLock {
            checked(if (closed) 13 else if (review.owner !== this) 9 else if (review.attempted || review.abandoned) 15
                else if (applyingReview != null || cancellingReview || assistCalls != 0) 8 else 0, "Auto Apply")
            checked(if (inspectAuto(review.proposal).contentEquals(review.plan.canonicalBytes())) 0 else 9, "Auto review")
            checked(if (validateAgentPlan(review.plan.canonicalBytes()).canonicalBytes().contentEquals(review.plan.canonicalBytes())) 0 else 9, "Auto review")
            checked(if (attemptedAutoSerial == review.proposal.packet.duplicate().order(ByteOrder.nativeOrder()).getLong(56)) 15 else 0, "Auto Apply")
            review.attempted = true; attemptedAutoSerial = review.proposal.packet.duplicate().order(ByteOrder.nativeOrder()).getLong(56)
            val issued = approveAgentPlanFromHuman(review.plan); applyingApproval = issued
            applyingReview = review; agentCalls++ // includes approval cleanup and callback; close must join all of it
            issued
        }
        try {
            onExecuting() // notification only, never an approval issuer
            val receipt = executeAgentPlan(review.plan, approval)
            checked(if (receipt.sourceRevision == review.sourceRevision) 0 else 14, "Auto receipt")
            return receipt
        } finally { gate.withLock {
            try { approval.close() } finally {
                applyingApproval = null; applyingReview = null; agentCalls--; idle.signalAll()
            }
        } }
    }
    fun cancelAutoReview(review: AutoReview) {
        val approval = gate.withLock {
            checked(if (closed) 13 else if (review.owner !== this) 9 else 0, "Auto cancellation")
            review.abandoned = true
            if (applyingReview !== review || cancellingReview) return
            cancellingReview = true; agentCalls++; applyingApproval
        }
        try {
            // Revoke first closes the gap before execute admission; cancel then
            // fences an execution that already consumed approval. No retry.
            if (approval != null) nativeAgentTerminal(storage, approval.token, false)
            cancelAgentExecution()
        } finally { gate.withLock { cancellingReview = false; agentCalls--; idle.signalAll() } }
    }
    override fun close() = gate.withLock {
        comparison = null
        closed = true
        if (!stopped) { checked(nativeStop(storage), "Close"); stopped = true }
        while (active != 0 || agentCalls != 0 || assistCalls != 0) idle.awaitUninterruptibly()
        if (!destroyed) { checked(nativeDestroy(storage), "Close"); destroyed = true }
    }
    private external fun nativeAgentBytes(storage: ByteBuffer, mode: Int, input: ByteArray?, output: ByteBuffer): Int
    /** Disposable display resources. No approval/candidate is present here. */
    class ReviewPreviewRequest internal constructor(internal val owner: ManualEditor, internal val ticket: ByteBuffer, internal val review: AutoReview)
    class ReviewComparison internal constructor(internal val owner: ManualEditor, internal val packet: ByteBuffer,
        internal val request: ReviewPreviewRequest, val before: Bitmap, val after: Bitmap, val geometryChanged: Boolean,
        val dimensions: String)
    fun beginReviewPreview(review: AutoReview): ReviewPreviewRequest = gate.withLock {
        checked(if (closed) 13 else if (review.owner !== this || review.attempted || review.abandoned) 9
            else if (assistCalls != 0 || applyingReview != null || cancellingReview) 8 else 0, "Review preview")
        comparison = null
        val ticket = ByteBuffer.allocateDirect(56).order(ByteOrder.nativeOrder())
        checked(nativeReviewBegin(storage, review.proposal.packet, review.canonicalBytes(), ticket), "Review preview")
        ReviewPreviewRequest(this, ticket.asReadOnlyBuffer(), review)
    }
    fun renderReviewPreview(request: ReviewPreviewRequest): ReviewComparison {
        val packet = gate.withLock {
            checked(if (closed) 13 else if (request.owner !== this) 9 else if (assistCalls != 0 || applyingReview != null) 8 else 0, "Review preview")
            // Allocate transport before joining the in-flight count: an OOM here
            // must not strand a count outside the release/join finally block.
            val output = ByteBuffer.allocateDirect(176).order(ByteOrder.nativeOrder())
            assistCalls++; output
        }
        try {
            checked(nativeReviewRender(storage, request.ticket, packet), "Review preview")
            fun image(side: Int): Bitmap {
                val words = nativeReviewImage(storage, packet, side) ?: throw EditorFailure(8, "Review preview copy")
                checked(if (words.size >= 3) words[0] else 14, "Review preview copy")
                val width = words[1]; val height = words[2]
                checked(if (width in 1..1024 && height in 1..1024 && words.size == 3 + width * height) 0 else 14, "Review preview copy")
                return Bitmap.createBitmap(words, 3, width, width, height, Bitmap.Config.ARGB_8888)
            }
            val before = image(0); val after = image(1)
            checked(nativeReviewCurrent(storage, packet), "Review preview current")
            return ReviewComparison(this, packet.asReadOnlyBuffer(), request, before, after, packet.getInt(152) == 1,
                "Before ${packet.getInt(136)} × ${packet.getInt(140)}; After ${packet.getInt(144)} × ${packet.getInt(148)}")
        } finally {
            nativeReviewRelease(storage, packet) // may be absent on failure; no publication authority
            gate.withLock { assistCalls--; idle.signalAll() }
        }
    }
    /** Check and actual presentation replacement share this lifecycle gate. */
    fun installReviewPreview(value: ReviewComparison, install: () -> Unit): Boolean = gate.withLock {
        if (closed || value.owner !== this || nativeReviewCurrent(storage, value.packet) != 0) return false
        comparison = value; install(); true
    }
    fun cancelReviewPreview(request: ReviewPreviewRequest) = gate.withLock {
        checked(if (closed) 13 else if (request.owner !== this) 9 else nativeReviewCancel(storage, request.ticket), "Review preview cancellation")
        if (comparison?.request === request) comparison = null
    }
    private external fun nativeReviewBegin(storage: ByteBuffer, proposal: ByteBuffer, plan: ByteArray, ticket: ByteBuffer): Int
    private external fun nativeReviewRender(storage: ByteBuffer, ticket: ByteBuffer, pair: ByteBuffer): Int
    private external fun nativeReviewCurrent(storage: ByteBuffer, pair: ByteBuffer): Int
    private external fun nativeReviewImage(storage: ByteBuffer, pair: ByteBuffer, side: Int): IntArray?
    private external fun nativeReviewRelease(storage: ByteBuffer, pair: ByteBuffer): Int
    private external fun nativeReviewCancel(storage: ByteBuffer, ticket: ByteBuffer): Int
    private external fun nativeAgentApprove(storage: ByteBuffer, plan: ByteArray, approval: ByteBuffer): Int
    private external fun nativeAgentTerminal(storage: ByteBuffer, approval: ByteBuffer, release: Boolean): Int
    private external fun nativeAgentExecute(storage: ByteBuffer, plan: ByteArray, approval: ByteBuffer?, receipt: ByteBuffer): Int
    private external fun nativeAgentCancel(storage: ByteBuffer): Int
    private external fun nativeAssistSize(): Int
    private external fun nativeAutoSize(): Int
    private external fun nativeAutoRequest(storage: ByteBuffer, index: Int, output: ByteBuffer): Int
    private external fun nativeAutoPlan(storage: ByteBuffer, proposal: ByteBuffer, output: ByteBuffer): Int
    private external fun nativeAutoCancel(storage: ByteBuffer): Int
    private external fun nativeAssistCount(): Int
    private external fun nativeAssistChoice(index: Int, output: ByteBuffer): Int
    private external fun nativeAssistRequest(storage: ByteBuffer, index: Int, output: ByteBuffer): Int
    private external fun nativeAssistCurrent(storage: ByteBuffer, advice: ByteBuffer): Int
    private external fun nativeAssistCancel(storage: ByteBuffer): Int
    private external fun nativeStorageSize(): Int
    private external fun nativeRequestSize(): Int
    private external fun nativeGeneration(request: ByteBuffer): Long
    private external fun nativeOpen(storage: ByteBuffer, root: ByteArray, identity: ByteArray, input: InputStream?, bytes: Long,
        facts: LongArray?, observation: LongArray?): Int
    private external fun nativeSelect(storage: ByteBuffer, index: Int): Int
    private external fun nativeUpdate(storage: ByteBuffer, values: IntArray): Int
    private external fun nativeAction(storage: ByteBuffer, action: Int): Int
    private external fun nativeState(storage: ByteBuffer): String?
    private external fun nativeRequest(storage: ByteBuffer, request: ByteBuffer): Int
    private external fun nativeRender(storage: ByteBuffer, request: ByteBuffer): IntArray?
    private external fun nativeCurrent(storage: ByteBuffer, request: ByteBuffer): Int
    private external fun nativeStop(storage: ByteBuffer): Int
    private external fun nativeDestroy(storage: ByteBuffer): Int
    private external fun nativePolicyConfigure(storage: ByteBuffer, facts: LongArray, observation: LongArray): LongArray?
    private external fun nativePolicyUpdate(storage: ByteBuffer, observation: LongArray): LongArray?
    private external fun nativePolicyQuery(storage: ByteBuffer): LongArray?
}
