package ai.pixaura.app

import android.app.Application
import android.graphics.Bitmap
import android.net.Uri
import androidx.lifecycle.AndroidViewModel
import androidx.core.content.edit
import ai.pixaura.bridge.EditorFailure
import ai.pixaura.bridge.CoreProbe
import ai.pixaura.bridge.ManualEditor
import ai.pixaura.bridge.ManualState
import ai.pixaura.bridge.ManualTool
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.ExecutionStrategy
import java.io.File
import java.util.UUID
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicReference
import java.util.concurrent.atomic.AtomicLong
import java.io.InputStream
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

data class EditorUiState(val tools: List<ManualTool> = emptyList(), val core: ManualState? = null,
    val image: Bitmap? = null, val busy: Boolean = false, val previewLoading: Boolean = false, val error: String? = null)
enum class AssistStatus { IDLE, REQUESTING, PROPOSAL_AVAILABLE, FAILED, STALE, CANCELLED, REJECTED }
data class AssistUiState(val enabled: Boolean = false, val status: AssistStatus = AssistStatus.IDLE,
    val choices: List<ManualEditor.AssistChoice> = emptyList(), val selected: Int = -1,
    val advice: ManualEditor.AssistAdvice? = null, val inFlight: Boolean = false,
    val error: String? = null, val notice: String? = null)

/** Two fixed lanes, one coalesced input and one coalesced preview request.
 * Native lifecycle/history is authoritative; no platform operation/history model.
 */
class ManualEditorModel(application: Application) : AndroidViewModel(application) {
    private val commands = Executors.newSingleThreadExecutor()
    private val rendering = Executors.newSingleThreadExecutor()
    private val advising = Executors.newSingleThreadExecutor()
    @Volatile private var owner: ManualEditor? = null
    private val assistMutable = MutableStateFlow(AssistUiState())
    val assistance = assistMutable.asStateFlow()
    private val assistEpoch = AtomicLong(0)
    private val assistActive = AtomicBoolean(false)
    private val loadingChoices = AtomicBoolean(false)
    private val cancellingAdvice = AtomicBoolean(false)
    private val pendingValues = AtomicReference<Pair<String, List<Int>>?>(null)
    private val updating = AtomicBoolean(false)
    private val pendingPreview = AtomicReference<Pair<ManualEditor, ManualEditor.Request>?>(null)
    private val renderingNow = AtomicBoolean(false)
    private val disposed = AtomicBoolean(false)
    private val mutable = MutableStateFlow(EditorUiState())
    val state = mutable.asStateFlow()
    private val app = application
    private val preferences = app.getSharedPreferences("manual-editor", 0)
    private val capabilities = (app as PixAuraApplication).capabilityOwner
    private val pendingCapability = AtomicReference<AndroidCapabilitySnapshot?>(null)
    private val applyingCapability = AtomicBoolean(false)
    private var capabilitySubscription: AutoCloseable? = null
    private var appliedCapabilitySequence = 0L
    internal val capabilityStrategy = MutableStateFlow<ExecutionStrategy?>(null)
    init { commands.execute {
        capabilitySubscription = capabilities.subscribe { snapshot ->
            pendingCapability.updateAndGet { previous ->
                if (previous == null) snapshot else snapshot.copy(observation = snapshot.observation.copy(
                    thermal = maxOf(previous.observation.thermal, snapshot.observation.thermal),
                    memory = maxOf(previous.observation.memory, snapshot.observation.memory),
                    constrained = previous.observation.constrained || snapshot.observation.constrained,
                    vulkan = minOf(previous.observation.vulkan, snapshot.observation.vulkan),
                    processBudgetBytes = minOf(previous.observation.processBudgetBytes, snapshot.observation.processBudgetBytes)))
            }
            drainCapabilities()
        }
    } }
    private fun configured(root: String, input: InputStream? = null, bytes: Long = 0): ManualEditor {
        val sampled = capabilities.sample()
        val snapshot = pendingCapability.getAndSet(null) ?: sampled
        val editor = ManualEditor(root, input, bytes, snapshot)
        try { capabilityStrategy.value = editor.executionStrategy(); appliedCapabilitySequence = snapshot.observation.sequence; return editor }
        catch (failure: Throwable) { editor.close(); throw failure }
    }
    private fun applyPendingCapability(): Boolean {
        val editor = owner ?: return false
        val snapshot = pendingCapability.getAndSet(null) ?: return false
        if (snapshot.observation.sequence <= appliedCapabilitySequence) return false
        val before = editor.executionStrategy().epoch
        val next = editor.updateCapability(snapshot.observation)
        capabilityStrategy.value = next
        appliedCapabilitySequence = snapshot.observation.sequence
        return next.epoch != before
    }
    /** One fixed pending slot/runnable; coalescing retains every worsening signal.
     * Core alone derives transitions. Native calls stay on the command lane. */
    private fun drainCapabilities() {
        if (disposed.get() || !applyingCapability.compareAndSet(false, true)) return
        commands.execute {
            try { if (applyPendingCapability()) {
                if (owner?.hasRequestedPreview() == true) preview()
                refresh()
            } }
            catch (failure: Throwable) { refresh(message(failure)) }
            finally { applyingCapability.set(false)
                if (!disposed.get() && owner != null && pendingCapability.get() != null) drainCapabilities() }
        }
    }
    private fun root() = File(app.noBackupFilesDir, "manual-projects").apply { mkdirs() }
    private fun message(e: Throwable): String = if (e is EditorFailure) {
        val meaning = when (e.status) {
            7 -> "Invalid parameter"
            9, 3 -> "Stale edit; reopen the committed project"
            8, 15 -> "Resource/admission failure"
            5, 16 -> "Tool or image capability unavailable"
            else -> "${e.boundary} failure"
        }
        val prefix = if (e.boundary == "Preview" || e.boundary == "Commit") "${e.boundary} failure: " else ""
        "$prefix$meaning (core ${e.status})"
    } else when (e) { is LinkageError -> "Shared core unavailable"; is OutOfMemoryError -> "Resource/admission failure"; else -> "${e.message ?: "Editor failure"}" }
    private fun refresh(error: String? = null) {
        val editor = owner
        mutable.value = mutable.value.copy(tools = editor?.tools ?: emptyList(), core = editor?.state(),
            image = editor?.displayedBitmap(), busy = false, error = error)
        if (assistMutable.value.enabled && assistMutable.value.choices.isEmpty()) loadAdviceChoices()
    }
    private fun nextAssistEpoch(): Long? {
        while (true) { val previous = assistEpoch.get()
            if (previous == Long.MAX_VALUE) { assistMutable.update { it.copy(status = AssistStatus.FAILED, error = "Advice session ended. Manual editing is available.", advice = null) }; return null }
            if (assistEpoch.compareAndSet(previous, previous + 1)) return previous + 1
        }
    }
    private fun cancelNativeAdvice() {
        if (disposed.get() || !cancellingAdvice.compareAndSet(false, true)) return
        commands.execute { try { owner?.cancelAdvice() } catch (_: Throwable) { /* Manual remains independent. */ }
            finally { cancellingAdvice.set(false) } }
    }
    private fun invalidateAdvice(reason: AssistStatus = AssistStatus.STALE) {
        nextAssistEpoch()
        val hadWork = assistMutable.value.advice != null || assistActive.get()
        assistMutable.update { it.copy(advice = null, status = if (it.enabled && hadWork) reason else AssistStatus.IDLE, error = null, notice = null) }
        if (hadWork) cancelNativeAdvice()
    }
    fun setAssistMode(enabled: Boolean) {
        invalidateAdvice(AssistStatus.CANCELLED)
        assistMutable.update { it.copy(enabled = enabled, status = AssistStatus.IDLE, advice = null, error = null, notice = null) }
        if (enabled) loadAdviceChoices()
    }
    private fun loadAdviceChoices() {
        if (disposed.get() || assistMutable.value.choices.isNotEmpty() || !loadingChoices.compareAndSet(false, true)) return
        commands.execute {
            try { owner?.adviceChoices()?.let { choices -> if (!disposed.get()) assistMutable.update { it.copy(choices = choices) } } }
            catch (_: Throwable) { if (!disposed.get() && assistMutable.value.enabled) assistMutable.update { it.copy(status = AssistStatus.FAILED, error = "Advice is unavailable. Manual editing is available.") } }
            finally { loadingChoices.set(false) }
        }
    }
    fun chooseAdvice(index: Int) {
        if (assistMutable.value.inFlight) return
        invalidateAdvice(); assistMutable.update { it.copy(selected = index, status = AssistStatus.IDLE) }
    }
    private fun adviceFailure(failure: Throwable): Pair<AssistStatus, String> = when ((failure as? EditorFailure)?.status) {
        3, 9 -> AssistStatus.STALE to "This advice has expired. Request new guidance for the current image."
        13 -> AssistStatus.CANCELLED to "Advice cancelled or stopped. Manual editing is available."
        8 -> AssistStatus.FAILED to "Advice is busy or resources are limited. You can keep editing manually."
        else -> AssistStatus.FAILED to "Advice could not be validated. You can keep editing manually."
    }
    fun requestAdvice() {
        val ui = assistMutable.value
        if (disposed.get() || !ui.enabled || ui.selected !in ui.choices.indices || !assistActive.compareAndSet(false, true)) return
        val editor = owner
        val epoch = nextAssistEpoch()
        if (editor == null || epoch == null) { assistActive.set(false); return }
        assistMutable.update { it.copy(status = AssistStatus.REQUESTING, advice = null, error = null, notice = null, inFlight = true) }
        advising.execute {
            try {
                val advice = editor.requestAdvice(ui.choices[ui.selected].index)
                if (!disposed.get()) commands.execute {
                    if (!disposed.get() && owner === editor && assistEpoch.get() == epoch && assistMutable.value.enabled) {
                        try { editor.inspectAdvice(advice); assistMutable.update { it.copy(status = AssistStatus.PROPOSAL_AVAILABLE, advice = advice, error = null) } }
                        catch (failure: Throwable) { val error = adviceFailure(failure); assistMutable.update { it.copy(status = error.first, error = error.second, advice = null) } }
                    }
                }
            } catch (failure: Throwable) {
                if (!disposed.get()) commands.execute {
                    if (!disposed.get() && owner === editor && assistEpoch.get() == epoch && assistMutable.value.enabled) {
                        val error = adviceFailure(failure); assistMutable.update { it.copy(status = error.first, error = error.second, advice = null) }
                    }
                }
            } finally { assistActive.set(false); assistMutable.update { it.copy(inFlight = false) } }
        }
    }
    fun cancelAdvice() { invalidateAdvice(AssistStatus.CANCELLED); assistMutable.update { it.copy(status = if (it.enabled) AssistStatus.CANCELLED else AssistStatus.IDLE) } }
    fun dismissAdvice() { invalidateAdvice(AssistStatus.REJECTED); assistMutable.update { it.copy(status = if (it.enabled) AssistStatus.REJECTED else AssistStatus.IDLE) } }
    fun useSuggestion() {
        val advice = assistMutable.value.advice ?: return
        val epoch = nextAssistEpoch() ?: return
        command {
            if (!disposed.get() && assistEpoch.get() == epoch) {
                try {
                    val editor = owner ?: throw EditorFailure(13, "Advice")
                    val choice = editor.inspectAdvice(advice)
                    // Navigation only. No update, preview, Apply or Agent execution.
                    editor.select(choice.toolIndex); editor.cancelAdvice()
                    if (!disposed.get() && assistEpoch.get() == epoch) assistMutable.update { it.copy(enabled = false, status = AssistStatus.IDLE, advice = null,
                        notice = "Manual tool opened. Set your preferred value, then choose Apply.") }
                } catch (failure: Throwable) { val error = adviceFailure(failure)
                    if (!disposed.get() && assistEpoch.get() == epoch) assistMutable.update { it.copy(status = error.first, error = error.second, advice = null) } }
            }
        }
    }
    private fun command(block: () -> Unit) {
        if (disposed.get()) return
        mutable.value = mutable.value.copy(busy = true, error = null)
        commands.execute { try { block(); refresh() } catch (e: Throwable) { refresh(message(e)) } }
    }
    fun openLast() = command {
        if (owner == null) {
            val name = preferences.getString("project", null)
            if (name != null && name.matches(Regex("[0-9a-f-]{36}"))) {
                owner = configured(File(root(), name).canonicalPath)
                preview()
            }
        }
    }
    fun reopenProject() { invalidateAdvice(); command { pendingValues.set(null); pendingPreview.set(null); owner?.close(); owner = null
        val name = preferences.getString("project", null)
        if (name != null && name.matches(Regex("[0-9a-f-]{36}"))) { owner = configured(File(root(), name).canonicalPath); preview() }
    } }
    /** Picker source is copied in a bounded stream to app-private staging, then
     * immutable AssetStore ingestion verifies it. The original URI is untouched.
     */
    fun importImage(uri: Uri) { invalidateAdvice(); command {
        val staging = File.createTempFile("import-", ".bin", app.cacheDir)
        try {
            val limit = CoreProbe().nativeImportLimit()
            checkNotNull(app.contentResolver.openInputStream(uri)).use { input ->
                staging.outputStream().use { output ->
                    val buffer = ByteArray(65536); var total = 0L
                    while (true) { val n = input.read(buffer); if (n < 0) break
                        total += n; if (total > limit) throw EditorFailure(8, "Import"); output.write(buffer, 0, n) }
                }
            }
            importFileOnWorker(staging)
        } finally { staging.delete() }
    } }
    internal fun importFile(file: File) { invalidateAdvice(); command { importFileOnWorker(file) } }
    private fun importFileOnWorker(file: File) {
        pendingPreview.set(null); pendingValues.set(null)
        val name = UUID.randomUUID().toString(); val directory = File(root(), name).apply { check(mkdir()) }
        val replacement = file.inputStream().use { configured(directory.canonicalPath, it, file.length()) }
        owner?.close(); owner = replacement
        preferences.edit { putString("project", name) }; preview()
    }
    fun select(index: Int) { invalidateAdvice(); command { pendingValues.set(null); owner?.select(index); pendingPreview.set(null); mutable.value = mutable.value.copy(previewLoading = false) } }
    fun update(values: List<Int>) {
        invalidateAdvice()
        if (disposed.get()) return
        val edit = mutable.value.core?.editId ?: return
        pendingValues.set(edit to values)
        drainValues()
    }
    private fun drainValues() {
        if (!updating.compareAndSet(false, true)) return
        commands.execute {
            val values = pendingValues.getAndSet(null)
            // Publish updated controls only after preview admission/loading has
            // been established. Otherwise observers can see new values with an
            // old image and a false "preview complete" flag.
            try { if (values != null) { owner?.update(values.second, values.first); preview(); refresh() } }
            catch (e: Throwable) { refresh(message(e)) }
            finally { updating.set(false); if (pendingValues.get() != null) drainValues() }
        }
    }
    fun action(action: Int) { invalidateAdvice(); command {
        applyPendingCapability()
        val finalValues = pendingValues.getAndSet(null)
        if (action == 1 && finalValues != null) owner?.update(finalValues.second, finalValues.first)
        pendingPreview.set(null); owner?.action(action); preview()
    } }
    fun retryPreview() = command { val editor = owner ?: return@command
        val core = editor.state(); if (core.active && !core.approvalPending) editor.update(core.values); preview() }
    fun interrupt() { invalidateAdvice(AssistStatus.CANCELLED); command { pendingValues.set(null); pendingPreview.set(null); owner?.action(4); mutable.value = mutable.value.copy(previewLoading = false) } }
    fun closeEditor() { setAssistMode(false); command { pendingValues.set(null); pendingPreview.set(null); owner?.close(); owner = null; mutable.value = mutable.value.copy(previewLoading = false) } }
    private fun preview() {
        val editor = owner ?: return
        applyPendingCapability()
        capabilities.sample()
        applyPendingCapability()
        pendingPreview.set(editor to editor.request())
        mutable.value = mutable.value.copy(previewLoading = true)
        if (!renderingNow.compareAndSet(false, true)) return
        rendering.execute {
            while (true) {
                val work = pendingPreview.getAndSet(null) ?: break
                try {
                    val candidate = work.first.render(work.second)
                    if (!disposed.get()) commands.execute {
                        if (owner === work.first && candidate != null) {
                            work.first.install(candidate) { bitmap -> mutable.value = mutable.value.copy(image = bitmap, previewLoading = false,
                                error = mutable.value.error?.takeUnless { it.startsWith("Preview failure") }) }
                        }
                    }
                } catch (e: Throwable) {
                    if (!disposed.get()) commands.execute {
                        if (owner === work.first && work.first.isRequested(work.second) && (e !is EditorFailure || e.status != 13))
                            mutable.value = mutable.value.copy(previewLoading = false, error = "Preview failure: ${message(e)}")
                    }
                }
            }
            renderingNow.set(false)
            if (!disposed.get() && pendingPreview.get() != null) commands.execute { preview() }
        }
    }
    override fun onCleared() {
        nextAssistEpoch()
        disposed.set(true)
        pendingValues.set(null); pendingPreview.set(null)
        commands.execute { capabilitySubscription?.close(); capabilitySubscription = null
            pendingCapability.set(null); owner?.close(); owner = null; rendering.shutdown(); advising.shutdown(); commands.shutdown() }
    }
}
