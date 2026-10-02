package ai.pixaura.app

import androidx.compose.ui.test.assertIsDisplayed
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import ai.pixaura.bridge.CoreProbe
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test

class ShellTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()

    @Test fun realJniBoundaryAndShellNavigation() {
        assertEquals(1, CoreProbe().nativeAbiVersion())
        compose.waitUntil(10_000) {
            compose.onAllNodes(androidx.compose.ui.test.hasText("Shared core ready · ABI 1"))
                .fetchSemanticsNodes().isNotEmpty()
        }
        compose.onNodeWithText("Shared core ready · ABI 1").assertIsDisplayed()
        for (destination in listOf("Projects", "Settings")) {
            compose.onNodeWithText("Open $destination").performClick()
            compose.onNodeWithText(destination).assertIsDisplayed()
            compose.onNodeWithText("Back to Home").performClick()
        }
        compose.onNodeWithText("Open Manual editor").performClick()
        compose.onNodeWithText("Editor").assertIsDisplayed()
        compose.activityRule.scenario.recreate()
        compose.onNodeWithText("Editor").assertIsDisplayed()
        compose.onNodeWithText("Manual session · Editing tools arrive in a later phase.").assertIsDisplayed()
    }
}
