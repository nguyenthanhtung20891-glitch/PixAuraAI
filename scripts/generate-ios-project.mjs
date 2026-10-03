// Dependency-free, deterministic Xcode project generation. Commit generated output.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const objects = [];
let counter = 0;
const id = () => (++counter).toString(16).padStart(24, '0').toUpperCase();
const add = (key, value) => objects.push(`${key} = { ${value} };`);
const project = id(), mainGroup = id(), productsGroup = id(), packageRef = id();
const appTarget = id(), testTarget = id(), uiTarget = id();
const appProduct = id(), testProduct = id(), uiProduct = id();
const appPackage = id(), testPackage = id();
add(packageRef, 'isa = XCLocalSwiftPackageReference; relativePath = ../../packages/core;');
for (const key of [appPackage, testPackage]) {
  add(key, `isa = XCSwiftPackageProductDependency; package = ${packageRef}; productName = PixAuraCore;`);
}
const groups = [];
function sources(directory, files) {
  const group = id(), phase = id(), buildFiles = [], refs = [];
  for (const file of files) {
    const ref = id(), build = id();
    refs.push(ref); buildFiles.push(build);
    add(ref, `isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = ${file}; sourceTree = "<group>";`);
    add(build, `isa = PBXBuildFile; fileRef = ${ref};`);
  }
  add(group, `isa = PBXGroup; children = (${refs.join(',')}); path = ${directory}; sourceTree = "<group>";`);
  add(phase, `isa = PBXSourcesBuildPhase; buildActionMask = 2147483647; files = (${buildFiles.join(',')}); runOnlyForDeploymentPostprocessing = 0;`);
  groups.push(group);
  return { group, phase };
}
const appSources = sources('PixAuraAI', ['PixAuraApp.swift', 'ShellView.swift', 'ShellModel.swift', 'ShellState.swift']);
const tests = sources('PixAuraAITests', ['ShellTests.swift']);
const uiTests = sources('PixAuraAIUITests', ['ShellUITests.swift']);
const privacyRef = id(), privacyBuild = id(), resources = id();
add(privacyRef, 'isa = PBXFileReference; lastKnownFileType = text.xml; path = PixAuraAI/PrivacyInfo.xcprivacy; sourceTree = "<group>";');
add(privacyBuild, `isa = PBXBuildFile; fileRef = ${privacyRef};`);
add(resources, `isa = PBXResourcesBuildPhase; buildActionMask = 2147483647; files = (${privacyBuild}); runOnlyForDeploymentPostprocessing = 0;`);
function frameworks(product) {
  const phase = id(), build = id();
  add(build, `isa = PBXBuildFile; productRef = ${product};`);
  add(phase, `isa = PBXFrameworksBuildPhase; buildActionMask = 2147483647; files = (${build}); runOnlyForDeploymentPostprocessing = 0;`);
  return phase;
}
function configs(settings, projectConfig = false) {
  const list = id(), debug = id(), release = id();
  for (const [key, name] of [[debug, 'Debug'], [release, 'Release']]) {
    const extra = projectConfig
      ? `SWIFT_OPTIMIZATION_LEVEL = "${name === 'Debug' ? '-Onone' : '-O'}"; ${name === 'Debug' ? 'ENABLE_TESTABILITY = YES; SWIFT_ACTIVE_COMPILATION_CONDITIONS = DEBUG;' : ''}`
      : '';
    add(key, `isa = XCBuildConfiguration; name = ${name}; buildSettings = { ${settings} ${extra} };`);
  }
  add(list, `isa = XCConfigurationList; buildConfigurations = (${debug},${release}); defaultConfigurationIsVisible = 0; defaultConfigurationName = Release;`);
  return list;
}
const projectConfigs = configs('ARCHS = "$(ARCHS_STANDARD)"; IPHONEOS_DEPLOYMENT_TARGET = 16.0; SDKROOT = iphoneos; SWIFT_VERSION = 5.0; CLANG_CXX_LANGUAGE_STANDARD = "c++17"; CLANG_ENABLE_MODULES = YES; SWIFT_TREAT_WARNINGS_AS_ERRORS = YES; GCC_TREAT_WARNINGS_AS_ERRORS = YES;', true);
function target(key, name, product, phase, packageProduct, extraSettings, kind, dependencies = []) {
  const config = configs(`PRODUCT_NAME = "$(TARGET_NAME)"; PRODUCT_BUNDLE_IDENTIFIER = ai.pixaura.${name}; GENERATE_INFOPLIST_FILE = YES; CURRENT_PROJECT_VERSION = 1; MARKETING_VERSION = 0.1.0; TARGETED_DEVICE_FAMILY = "1,2"; SUPPORTED_PLATFORMS = "iphoneos iphonesimulator"; CODE_SIGN_STYLE = Automatic; ${extraSettings}`);
  const phases = [phase];
  if (packageProduct) phases.push(frameworks(packageProduct));
  if (key === appTarget) phases.push(resources);
  add(key, `isa = PBXNativeTarget; buildConfigurationList = ${config}; buildPhases = (${phases.join(',')}); buildRules = (); dependencies = (${dependencies.join(',')}); name = ${name}; productName = ${name}; productReference = ${product}; productType = "com.apple.product-type.${kind}"; packageProductDependencies = (${packageProduct ?? ''});`);
}
function dependency() {
  const proxy = id(), dep = id();
  add(proxy, `isa = PBXContainerItemProxy; containerPortal = ${project}; proxyType = 1; remoteGlobalIDString = ${appTarget}; remoteInfo = PixAuraAI;`);
  add(dep, `isa = PBXTargetDependency; target = ${appTarget}; targetProxy = ${proxy};`);
  return dep;
}
target(appTarget, 'PixAuraAI', appProduct, appSources.phase, appPackage,
  'INFOPLIST_KEY_CFBundleDisplayName = PixAuraAI; INFOPLIST_KEY_LSRequiresIPhoneOS = YES; INFOPLIST_KEY_UIApplicationSceneManifest_Generation = YES; INFOPLIST_KEY_UILaunchScreen_Generation = YES; INFOPLIST_KEY_UISupportedInterfaceOrientations = "UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight";', 'application');
target(testTarget, 'PixAuraAITests', testProduct, tests.phase, testPackage,
  'TEST_HOST = "$(BUILT_PRODUCTS_DIR)/PixAuraAI.app/$(BUNDLE_EXECUTABLE_FOLDER_PATH)/PixAuraAI"; BUNDLE_LOADER = "$(TEST_HOST)";', 'bundle.unit-test', [dependency()]);
target(uiTarget, 'PixAuraAIUITests', uiProduct, uiTests.phase, null,
  'TEST_TARGET_NAME = PixAuraAI;', 'bundle.ui-testing', [dependency()]);
for (const [key, name, type] of [[appProduct, 'PixAuraAI.app', 'wrapper.application'], [testProduct, 'PixAuraAITests.xctest', 'wrapper.cfbundle'], [uiProduct, 'PixAuraAIUITests.xctest', 'wrapper.cfbundle']]) {
  add(key, `isa = PBXFileReference; explicitFileType = ${type}; path = ${name}; sourceTree = BUILT_PRODUCTS_DIR;`);
}
add(productsGroup, `isa = PBXGroup; children = (${appProduct},${testProduct},${uiProduct}); name = Products; sourceTree = "<group>";`);
add(mainGroup, `isa = PBXGroup; children = (${groups.join(',')},${privacyRef},${productsGroup}); sourceTree = "<group>";`);
add(project, `isa = PBXProject; attributes = { BuildIndependentTargetsInParallel = YES; LastUpgradeCheck = 2600; }; buildConfigurationList = ${projectConfigs}; compatibilityVersion = "Xcode 14.0"; developmentRegion = en; hasScannedForEncodings = 0; knownRegions = (en,Base); mainGroup = ${mainGroup}; productRefGroup = ${productsGroup}; projectDirPath = ""; projectRoot = ""; targets = (${appTarget},${testTarget},${uiTarget}); packageReferences = (${packageRef});`);
const directory = path.join(root, 'platforms/ios/PixAuraAI.xcodeproj');
fs.mkdirSync(path.join(directory, 'xcshareddata/xcschemes'), { recursive: true });
fs.writeFileSync(path.join(directory, 'project.pbxproj'), `// !$*UTF8*$!\n{ archiveVersion = 1; classes = {}; objectVersion = 56; objects = {\n${objects.join('\n')}\n}; rootObject = ${project}; }\n`);
const reference = (target, name) => `<BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="${target}" BuildableName="${name}" BlueprintName="${name.replace(/\.(app|xctest)$/, '')}" ReferencedContainer="container:PixAuraAI.xcodeproj"/>`;
fs.writeFileSync(path.join(directory, 'xcshareddata/xcschemes/PixAuraAI.xcscheme'), `<?xml version="1.0" encoding="UTF-8"?>
<Scheme LastUpgradeVersion="2600" version="1.3">
<BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries>
<BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES">${reference(appTarget, 'PixAuraAI.app')}</BuildActionEntry>
</BuildActionEntries></BuildAction>
<TestAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" shouldUseLaunchSchemeArgsEnv="YES"><Testables>
<TestableReference skipped="NO">${reference(testTarget, 'PixAuraAITests.xctest')}</TestableReference>
<TestableReference skipped="NO">${reference(uiTarget, 'PixAuraAIUITests.xctest')}</TestableReference>
</Testables></TestAction>
<LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" debugServiceExtension="internal" allowLocationSimulation="YES"><BuildableProductRunnable runnableDebuggingMode="0">${reference(appTarget, 'PixAuraAI.app')}</BuildableProductRunnable></LaunchAction>
<ProfileAction buildConfiguration="Release" shouldUseLaunchSchemeArgsEnv="YES" savedToolIdentifier="" useCustomWorkingDirectory="NO" debugDocumentVersioning="YES"><BuildableProductRunnable runnableDebuggingMode="0">${reference(appTarget, 'PixAuraAI.app')}</BuildableProductRunnable></ProfileAction>
<AnalyzeAction buildConfiguration="Debug"/><ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/>
</Scheme>
`);
