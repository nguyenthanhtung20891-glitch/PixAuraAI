import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');
const hash = (name) => crypto.createHash('sha256').update(fs.readFileSync(name)).digest('hex');

test('12-bit JPEG histogram indexes fit in int without changing vendor bytes', () => {
  const manifest = read('packages/core/Package.swift');
  const codecs = manifest.slice(manifest.indexOf('.target(name: "CPixAuraCodecs"'), manifest.indexOf('.target(name: "CPixAuraMetal"'));
  const owned = manifest.slice(manifest.indexOf('.target(name: "CPixAuraCore"'));
  assert.match(codecs, /\.unsafeFlags\(\["-Wno-shorten-64-to-32"\], \.when\(platforms: \[\.macOS, \.iOS\]\)\)/);
  assert.match(codecs, /src\/decode_codec\.c/);
  assert.match(codecs, /jquant2-12\.c/);
  assert.doesNotMatch(owned, /-Wno-shorten-64-to-32/);
  assert.match(read('scripts/check-apple.sh'), /-Xcc -Werror/);
  assert.doesNotMatch(read('scripts/check-apple.sh'), /-Wno-shorten-64-to-32|-Wno-error/);
  const cmake = read('CMakeLists.txt');
  const adapter = cmake.slice(cmake.indexOf('packages/core/src/decode_codec.c PROPERTIES'));
  assert.match(adapter, /-Wall;-Wextra;-Wpedantic;-Werror/);
  assert.doesNotMatch(adapter.slice(0, adapter.indexOf(')')), /-Wno-shorten-64-to-32/);
  const quant = read('packages/core/vendor/codecs/jpeg/src/jquant2.c');
  const wrapper = read('packages/core/vendor/codecs/jpeg/src/wrapper/jquant2-12.c');
  assert.match(quant, /typedef JLONG LOCFSERROR/);
  assert.match(quant, /fill_inverse_cmap\(j_decompress_ptr cinfo, int c0, int c1, int c2\)/);
  assert.match(quant, /fill_inverse_cmap\(cinfo, cur0 >> C0_SHIFT, cur1 >> C1_SHIFT,\s*cur2 >> C2_SHIFT\)/);
  assert.match(wrapper, /#define BITS_IN_JSAMPLE\s+12/);
  assert.match(wrapper, /#include "\.\.\/jquant2\.c"/);
  const bits = 12;
  const maxSample = 4095;
  assert.equal(maxSample >> (bits - 5), 31);
  assert.equal(maxSample >> (bits - 6), 63);
  assert.equal(maxSample >> (bits - 5), 31);
  const pins = JSON.parse(read('packages/core/vendor/codecs/provenance.json'));
  assert.equal(hash('packages/core/vendor/codecs/jpeg/src/jquant2.c'), pins.jpeg.files['src/jquant2.c']);
  assert.equal(hash('packages/core/vendor/codecs/jpeg/src/wrapper/jquant2-12.c'), pins.jpeg.files['src/wrapper/jquant2-12.c']);
  const decoder = read('packages/core/src/decode_codec.c');
  assert.match(decoder, /data_precision!=8/);
  assert.match(decoder, /jpeg_start_compress/);
  assert.match(decoder, /png_create_read_struct/);
  assert.match(decoder, /status=MALFORMED/);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/jpeg-quant-compat\.test\.mjs/);
  }
});
