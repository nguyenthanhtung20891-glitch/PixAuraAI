package ai.pixaura.app

import ai.pixaura.bridge.ManualEditor
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp

/** Native-derived read-only records. No Plan parser, formatting math or edits. */
@Composable
internal fun PlanInspectionPanel(inspection: ManualEditor.PlanInspection) {
    Column(Modifier.testTag("review-action-list"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("${inspection.actions.size} planned action(s). Human approval required; inspection applied nothing.", Modifier.testTag("review-plan-count"))
        if (inspection.warnings and 1 != 0) Text("Multiple actions execute in this exact order. Order matters.", Modifier.testTag("review-order-warning"))
        for (action in inspection.actions) {
            Column(Modifier.testTag("review-action-${action.index}").semantics(mergeDescendants = true) {}) {
                Text("Action ${action.index + 1} of ${inspection.actions.size}: ${action.label}")
                Text("${action.toolId}; operation v${action.operationVersion}; parameters v${action.parameterVersion}")
                Text(action.parameters.joinToString("; ") { "${it.name} = ${it.raw} (${it.unit})" }, Modifier.testTag("auto-exact-values-${action.index}"))
                Text(action.parameters.joinToString("; ") { "${it.name}: range ${it.minimum} to ${it.maximum}; reference ${it.neutral}" }, Modifier.testTag("review-action-detail-${action.index}"))
                if (action.geometry) Text("Geometry change: dimensions or orientation may change. Use fit/toggle comparison.", Modifier.testTag("review-geometry-warning-${action.index}"))
            }
        }
    }
}
