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
    else -> 0
}

@Composable
internal fun presented(canonical: String): String {
    val id = presentationId(canonical)
    return if (id == 0) canonical else stringResource(id)
}
