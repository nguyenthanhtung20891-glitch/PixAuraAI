plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

val stagedRuntimePool = tasks.register<Sync>("stageRuntimePoolSource") {
    from("../app/src/main/java/ai/pixaura/app/NativeEditorRuntimePoolV2.kt")
    into(layout.buildDirectory.dir("staged-runtime-pool/ai/pixaura/app"))
}
tasks.named("preBuild") { dependsOn(stagedRuntimePool) }
android {
    namespace = "ai.pixaura.commerce.integration"
    compileSdk = 36
    defaultConfig { minSdk = 26 }
    sourceSets["main"].java.srcDir("../app/src/main/java/ai/pixaura/app/commerce")
    // Native store ingress compiles against the checked Kotlin bridge (no JNI library is
    // linked here) and the one runtime-pool type its owner leases. Nothing else of :app.
    sourceSets["main"].java.srcDir("../bridge/src/main/kotlin")
    sourceSets["main"].java.srcDir(layout.buildDirectory.dir("staged-runtime-pool"))
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    lint { warningsAsErrors = true; abortOnError = true; disable += setOf("AndroidGradlePluginVersion", "GradleDependency") }
}
kotlin { compilerOptions { jvmTarget.set(org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17); allWarningsAsErrors.set(true) } }
// Compile proof only. The shipping :app does NOT depend on this module or Billing.
// No transitive telemetry/location modules are admitted into either artifact.
dependencies { compileOnly("com.android.billingclient:billing:9.1.0") { isTransitive = false } }
dependencyLocking { lockAllConfigurations() }

// No tests live in this compile-only artifact. Runtime construction/crypto/cache
// tests run in :app's test APK; no SDK is linked into the shipping application.
androidComponents { beforeVariants { it.enableAndroidTest = false } }
