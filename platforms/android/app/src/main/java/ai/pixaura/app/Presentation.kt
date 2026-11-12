package ai.pixaura.app

import androidx.annotation.StringRes
import androidx.compose.runtime.Composable
import androidx.compose.ui.res.stringResource

@StringRes
internal fun presentationId(canonical: String): Int = when (canonical) {
    "Import cancelled. Open JPEG or PNG to try again." -> R.string.import_cancelled
    "Share sheet closed. Delivery to another app is not confirmed." -> R.string.share_closed
    "Share cancelled" -> R.string.share_cancelled
    "Save cancelled" -> R.string.save_cancelled
    "Export failed" -> R.string.export_failed
    "Export failed. Nothing was saved." -> R.string.export_failed_saved_nothing
    "Saved" -> R.string.saved
    "Choose a destination" -> R.string.choose_destination
    "Opening the share sheet. Delivery is not confirmed." -> R.string.share_opening
    "Exporting" -> R.string.exporting
    "Photo library permission was denied. Nothing was saved." -> R.string.photos_denied
    "Open an image before export" -> R.string.open_before_export
    "Stale review" -> R.string.stale_review
    "Stale edit; reopen the committed project" -> R.string.stale_edit
    "Preview cancelled. The Plan remains inspectable if current." -> R.string.preview_cancelled
    "After Preview — not committed" -> R.string.after_preview
    "Before — bound source revision" -> R.string.before_source
    "Before / After Preview split" -> R.string.preview_split
    "Committed result" -> R.string.committed_result
    "Result review is stale. Ordinary history changed." -> R.string.result_stale
    "Outcome unconfirmed. Ordinary history remains authoritative." -> R.string.result_unconfirmed
    "Receipt revision is absent from ordinary history." -> R.string.result_absent
    "No committed result." -> R.string.result_none
    "Cancelled" -> R.string.cancelled
    "Failed" -> R.string.failed
    "Stale" -> R.string.stale
    "Save to Photos" -> R.string.save_to_photos
    "Save to a file" -> R.string.save_to_file
    "Share" -> R.string.share
    "Cancel export" -> R.string.cancel_export
    "Export" -> R.string.export_title
    "Restore original review content — not document undo" -> R.string.restore_review
    "Undo document" -> R.string.undo_document
    "Redo document" -> R.string.redo_document
    "Home" -> R.string.phase11_3a78695388
    "Editor" -> R.string.phase11_936a161c39
    "Projects" -> R.string.phase11_04e2a9728a
    "Settings" -> R.string.phase11_74a883a037
    "Manual" -> R.string.phase11_b0b9fe24ff
    "AI Assist" -> R.string.phase11_8d36d81711
    "AI Auto-Edit" -> R.string.phase11_5bce52c193
    "Open Manual editor" -> R.string.phase11_2c0a7d2aee
    "Open Assisted editor" -> R.string.phase11_36971679f1
    "Open AI editor" -> R.string.phase11_3cc77f5151
    "Open Projects" -> R.string.phase11_3f5599a9fe
    "Open Settings" -> R.string.phase11_3f940108cb
    "Back to Home" -> R.string.phase11_eec7842677
    "Your projects will appear here." -> R.string.phase11_a2ddfca116
    "Local-first photo editing. Application shell preview." -> R.string.phase11_945c960945
    "Offline by default. No account, analytics or photo access requested." -> R.string.phase11_703b9aaeda
    "Geometry" -> R.string.phase11_fa9da5754c
    "Tone/Color" -> R.string.phase11_a7bc9091e0
    "Detail" -> R.string.phase11_fb5f27d545
    "Filters/Presets" -> R.string.phase11_a4a4d16f21
    "Blur" -> R.string.phase11_cd83e6731d
    "Brightness" -> R.string.phase11_ec811d30a8
    "Contrast" -> R.string.phase11_08795ba031
    "Crop" -> R.string.phase11_98caf7c43c
    "Exposure" -> R.string.phase11_cc5454a4b7
    "Highlights" -> R.string.phase11_6e517828c3
    "Rotate" -> R.string.phase11_c3613b1704
    "Saturation" -> R.string.phase11_993f024b65
    "Shadows" -> R.string.phase11_2a19bdc09a
    "Sharpen" -> R.string.phase11_ac6f0be127
    "Temperature" -> R.string.phase11_b958ce8b87
    "No presets available. The shipping catalog is empty." -> R.string.phase11_91ccf2b387
    "Reset to default" -> R.string.phase11_bc5b45ae7b
    "Preview crop" -> R.string.phase11_5c3d1e62b5
    "Crop values are millionths of the current extent. Invalid rectangles are rejected." -> R.string.phase11_5f630d2afa
    "Loading or saving…" -> R.string.phase11_7783b255ef
    "Rendering preview; changes are not saved until Apply" -> R.string.phase11_c574fe4a4a
    "Image preview" -> R.string.phase11_f09247433b
    "Current manual preview" -> R.string.phase11_556967ee02
    "Editor busy" -> R.string.phase11_7e3a25f5df
    "Save the committed revision to Photos" -> R.string.phase11_8c6b5cd53c
    "Save the committed revision to a file" -> R.string.phase11_9c88214fe5
    "Share the committed revision" -> R.string.phase11_29a5b13431
    "Saves the committed revision. Export does not change history." -> R.string.phase11_94f68b20ec
    "JPEG quality 1…100" -> R.string.phase11_9eb8d67dd3
    "Choose a precise adjustment. Local reference guidance does not analyze your image or apply edits." -> R.string.phase11_9ec26c3cb9
    "Finish or cancel your current manual adjustment before requesting advice." -> R.string.phase11_912f83afdc
    "Getting advice…" -> R.string.phase11_5a0d1a7944
    "Request advice" -> R.string.phase11_ec592ee72d
    "Cancel advice" -> R.string.phase11_412e0641ed
    "Dismiss" -> R.string.phase11_48845bff33
    "Use suggestion" -> R.string.phase11_4c363f056a
    "Open the manual tool and choose your value. Nothing has been applied." -> R.string.phase11_6b4161af33
    "Advice cancelled. You can keep editing manually." -> R.string.phase11_079b712434
    "Advice dismissed. Your image is unchanged." -> R.string.phase11_c14826f7d0
    "Advice expired after an editing change. Request new guidance when ready." -> R.string.phase11_b44b7439ee
    "Local reference planning does not analyze your image. Review exact actions, then choose Apply yourself." -> R.string.phase11_5ed737eeb1
    "Finish or cancel your manual adjustment before requesting a proposal." -> R.string.phase11_93782c1ff1
    "Prepare proposal" -> R.string.phase11_7eea0636b9
    "Cancel proposal" -> R.string.phase11_15fe0613b5
    "Reject proposal" -> R.string.phase11_8c3acdbb3b
    "Cancel execution" -> R.string.phase11_485d534702
    "Requesting and validating proposal…" -> R.string.phase11_5f14c39169
    "Approving and applying through the editing engine…" -> R.string.phase11_b58fc52c70
    "No executable review — restore original or request a new proposal" -> R.string.phase11_7258cc73f6
    "Ready for human approval" -> R.string.phase11_bdd1555740
    "Reviewed plan" -> R.string.phase11_3ceb6e808d
    "Proposal cancelled. Nothing was applied." -> R.string.phase11_db9bc6aa4f
    "Proposal dismissed. Nothing was applied." -> R.string.phase11_46e11c5f52
    "Proposal expired after an editing change." -> R.string.phase11_2a7d46b3c0
    "Before / After Preview" -> R.string.phase11_7af9179182
    "Disposable display projection. Preview does not apply edits or guarantee Apply will succeed." -> R.string.phase11_a6918d9228
    "Geometry changed. Aspect-preserving fit/toggle is available; spatial split is unavailable." -> R.string.phase11_e9ed4187ad
    "Preview reviewed plan" -> R.string.phase11_adc157ae7a
    "Preparing preview…" -> R.string.phase11_11e66e1bb9
    "Split" -> R.string.phase11_32afaa7843
    "Reset comparison to fit" -> R.string.phase11_933fd5de32
    "Preview is stale. Request a freshly validated proposal." -> R.string.phase11_d158daf880
    "Comparison split position" -> R.string.phase11_fa1aef98b4
    "Comparison zoom" -> R.string.phase11_6e9f2697bc
    "Comparison horizontal pan" -> R.string.phase11_cf7facf583
    "Comparison vertical pan" -> R.string.phase11_58783b00e9
    "Plan inspection stale. Request a new review." -> R.string.phase11_3a0dd04f46
    "Plan inspection unavailable. No actionable review; request a new proposal explicitly." -> R.string.phase11_83aa7fc1c7
    "Multiple actions execute in this exact order. Order matters." -> R.string.phase11_f41c709b97
    "Geometry change: dimensions or orientation may change. Use fit/toggle comparison." -> R.string.phase11_6c140ae2fd
    "Unvalidated input — Validate value or revert this field before Apply." -> R.string.phase11_b4b16092eb
    "Validating review edit…" -> R.string.phase11_293c48d130
    "Geometry changed. Fitted comparison is not a pixel-aligned split." -> R.string.phase11_14c8be794b
    "Previous" -> R.string.phase11_a57b08a480
    "Current Result" -> R.string.phase11_93037e4317
    "Back to editing" -> R.string.phase11_69a02c64d5
    else -> 0
}

@Composable
internal fun presented(canonical: String): String {
    val id = presentationId(canonical)
    return if (id == 0) canonical else stringResource(id)
}

/** Presentation-only formatting; native identifiers and raw values stay verbatim. */
@Composable
internal fun presentedFormat(id: Int, vararg values: Any): String = stringResource(id, *values)
