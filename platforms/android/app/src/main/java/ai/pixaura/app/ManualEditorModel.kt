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
import java.io.InputStream
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow

data class EditorUiState(val tools: List<ManualTool> = emptyList(), val core: ManualState? = null,
    val image: Bitmap? = null, val busy: Boolean = false, val previewLoading: Boolean = false, val error: String? = null)

/** Two fixed lanes, one coalesced input and one coalesced preview request.
 * Native lifecycle/history is authoritative; no platform operation/history model.
 */
class ManualEditorModel(application: Application) : AndroidViewModel(application) {
    private val commands = Executors.newSingleThreadExecutor()
    private val rendering = Executors.newSingleThreadExecutor()
    private var owner: ManualEditor? = null
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
    fun reopenProject() = command { pendingValues.set(null); pendingPreview.set(null); owner?.close(); owner = null
        val name = preferences.getString("project", null)
        if (name != null && name.matches(Regex("[0-9a-f-]{36}"))) { owner = configured(File(root(), name).canonicalPath); preview() }
    }
    /** Picker source is copied in a bounded stream to app-private staging, then
     * immutable AssetStore ingestion verifies it. The original URI is untouched.
     */
    fun importImage(uri: Uri) = command {
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
    }
    internal fun importFile(file: File) = command { importFileOnWorker(file) }
    private fun importFileOnWorker(file: File) {
        pendingPreview.set(null); pendingValues.set(null)
        val name = UUID.randomUUID().toString(); val directory = File(root(), name).apply { check(mkdir()) }
        val replacement = file.inputStream().use { configured(directory.canonicalPath, it, file.length()) }
        owner?.close(); owner = replacement
        preferences.edit { putString("project", name) }; preview()
    }
    fun select(index: Int) = command { pendingValues.set(null); owner?.select(index); pendingPreview.set(null); mutable.value = mutable.value.copy(previewLoading = false) }
    fun update(values: List<Int>) {
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
    fun action(action: Int) = command {
        applyPendingCapability()
        val finalValues = pendingValues.getAndSet(null)
        if (action == 1 && finalValues != null) owner?.update(finalValues.second, finalValues.first)
        pendingPreview.set(null); owner?.action(action); preview()
    }
    fun retryPreview() = command { val editor = owner ?: return@command
        val core = editor.state(); if (core.active && !core.approvalPending) editor.update(core.values); preview() }
    fun interrupt() = command { pendingValues.set(null); pendingPreview.set(null); owner?.action(4); mutable.value = mutable.value.copy(previewLoading = false) }
    fun closeEditor() = command { pendingValues.set(null); pendingPreview.set(null); owner?.close(); owner = null; mutable.value = mutable.value.copy(previewLoading = false) }
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
        disposed.set(true)
        pendingValues.set(null); pendingPreview.set(null)
        commands.execute { capabilitySubscription?.close(); capabilitySubscription = null
            pendingCapability.set(null); owner?.close(); owner = null; rendering.shutdown(); commands.shutdown() }
    }
}
