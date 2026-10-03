// Verify the actual disk and emulator-generated hardware, never just config.ini.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

export function readDiskSize(value) {
  const match = /^(\d+)([kmgt]?)$/i.exec(value.trim());
  if (!match) throw new Error(`Invalid resolved disk size: ${value}`);
  const exponent = match[2] ? 'kmgt'.indexOf(match[2].toLowerCase()) + 1 : 0;
  const size = Number(match[1]) * 1024 ** exponent;
  if (!Number.isSafeInteger(size) || size <= 0) throw new Error('Invalid resolved disk size');
  return size;
}

export function assertUserdata(image, boundMiB, hardware, qemuInfo) {
  const bound = boundMiB * 1024 * 1024;
  if (!Number.isSafeInteger(bound) || bound <= 0) throw new Error('Invalid userdata bound');
  const stat = fs.statSync(image);
  if (!stat.isFile() || stat.size > bound || stat.size < 2048) {
    throw new Error(`Userdata file size ${stat.size} exceeds bound ${bound} or is invalid`);
  }
  const fd = fs.openSync(image, 'r');
  const block = Buffer.alloc(1024);
  try {
    if (fs.readSync(fd, block, 0, block.length, 1024) !== block.length || block.readUInt16LE(56) !== 0xef53) {
      throw new Error('Userdata is not a raw ext4 filesystem');
    }
  } finally { fs.closeSync(fd); }
  const shift = block.readUInt32LE(24);
  if (shift > 6) throw new Error('Invalid ext4 block size');
  const high = (block.readUInt32LE(96) & 0x80) ? block.readUInt32LE(336) : 0;
  const blocks = block.readUInt32LE(4) + high * 2 ** 32;
  const virtualBytes = blocks * 1024 * 2 ** shift;
  if (!Number.isSafeInteger(virtualBytes) || virtualBytes !== stat.size || virtualBytes > bound) {
    throw new Error(`Userdata ext4 virtual size ${virtualBytes} differs from file size or exceeds bound ${bound}`);
  }
  let resolvedBytes;
  if (hardware) {
    const keys = new Map();
    for (const line of fs.readFileSync(hardware, 'utf8').split(/\r?\n/)) {
      const match = /^\s*(disk\.dataPartition\.(?:size|path|initPath))\s*=\s*(.*?)\s*$/.exec(line);
      if (!match) continue;
      if (keys.has(match[1])) throw new Error(`Duplicate resolved key: ${match[1]}`);
      keys.set(match[1], match[2]);
    }
    resolvedBytes = readDiskSize(keys.get('disk.dataPartition.size') ?? '');
    if (resolvedBytes > bound || resolvedBytes !== virtualBytes) {
      throw new Error(`Effective resolved userdata ${resolvedBytes} exceeds bound ${bound} or differs from ext4 size`);
    }
    const resolvedPath = keys.get('disk.dataPartition.path');
    if (!resolvedPath || fs.realpathSync(resolvedPath) !== fs.realpathSync(image)) {
      throw new Error('Effective userdata path differs from the verified CI image');
    }
    if (keys.get('disk.dataPartition.initPath')) throw new Error('Resolved userdata would be recreated from initPath');
  }
  let qemuVirtualBytes;
  if (qemuInfo) {
    const info = JSON.parse(fs.readFileSync(qemuInfo, 'utf8'));
    qemuVirtualBytes = info['virtual-size'];
    if (!Number.isSafeInteger(qemuVirtualBytes) || qemuVirtualBytes !== virtualBytes || qemuVirtualBytes > bound) {
      throw new Error(`QEMU virtual userdata size ${qemuVirtualBytes} differs from ext4 size or exceeds bound ${bound}`);
    }
    const expectedPath = info.format === 'raw' ? image : `${image}.qcow2`;
    if (!['raw', 'qcow2'].includes(info.format) || path.resolve(info.filename) !== path.resolve(expectedPath)) {
      throw new Error('QEMU userdata image format/path differs from the verified CI image');
    }
    if (info.format === 'qcow2' && fs.realpathSync(info['full-backing-filename']) !== fs.realpathSync(image)) {
      throw new Error('QEMU userdata overlay backing path differs from the verified CI image');
    }
  }
  return { image: fs.realpathSync(image), fileBytes: stat.size, ext4VirtualBytes: virtualBytes,
    resolvedBytes, qemuVirtualBytes, boundBytes: bound };
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  try {
    const [, , image, boundMiB, hardware, qemuInfo] = process.argv;
    console.log(JSON.stringify(assertUserdata(image, Number(boundMiB), hardware, qemuInfo), null, 2));
  } catch (error) {
    console.error(`Userdata pre-launch assertion failed: ${error.message}`);
    process.exitCode = 1;
  }
}
