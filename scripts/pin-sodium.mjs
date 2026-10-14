// Engineering-only: derive conservative portable build inputs from approved release.
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
const base = 'packages/core/vendor/libsodium';
const make = fs.readFileSync(`${base}/src/libsodium/Makefile.am`, 'utf8');
const main = make.split('if HAVE_TI_MODE')[0];
const extra = make.split('if !MINIMAL')[1].split('endif')[0];
const paths = [...new Set((main + extra).match(/\b[\w/.-]+\.c\b/g))];
paths.push('crypto_stream/salsa20/ref/salsa20_ref.c', 'randombytes/sysrandom/randombytes_sysrandom.c', 'crypto_pwhash/argon2/argon2-fill-block-neon.c');
// Upstream common.h enables these dispatch implementations under MSVC itself.
// Preserve their guarded upstream sources; no owned/vendor macro suppression.
paths.push('crypto_pwhash/argon2/argon2-fill-block-avx2.c',
    'crypto_pwhash/argon2/argon2-fill-block-avx512f.c',
    'crypto_pwhash/argon2/argon2-fill-block-ssse3.c',
    'crypto_generichash/blake2b/ref/blake2b-compress-avx2.c',
    'crypto_generichash/blake2b/ref/blake2b-compress-sse41.c',
    'crypto_generichash/blake2b/ref/blake2b-compress-ssse3.c',
    'crypto_stream/chacha20/dolbeau/chacha20_dolbeau-avx2.c',
    'crypto_stream/chacha20/dolbeau/chacha20_dolbeau-ssse3.c',
    'crypto_stream/salsa20/xmm6int/salsa20_xmm6int-avx2.c',
    'crypto_stream/salsa20/xmm6int/salsa20_xmm6int-sse2.c',
    'crypto_aead/aegis128l/aegis128l_aesni.c',
    'crypto_aead/aegis256/aegis256_aesni.c');
paths.sort();
fs.writeFileSync('packages/core/sodium-sources.cmake', 'set(PIXAURA_SODIUM_SOURCES\n' + paths.map(p => `    packages/core/vendor/libsodium/src/libsodium/${p}`).join('\n') + '\n)\n');
const walk = dir => fs.readdirSync(dir, {withFileTypes:true}).flatMap(e => e.isDirectory() ? walk(path.join(dir,e.name)) : [path.join(dir,e.name)]);
const files = Object.fromEntries(walk(base).filter(p=>!p.endsWith('provenance.json')).sort().map(p=>[path.relative(base,p).replaceAll('\\','/'),crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex')]));
fs.writeFileSync(`${base}/provenance.json`, JSON.stringify({version:'1.0.22',license:'ISC; upstream embedded notices retained',source:'https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz',archive_sha256:'adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349',configuration:'portable full generic C; fe25.5; SODIUM_STATIC; CONFIGURED=1; compiler-mandated AArch64 NEON and MSVC guarded dispatch sources retained for initialization closure; no optional ISA flags/assembly, COMPAT, LTO or native tuning; point-validation API retained',files},null,2)+'\n');
