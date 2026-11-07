import assert from 'node:assert/strict';
import crypto from 'node:crypto';
import fs from 'node:fs';
import test from 'node:test';

const read = (name) => fs.readFileSync(name, 'utf8');
const between = (text, start, end) => text.slice(text.indexOf(start), end ? text.indexOf(end) : undefined);

test('SQLite width diagnostics stay inside the vendored amalgamation target', () => {
  const manifest = read('packages/core/Package.swift');
  const sodium = between(manifest, '.target(name: "CPixAuraSodium"', '.target(name: "CPixAuraSQLite"');
  const sqlite = between(manifest, '.target(name: "CPixAuraSQLite"', '.target(name: "CPixAuraCodecs"');
  const codecs = between(manifest, '.target(name: "CPixAuraCodecs"', '.target(name: "CPixAuraMetal"');
  const owned = manifest.slice(manifest.indexOf('.target(name: "CPixAuraMetal"'));
  const exemption = '.unsafeFlags(["-Wno-shorten-64-to-32"], .when(platforms: [.macOS, .iOS]))';
  assert.match(sqlite, /sources: \["sqlite3\.c"\]/);
  assert.match(sqlite, /\.unsafeFlags\(\["-Wno-ambiguous-macro"\], \.when\(platforms: \[\.macOS, \.iOS\]\)\)/);
  assert.ok(sqlite.includes(exemption));
  assert.match(sqlite, /SQLITE_THREADSAFE", to: "1"/);
  assert.match(sqlite, /SQLITE_DQS", to: "0"/);
  assert.match(sqlite, /SQLITE_OMIT_LOAD_EXTENSION/);
  assert.match(sqlite, /SQLITE_OMIT_SHARED_CACHE/);
  assert.match(sqlite, /SQLITE_MAX_LENGTH", to: "8388608"/);
  assert.match(sqlite, /SQLITE_MAX_SQL_LENGTH", to: "65536"/);
  assert.match(sqlite, /SQLITE_MAX_ATTACHED", to: "0"/);
  assert.doesNotMatch(sodium, /-Wno-/);
  assert.ok(codecs.includes(exemption));
  assert.doesNotMatch(owned, /-Wno-|-Wno-error|\b-w\b/);
  assert.match(read('scripts/check-apple.sh'), /-warnings-as-errors/);
  assert.match(read('scripts/check-apple.sh'), /-Werror/);
  assert.doesNotMatch(read('scripts/check-apple.sh'), /-Wno-shorten-64-to-32|-Wno-error/);
  const cmake = read('CMakeLists.txt');
  const vendor = between(cmake, 'add_library(pixaura_sqlite', 'add_library(pixaura_storage');
  assert.match(vendor, /pixaura_strict\(pixaura_sqlite\)/);
  assert.match(vendor, /-Wno-unused-parameter -Wno-unused-variable/);
  assert.match(vendor, /SQLITE_MAX_LENGTH=8388608 SQLITE_MAX_SQL_LENGTH=65536 SQLITE_MAX_ATTACHED=0/);
  assert.doesNotMatch(vendor, /-Wno-shorten-64-to-32|-Wno-error/);
  const pin = JSON.parse(read('packages/core/vendor/sqlite/provenance.json'));
  assert.equal(pin.version, '3.53.4');
  for (const [file, digest] of Object.entries(pin.files)) {
    const actual = crypto.createHash('sha256').update(fs.readFileSync(`packages/core/vendor/sqlite/${file}`)).digest('hex');
    assert.equal(actual, digest, file);
  }
  const storage = read('packages/core/src/storage.cpp');
  assert.match(storage, /SQLITE_LIMIT_LENGTH,static_cast<int>\(document::manifest_limit\)/);
  assert.match(storage, /SQLITE_LIMIT_SQL_LENGTH,65536/);
  assert.match(storage, /SQLITE_LIMIT_ATTACHED,0/);
  assert.match(storage, /PRAGMA max_page_count=65536/);
  assert.match(storage, /PRAGMA journal_size_limit=16777216/);
  assert.match(storage, /PRAGMA page_size"\)==4096/);
  assert.match(storage, /SQLITE_OPEN_NOFOLLOW/);
  assert.match(storage, /SQLITE_DBCONFIG_DEFENSIVE/);
  assert.match(storage, /SQLITE_CORRUPT\|\|c==SQLITE_NOTADB/);
  assert.match(read('packages/core/src/document.hpp'), /manifest_limit = 8u \* 1024u \* 1024u/);
  assert.match(read('packages/core/vendor/sqlite/sqlite3.h'), /SQLITE_API void \*sqlite3_malloc\(int\)/);
  assert.match(read('packages/core/vendor/sqlite/sqlite3.c'), /#\s*define SQLITE_MAX_MMAP_SIZE 0x7fff0000/);
  for (const workflow of ['.github/workflows/foundation.yml', '.github/workflows/native-shells.yml']) {
    assert.match(read(workflow), /tests\/sqlite-warning-policy\.test\.mjs/);
  }
});
