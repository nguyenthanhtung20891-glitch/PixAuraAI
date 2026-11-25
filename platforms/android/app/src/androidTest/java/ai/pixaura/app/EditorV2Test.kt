package ai.pixaura.app

import ai.pixaura.bridge.BitmapLifetimeV2
import ai.pixaura.bridge.EditorContextV2
import ai.pixaura.bridge.EditorFailure
import ai.pixaura.bridge.EditorRuntimeV2
import ai.pixaura.bridge.ManualEditor
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.ParcelFileDescriptor
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.io.FileOutputStream
import java.util.UUID

/** Real JNI/native application bridge. Synthetic images/secrets; this is not a
 * physical cohort benchmark, genuine purchase or human accessibility result. */
@RunWith(AndroidJUnit4::class)
class EditorV2Test {
    @org.junit.Before fun waitForPriorApplicationOwner() = awaitShippingEditorRetirement()

    private fun fixture(file: File) {
        val bitmap = Bitmap.createBitmap(4000, 3000, Bitmap.Config.ARGB_8888)
        try {
            val row = IntArray(4000)
            for (y in 0 until 3000) {
                for (x in row.indices) row[x] = (255 shl 24) or (((x * 13 + y * 7) and 255) shl 16) or
                    (((x * 3 + y * 11) and 255) shl 8) or ((x + y * 5) and 255)
                bitmap.setPixels(row, 0, row.size, 0, y, row.size, 1)
            }
            FileOutputStream(file).use { assertTrue(bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)) }
        } finally { bitmap.recycle() }
    }
    @Test fun existingManualFacadeUsesSeparateV2OwnerAndStableGesture() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(context.noBackupFilesDir, "facade-v2-${UUID.randomUUID()}"); assertTrue(root.mkdir())
        try {
            val project = File(root, "project"); assertTrue(project.mkdir())
            val input = File(root, "source.png")
            val bitmap = Bitmap.createBitmap(16, 12, Bitmap.Config.ARGB_8888)
            try { bitmap.eraseColor(0xff8090a0.toInt()); FileOutputStream(input).use { assertTrue(bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)) } } finally { bitmap.recycle() }
            EditorRuntimeV2(root, ByteArray(32) { (it + 1).toByte() }).use { runtime ->
                val owner = ParcelFileDescriptor.open(input, ParcelFileDescriptor.MODE_READ_ONLY).use { ManualEditor.openV2(project, runtime, it, input.length()) }
                var revision: String
                owner.use {
                    val first = assertNotNullCandidate(owner.render(owner.request())); assertTrue(owner.install(first))
                    owner.select(4); val gesture = owner.state().editId; assertEquals(32, gesture.length)
                    owner.update(listOf(1000), gesture); assertEquals(gesture, owner.state().editId)
                    try { owner.update(listOf(2000), "0".repeat(32)); fail("stale gesture accepted") } catch (e: EditorFailure) { assertEquals(9, e.status) }
                    assertFalse(owner.install(first)); assertEquals(1, owner.state().revisionCount)
                    val originalRequest = owner.request(original = true)
                    val originalCandidate = assertNotNullCandidate(owner.render(originalRequest))
                    val originalPixels = IntArray(originalCandidate.bitmap.width * originalCandidate.bitmap.height)
                    val firstPixels = IntArray(first.bitmap.width * first.bitmap.height)
                    originalCandidate.bitmap.getPixels(originalPixels, 0, originalCandidate.bitmap.width, 0, 0, originalCandidate.bitmap.width, originalCandidate.bitmap.height)
                    first.bitmap.getPixels(firstPixels, 0, first.bitmap.width, 0, 0, first.bitmap.width, first.bitmap.height)
                    org.junit.Assert.assertArrayEquals(firstPixels, originalPixels)
                    val editedRequest = owner.request()
                    assertFalse(owner.isRequested(originalRequest)); assertFalse(owner.install(originalCandidate)); originalCandidate.close()
                    assertTrue(owner.isRequested(editedRequest))
                    val edited = assertNotNullCandidate(owner.render(editedRequest)); assertTrue(owner.install(edited)); edited.close()
                    assertEquals(1, owner.state().revisionCount)
                    val draft = assertNotNullCandidate(owner.render(owner.request())); assertTrue(owner.install(draft))
                    owner.action(1); assertEquals(2, owner.state().revisionCount); revision = owner.state().revisionId
                    val approved = assertNotNullCandidate(owner.render(owner.request())); assertTrue(owner.install(approved))
                    first.close(); draft.close() // All test display consumers retired.
                    owner.action(2); assertNotEquals(revision, owner.state().revisionId)
                    owner.action(3); assertEquals(revision, owner.state().revisionId)
                    val pending = requireNotNull(owner.prepareAdvisory(2)); owner.cancelAuto()
                    try { owner.requestAuto(2, pending); fail("cancelled prepared AI request delivered") } catch (e: EditorFailure) { assertEquals(13, e.status) }
                    val advice = owner.requestAdvice(2); val choice = owner.inspectAdvice(advice)
                    owner.select(choice.toolIndex); owner.cancelAdvice(); assertTrue(owner.state().active); owner.action(0)
                    val proposal = owner.requestAuto(2); val original = owner.prepareAutoReview(proposal)
                    val session = owner.beginReviewEditing(original)
                    owner.editReview(session, session.bindingIdentity, 1, value = 500)
                    try { owner.inspectReview(original); fail("stale review remained usable") } catch (e: EditorFailure) { assertEquals(9, e.status) }
                    assertNull(owner.editReview(session, session.bindingIdentity, 2)); assertEquals(3, session.state)
                    val review = requireNotNull(owner.editReview(session, session.bindingIdentity, 4))
                    owner.beginReviewPreview(review).use { request ->
                        owner.cancelReviewPreview(request)
                        try { owner.renderReviewPreview(request); fail("cancelled comparison rendered") } catch (e: EditorFailure) { assertEquals(13, e.status) }
                    }
                    owner.beginReviewPreview(review).use { request ->
                        owner.renderReviewPreview(request).use { pair ->
                            assertTrue(owner.installReviewPreview(pair) {}); owner.cancelReviewPreview(request)
                            assertFalse(owner.installReviewPreview(pair) {})
                        }
                    }
                    assertFalse(owner.inspectReview(review).actions.isEmpty())
                    val receipt = owner.applyAutoFromHuman(review); assertEquals(revision, receipt.sourceRevision)
                    revision = receipt.resultingRevision; assertEquals(revision, owner.state().revisionId)
                    try { owner.applyAutoFromHuman(review); fail("review Apply replay accepted") } catch (e: EditorFailure) { assertEquals(15, e.status) }
                    val result = owner.resultReview(receipt)
                    assertEquals(1, result.state); assertTrue(result.publicationVisible); assertTrue(result.hasPrevious)
                    assertFalse(result.geometryChanged); assertEquals(revision, result.resultRevision)
                    val resultBitmap = owner.renderResult(result, result.resultRevision)
                    assertTrue(resultBitmap.width in 1..256); owner.retireResult(resultBitmap)
                    owner.action(2)
                    try { owner.renderResult(result, result.resultRevision); fail("stale result rendered") } catch (e: EditorFailure) { assertEquals(9, e.status) }
                    assertEquals(2, owner.resultReview(receipt).state); owner.action(3)
                    assertEquals(3, owner.resultReview(null).state)
                    repeat(4) { owner.prepareExport(2, 0).close() }
                    val exported = owner.exportStaged(File(root, "facade-export.png"), 2, 0)
                    assertEquals(16, exported.width); assertEquals(12, exported.height)
                    assertEquals(revision, exported.native?.revision)
                    val preserved = File(root, "existing").apply { writeText("keep") }
                    try { owner.exportStaged(preserved, 2, 0); fail("existing file overwritten") } catch (_: Exception) { }
                    assertEquals("keep", preserved.readText())
                    val cancelledExport = owner.prepareExport(2, 0); owner.exportCancel()
                    try { owner.exportStaged(File(root, "cancelled.png"), 2, 0, cancelledExport); fail("cancelled export completed") } catch (e: EditorFailure) { assertEquals(13, e.status) }
                    assertFalse(File(root, "cancelled.png").exists())
                    assertEquals(revision, owner.state().revisionId)
                }
                ManualEditor.openV2(project, runtime).use { assertEquals(revision, it.state().revisionId) }
            }
        } finally { assertTrue(root.deleteRecursively()) }
    }
    @Test fun sharedRuntimeIgnoresDuplicateAndOlderOsCallbacks() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(context.noBackupFilesDir, "policy-v2-${UUID.randomUUID()}"); assertTrue(root.mkdir())
        try {
            val first = ai.pixaura.bridge.AndroidCapabilitySnapshot(
                ai.pixaura.bridge.AndroidStaticFacts(8, 36, 8L shl 30, 512L shl 20, 2),
                ai.pixaura.bridge.AndroidObservation(1, 1, false, 2, 512L shl 20, 1))
            EditorRuntimeV2(root, ByteArray(32) { (it + 1).toByte() }, first).use { runtime ->
                val original = runtime.strategy()
                assertEquals(original.epoch, runtime.updateCapability(first.observation).epoch)
                val critical = first.observation.copy(memory = 4, sequence = 2)
                val reduced = runtime.updateCapability(critical)
                assertTrue(reduced.cap < original.cap)
                assertEquals(reduced.epoch, runtime.updateCapability(critical).epoch)
                assertEquals(reduced.cap, runtime.updateCapability(first.observation).cap)
                assertTrue(reduced.forcedCpu); assertEquals(0, reduced.backend)
            }
        } finally { assertTrue(root.deleteRecursively()) }
    }
    private data class UiRetirement(val runtime: EditorRuntimeV2, val root: File, val weak: java.lang.ref.WeakReference<Bitmap>)
    private fun transferRealBitmapToUi(): UiRetirement {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(context.noBackupFilesDir, "ui-lifetime-v2-${UUID.randomUUID()}"); assertTrue(root.mkdir())
        val project = File(root, "project"); assertTrue(project.mkdir())
        val input = File(root, "source.png")
        val synthetic = Bitmap.createBitmap(16, 12, Bitmap.Config.ARGB_8888)
        try { synthetic.eraseColor(0xff8090a0.toInt()); FileOutputStream(input).use { assertTrue(synthetic.compress(Bitmap.CompressFormat.PNG, 100, it)) } } finally { synthetic.recycle() }
        val runtime = EditorRuntimeV2(root, ByteArray(32) { (it + 1).toByte() })
        try {
            val owner = ParcelFileDescriptor.open(input, ParcelFileDescriptor.MODE_READ_ONLY).use { ManualEditor.openV2(project, runtime, it, input.length(), uiBitmapLifetime = true) }
            val approved = assertNotNullCandidate(owner.render(owner.request())); assertTrue(owner.install(approved))
            val bitmap = approved.bitmap
            owner.select(4); owner.update(listOf(1000)); val draft = assertNotNullCandidate(owner.render(owner.request())); assertTrue(owner.install(draft))
            approved.close() // A cached approved image and UI reference remain valid.
            assertFalse(bitmap.isRecycled)
            owner.action(0); assertSame(bitmap, owner.displayedBitmap()); draft.close()
            owner.close(); assertFalse(bitmap.isRecycled)
            System.gc(); assertFalse(bitmap.isRecycled)
            try { runtime.close(); fail("UI Bitmap reservation released before retirement") } catch (e: EditorFailure) { assertEquals(15, e.status) }
            return UiRetirement(runtime, root, java.lang.ref.WeakReference(bitmap))
        } catch (failure: Throwable) { runCatching { runtime.close() }; root.deleteRecursively(); throw failure }
    }
    @Test fun uiBitmapReferencesOwnTheNativeReservationAfterEditorClose() {
        val retired = transferRealBitmapToUi()
        try {
            // Actual collection, not a manually enqueued/fabricated release.
            val deadline = android.os.SystemClock.elapsedRealtime() + 5000
            while ((retired.weak.get() != null || BitmapLifetimeV2.outstanding() != 0) && android.os.SystemClock.elapsedRealtime() < deadline) {
                System.gc(); System.runFinalization(); Thread.sleep(20)
            }
            assertNull(retired.weak.get()); assertEquals(0, BitmapLifetimeV2.outstanding())
            retired.runtime.close()
        } finally { assertTrue(retired.root.deleteRecursively()) }
    }
    private fun assertNotNullCandidate(value: ManualEditor.Candidate?): ManualEditor.Candidate = requireNotNull(value)

    @Test fun real12mpNativePublicationHistoryAIAndFullResolutionExport() {
        val app = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(app.noBackupFilesDir, "editor-v2-${UUID.randomUUID()}"); assertTrue(root.mkdir())
        try {
            val project = File(root, "project"); assertTrue(project.mkdir())
            val input = File(root, "synthetic-12mp.png"); fixture(input)
            val syntheticKey = ByteArray(32) { (it + 1).toByte() }
            EditorRuntimeV2(root, syntheticKey).use { runtime ->
                val owner = ParcelFileDescriptor.open(input, ParcelFileDescriptor.MODE_READ_ONLY).use {
                    EditorContextV2(runtime, project, it, input.length())
                }
                val approved: String
                var genericDelivery = ""
                owner.use {
                    assertEquals(1, owner.state().revisionCount)
                    assertEquals(1, runtime.commercialState().photos); assertEquals(0, runtime.commercialState().access)
                    owner.preview(64, 48, original = true).use { frame ->
                        assertEquals(4000, frame.sourceWidth); assertEquals(3000, frame.sourceHeight)
                        assertEquals(64, frame.bitmap.width); assertTrue(frame.isCurrent)
                        owner.select(4); owner.update(listOf(1000)); assertFalse(frame.isCurrent)
                    }
                    owner.preview(64, 48).use { assertEquals(64, it.bitmap.width) }
                    assertEquals(1, owner.state().revisionCount); owner.applyFromHuman()
                    assertEquals(2, owner.state().revisionCount); assertEquals(1, runtime.commercialState().access)
                    val manual = owner.state().revision
                    owner.undo(); assertNotEquals(manual, owner.state().revision)
                    owner.redo(); assertEquals(manual, owner.state().revision)
                    val initialUses = runtime.commercialState().ai
                    val cancelledRequest = owner.beginAdvisory(2)
                    owner.cancelAdvisoryRequest(cancelledRequest)
                    try { owner.runAdvisory(cancelledRequest); fail("cancelled request delivered") }
                    catch (e: EditorFailure) { assertEquals(13, e.status) }
                    assertEquals(initialUses, runtime.commercialState().ai)
                    val abandoned = owner.requestAdvisory(2)
                    owner.preview(64, 48, advice = abandoned).use { frame ->
                        assertTrue(frame.isCurrent); owner.cancelAdvisory(abandoned)
                        assertFalse(frame.isCurrent)
                    }
                    owner.select(4); owner.update(listOf(1000))
                    try { owner.cancelAdvisory(abandoned); fail("old review cancelled a newer gesture") }
                    catch (e: EditorFailure) { assertEquals(9, e.status) }
                    assertEquals(4, owner.state().selected); owner.cancel()
                    var advice = owner.requestAdvisory(2)
                    assertEquals(1, owner.inspectReview(advice).actions.size)
                    try { owner.editAdvisory(advice, "{".toByteArray()); fail("malformed edit accepted") } catch (_: EditorFailure) {}
                    try { owner.applyAdvisoryFromHuman(advice); fail("failed edit retained approval eligibility") }
                    catch (e: EditorFailure) { assertEquals(13, e.status) }
                    advice = owner.restoreReview(advice)
                    advice = owner.editReviewCommand(advice, 1, value = 500)
                    assertEquals(500, owner.inspectReview(advice).actions.first().parameters.first().raw)
                    advice = owner.editReviewCommand(advice, 4)
                    owner.beginExport(2, 93).use { task ->
                        task.cancel()
                        val cancelled = File(root, "cancelled-export.png"); assertTrue(cancelled.createNewFile())
                        ParcelFileDescriptor.open(cancelled, ParcelFileDescriptor.MODE_READ_WRITE).use { fd ->
                            try { task.run(fd); fail("cancelled export completed") }
                            catch (e: EditorFailure) { assertEquals(13, e.status) }
                        }
                        assertEquals(0L, cancelled.length())
                    }
                    assertTrue(owner.advisoryContent(advice, true).isNotEmpty())
                    owner.beginReviewPreview(advice, 64, 48).use { task ->
                        task.cancel()
                        try { task.render(); fail("cancelled comparison rendered") }
                        catch (e: EditorFailure) { assertEquals(13, e.status) }
                    }
                    owner.beginReviewPreview(advice, 64, 48).use { task ->
                        task.render().use { pair ->
                            assertTrue(pair.isCurrent); task.cancel(); assertFalse(pair.isCurrent)
                        }
                    }
                    assertTrue(owner.advisoryContent(advice, true).isNotEmpty())
                    val empty = owner.editAdvisory(advice, "{\"operations\":[]}".toByteArray())
                    try { owner.applyAdvisoryFromHuman(empty); fail("empty review authorized") }
                    catch (e: EditorFailure) { assertEquals(7, e.status) }
                    val restored = owner.restoreReview(empty)
                    val publication = owner.applyAdvisoryReceiptFromHuman(restored)
                    assertEquals(manual, publication.parent)
                    assertEquals(owner.state().revision, publication.revision)
                    assertEquals(32, publication.event.length); assertEquals(64, publication.digest.length)
                    assertFalse(publication.authenticatedUtc)
                    assertEquals(3, owner.state().revisionCount); approved = owner.state().revision
                    try { owner.applyAdvisoryFromHuman(restored); fail("approval replay accepted") }
                    catch (e: EditorFailure) { assertEquals(9, e.status) }
                    val binding = owner.agentBytes(1).toString(Charsets.UTF_8).trim()
                    val actions = "{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":101},\"tool_id\":\"pixaura.exposure\"},{\"action_index\":1,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1},\"tool_id\":\"pixaura.rotate\"}"
                    val plan = owner.agentBytes(2, "{\"actions\":[$actions],\"binding\":$binding,\"plan_version\":1}\n".toByteArray())
                    val cancelledPlan = owner.beginPlanCandidate(plan); owner.cancelAdvisoryRequest(cancelledPlan)
                    try { owner.runAdvisory(cancelledPlan); fail("cancelled Plan delivered") } catch (e: EditorFailure) { assertEquals(13, e.status) }
                    val generic = owner.runAdvisory(owner.beginPlanCandidate(plan)); genericDelivery = generic.delivery
                    assertEquals(2, owner.inspectReview(generic).actions.size)
                    assertTrue(owner.inspectReview(generic).actions[1].geometry)
                    for (format in 1..2) {
                        val output = File(root, "full-$format"); assertTrue(output.createNewFile())
                        ParcelFileDescriptor.open(output, ParcelFileDescriptor.MODE_READ_WRITE).use {
                            val exported = owner.exportSavedObserved(it, format, 93)
                            assertTrue(exported.bytes > 0); assertEquals(approved, exported.revision)
                            assertEquals(4000, exported.width); assertEquals(3000, exported.height)
                            assertEquals(3, owner.state().revisionCount)
                        }
                        val dimensions = BitmapFactory.Options().apply { inJustDecodeBounds = true }
                        BitmapFactory.decodeFile(output.path, dimensions)
                        assertEquals(4000, dimensions.outWidth); assertEquals(3000, dimensions.outHeight)
                    }
                }
                EditorContextV2(runtime, project).use { reopened ->
                    assertEquals(approved, reopened.state().revision); assertEquals(3, reopened.state().revisionCount)
                    assertTrue(reopened.pendingAdvisories().contains(genericDelivery))
                    val recovered = reopened.restoreAdvisory(genericDelivery)
                    assertEquals(2, reopened.inspectReview(recovered).actions.size)
                    assertEquals(approved, reopened.state().revision)
                    reopened.cancelAdvisory(recovered)
                }
            }
            val commercial = File(root, "commercial-v2.sqlite")
            val savedCommercial = commercial.readBytes()
            EditorRuntimeV2.recovery(root).use { recovery ->
                try { recovery.commercialState(); fail("recovery granted commercial access") } catch (e: EditorFailure) { assertEquals(3003, e.status) }
                EditorContextV2(recovery, project).use { recovered ->
                    assertTrue(recovered.state().recoveryOnly); assertTrue(recovered.state().commercialUnavailable)
                    assertEquals(3, recovered.state().revisionCount)
                    try { recovered.select(4); fail("recovery granted NEW editing") } catch (e: EditorFailure) { assertEquals(3003, e.status) }
                    recovered.preview(64, 48).use { assertEquals(4000, it.sourceWidth); assertTrue(it.isCurrent) }
                    val output = File(root, "key-loss-recovery.png"); assertTrue(output.createNewFile())
                    ParcelFileDescriptor.open(output, ParcelFileDescriptor.MODE_READ_WRITE).use { assertEquals(4000, recovered.exportSavedObserved(it, 2, 0).width) }
                }
            }
            assertArrayEquals(savedCommercial, commercial.readBytes())
            syntheticKey.fill(0)
        } finally { assertTrue(root.deleteRecursively()) }
    }
}
