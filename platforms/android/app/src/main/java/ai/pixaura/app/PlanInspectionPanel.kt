package ai.pixaura.app

import ai.pixaura.bridge.ManualEditor
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp
import androidx.compose.ui.res.pluralStringResource

/** Native-derived read-only records. No Plan parser, formatting math or edits. */
@Composable
internal fun PlanInspectionPanel(inspection: ManualEditor.PlanInspection) {
    Column(Modifier.testTag("review-action-list"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text(pluralStringResource(R.plurals.phase11_plan_count, inspection.actions.size, inspection.actions.size), Modifier.testTag("review-plan-count"))
        if (inspection.warnings and 1 != 0) Text(presented("Multiple actions execute in this exact order. Order matters."), Modifier.testTag("review-order-warning"))
        for (action in inspection.actions) {
            val position = presentedFormat(R.string.phase11_action_position, action.index + 1, inspection.actions.size, action.label)
            val identity = "${action.toolId}; operation v${action.operationVersion}; parameters v${action.parameterVersion}"
            val exact = action.parameters.joinToString("; ") { "${it.name} = ${it.raw} (${it.unit})" }
            val detail = action.parameters.joinToString("; ") { "${it.name}: range ${it.minimum} to ${it.maximum}; reference ${it.neutral}" }
            val warning = presented("Geometry change: dimensions or orientation may change. Use fit/toggle comparison.")
            val spoken = if (action.geometry) listOf(position, identity, exact, detail, warning) else listOf(position, identity, exact, detail)
            // Keep exact-value/detail nodes addressable in the default semantics
            // tree while exposing the complete row to accessibility services.
            Column(Modifier.testTag("review-action-${action.index}").semantics {
                contentDescription = spoken.joinToString("; ")
            }) {
                Text(position)
                Text(identity)
                Text(exact, Modifier.testTag("auto-exact-values-${action.index}"))
                Text(detail, Modifier.testTag("review-action-detail-${action.index}"))
                if (action.geometry) Text(warning, Modifier.testTag("review-geometry-warning-${action.index}"))
            }
        }
    }
}
