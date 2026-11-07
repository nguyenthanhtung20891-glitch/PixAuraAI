import assert from 'node:assert/strict';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');

test('macOS package tests keep hidden externs through each target ld -r', () => {
  const script = read('scripts/check-apple.sh');
  assert.doesNotMatch(script, /swift test --package-path packages\/core/);
  const commands = script.match(/xcodebuild[^\n]*(?:\\\n[^\n]*)*(?:build|test)/g);
  const macos = commands.filter(command => command.includes("destination 'platform=macOS'"));
  assert.equal(macos.length, 1);
  assert.match(macos[0], /-scheme PixAuraCore /);
  assert.match(macos[0], /test$/);
  assert.ok(macos[0].includes("'KEEP_PRIVATE_EXTERNS=$(PIXAURA_PRIVATE_EXTERNS_$(TARGET_NAME))'"));
  for (const target of ['CPixAuraSQLite', 'CPixAuraCodecs', 'CPixAuraSodium', 'CPixAuraMetal']) {
    assert.match(macos[0], new RegExp(`PIXAURA_PRIVATE_EXTERNS_${target}=YES`));
  }
  assert.doesNotMatch(macos[0], /\bKEEP_PRIVATE_EXTERNS=YES/);
  assert.match(macos[0], /-warnings-as-errors/);
  assert.match(macos[0], /-Werror/);
  assert.doesNotMatch(script, /-Wno-shorten-64-to-32|-Wno-error|-keep_private_externs/);
});

test('hidden link inputs stay on their existing targets', () => {
  const manifest = read('packages/core/Package.swift');
  assert.doesNotMatch(manifest, /keep_private_externs|all_load/);
  const core = manifest.slice(manifest.indexOf('.target(name: "CPixAuraCore"'), manifest.indexOf('.target(name: "CPixAuraCertification"'));
  for (const target of ['CPixAuraSQLite', 'CPixAuraCodecs', 'CPixAuraSodium', 'CPixAuraMetal']) {
    assert.match(core, new RegExp(`"${target}"`));
  }
  const header = read('packages/core/src/metal_worker.hpp');
  assert.match(header, /__attribute__\(\(visibility\("hidden"\)\)\) bool metal_available\(\)noexcept;/);
  assert.match(header, /__attribute__\(\(visibility\("hidden"\)\)\) std::unique_ptr<MetalWorker> make_metal_worker\(bool reference\);/);
});
