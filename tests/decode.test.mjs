import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const root = new URL('../', import.meta.url);
const read = name => fs.readFileSync(new URL(name, root), 'utf8');
test('codec source pins retain exact official distribution bytes', () => {
  const pins = JSON.parse(read('packages/core/vendor/codecs/provenance.json'));
  for (const [codec, version] of Object.entries({ jpeg: '3.2.0', png: '1.6.59', zlib: '1.3.2' })) {
    const pin = pins[codec];
    assert.equal(pin.version, version);
    assert.match(pin.url, /^https:/);
    assert.ok(pin.license);
    for (const [file, hash] of Object.entries(pin.files)) {
      const bytes = fs.readFileSync(new URL(`packages/core/vendor/codecs/${codec}/${file}`, root));
      assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'), hash, file);
    }
  }
});
test('synthetic fixture identities match independent C and mobile consumers', () => {
  const fixtures = JSON.parse(read('tests/fixtures/decode/fixtures.json'));
  for (const file of ['platforms/android/app/src/androidTest/assets/decode.json', 'packages/core/swift/Tests/Fixtures/decode.json']) {
    assert.equal(read(file), read('tests/fixtures/decode/fixtures.json'));
  }
  for (const [name, fixture] of Object.entries(fixtures)) {
    assert.equal(crypto.createHash('sha256').update(Buffer.from(fixture.bytes)).digest('hex'), fixture.sha256);
    assert.ok(read('tests/fixtures/decode/fixtures.h').includes(fixture.bytes.join(',')), name);
  }
});
test('decode execution is included in portable and Apple builds', () => {
  for (const file of ['CMakeLists.txt', 'packages/core/Package.swift']) {
    for (const source of ['decode.cpp', 'decode_api.cpp', 'decode_codec.c']) assert.ok(read(file).includes(source));
  }
  assert.match(read('CMakeLists.txt'), /decode_test PROPERTIES TIMEOUT 90/);
  assert.match(read('CMakeLists.txt'), /decode_c_consumer PROPERTIES LINKER_LANGUAGE CXX/);
});
