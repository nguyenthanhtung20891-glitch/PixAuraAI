import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import { execFileSync } from 'node:child_process';
const read=p=>fs.readFileSync(p,'utf8');
test('approved libsodium bytes and strict portable configuration remain pinned',()=>{
    const base='packages/core/vendor/libsodium/';
    const pin=JSON.parse(read(base+'provenance.json'));
    assert.equal(pin.version,'1.0.22');assert.ok(pin.license.startsWith('ISC'));
    assert.equal(pin.archive_sha256,'adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349');
    for(const [file,hash] of Object.entries(pin.files))assert.equal(crypto.createHash('sha256').update(fs.readFileSync(base+file)).digest('hex'),hash,file);
    const config=read('packages/core/sodium.cmake')+read('packages/core/Package.swift');
    const cmakeSources=read('packages/core/sodium-sources.cmake').match(/packages\/core\/vendor\/libsodium\/[^\s)]+/g).map(s=>s.replace('packages/core/',''));
    const swiftSources=JSON.parse(read('packages/core/Package.swift').match(/let sodiumSources = (\[[^\n]+\])/)[1]);
    assert.deepEqual(swiftSources,cmakeSources,'CMake/Swift dependency source parity');
    assert.ok(!/define\([^\n]*ED25519_COMPAT|march=native|INTERPROCEDURAL_OPTIMIZATION ON/.test(config));
    assert.match(read('packages/core/src/model_package.cpp'),/crypto_core_ed25519_is_valid_point/);
    assert.match(read('packages/core/src/model_package.cpp'),/crypto_sign_ed25519_verify_detached/);
});
test('independent package fixtures are deterministic and test trust is excluded from release source',()=>{
    execFileSync(process.execPath,['scripts/model-package-fixtures.mjs','--check']);
    const source=read('packages/core/src/model_package.cpp');
    assert.match(source,/#ifdef PIXAURA_CERTIFICATION_TESTING\s+#include "\.\.\/tests\/model_package_boundary.hpp"/);
    assert.match(source,/\{1,1,1,nullptr,0\}/);
    const header=read('packages/core/include/pixaura/model_package.h');
    assert.ok(!/sign\(|key_pair|private_key|verify_test|set_trust/.test(header));
    assert.ok(!read('packages/core/sodium-config/include/sodium_marker.h').includes('#include <sodium.h>'));
});
