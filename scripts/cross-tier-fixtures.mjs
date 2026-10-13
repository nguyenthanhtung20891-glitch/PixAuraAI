import fs from 'node:fs';
import crypto from 'node:crypto';
import zlib from 'node:zlib';
const directory = 'tests/fixtures/cross-tier';
const crc = bytes => {
  let result = 0xffffffff;
  for (const byte of bytes) { result ^= byte; for (let i = 0; i < 8; i++) result = (result >>> 1) ^ ((result & 1) ? 0xedb88320 : 0); }
  return (result ^ 0xffffffff) >>> 0;
};
function rows(width, height) {
  const bytes = Buffer.alloc((width * 4 + 1) * height);
  for (let y = 0; y < height; y++) for (let x = 0; x < width; x++) {
    const offset = y * (width * 4 + 1) + x * 4 + 1;
    const px = width >= 1024 ? Math.floor(x / 32) : x, py = width >= 1024 ? Math.floor(y / 32) : y;
    bytes.set([(px % 32) * 7, (py % 16) * 13, ((px + py) % 16) * 15, px % 4 === 0 ? 128 : 255], offset);
  }
  return bytes;
}
function png(width, height) {
  const chunk = (name, bytes) => {
    const type = Buffer.from(name), size = Buffer.alloc(4), checksum = Buffer.alloc(4);
    size.writeUInt32BE(bytes.length); checksum.writeUInt32BE(crc(Buffer.concat([type, bytes])));
    return Buffer.concat([size, type, bytes, checksum]);
  };
  const ihdr = Buffer.alloc(13); ihdr.writeUInt32BE(width); ihdr.writeUInt32BE(height, 4); ihdr[8] = 8; ihdr[9] = 6;
  return Buffer.concat([Buffer.from([137,80,78,71,13,10,26,10]), chunk('IHDR', ihdr), chunk('IDAT', zlib.deflateSync(rows(width, height), {level: 9})), chunk('IEND', Buffer.alloc(0))]);
}
if (process.argv.includes('--generate')) {
  fs.mkdirSync(directory, {recursive: true});
  const fixtures = [['small',48,32],['large',1024,512]].map(([name,width,height]) => ({name,width,height,bytes:png(width,height)}));
  fs.writeFileSync(`${directory}/fixtures.h`, '#ifndef PIXAURA_CROSS_TIER_FIXTURES_H\n#define PIXAURA_CROSS_TIER_FIXTURES_H\n#include <stdint.h>\n// SYNTHETIC test images only; deterministic pattern, no user photos.\n' + fixtures.map(f => `static const uint8_t cross_tier_${f.name}[] = {\n${Array.from(f.bytes).reduce((lines, byte, index) => { if (index % 24 === 0) lines.push([]); lines.at(-1).push(byte); return lines; }, []).map(line => '    ' + line.join(',') + ',').join('\n')}\n};\nstatic const char cross_tier_${f.name}_sha[] = "${crypto.createHash('sha256').update(f.bytes).digest('hex')}";\n`).join('') + '#endif\n');
  fs.writeFileSync(`${directory}/provenance.json`, JSON.stringify({kind:'SYNTHETIC POLICY EVIDENCE', generator:'scripts/cross-tier-fixtures.mjs', fixtures:fixtures.map(({name,width,height,bytes})=>({name,width,height,encodedBytes:bytes.length,sha256:crypto.createHash('sha256').update(bytes).digest('hex')}))},null,2)+'\n');
} else {
  const source = fs.readFileSync(`${directory}/fixtures.h`, 'utf8');
  const fixtures = JSON.parse(fs.readFileSync(`${directory}/provenance.json`, 'utf8')).fixtures;
  for (const fixture of fixtures) {
    const match = source.match(new RegExp(`cross_tier_${fixture.name}\\[\\] = \\{([\\s\\S]*?)\\};`));
    const bytes = Buffer.from(match[1].match(/\d+/g).map(Number));
    if (bytes.length > 32768 || bytes.length !== fixture.encodedBytes || crypto.createHash('sha256').update(bytes).digest('hex') !== fixture.sha256) throw Error('fixture identity/bound');
    let at = 8; const idat = [];
    while (at < bytes.length) {
      const count = bytes.readUInt32BE(at), name = bytes.toString('ascii',at+4,at+8), data=bytes.subarray(at+8,at+8+count);
      if (crc(bytes.subarray(at+4,at+8+count)) !== bytes.readUInt32BE(at+8+count)) throw Error('PNG CRC');
      if (name === 'IDAT') idat.push(data); at += count + 12;
    }
    if (!zlib.inflateSync(Buffer.concat(idat)).equals(rows(fixture.width,fixture.height))) throw Error('independent fixture pixels');
  }
  console.log('bounded synthetic cross-tier PNG identity/CRC/pixels PASS');
}
