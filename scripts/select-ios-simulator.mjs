import fs from 'node:fs';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

export function selectSimulator(list) {
  const runtimes = Object.entries(list.devices ?? {})
    .filter(([runtime]) => /^com\.apple\.CoreSimulator\.SimRuntime\.iOS-\d/.test(runtime))
    .sort(([a], [b]) => b.localeCompare(a, undefined, { numeric: true }));
  for (const [, devices] of runtimes) {
    const phone = devices.find(device => device.isAvailable === true &&
      device.name.startsWith('iPhone') && /^[0-9A-F-]{36}$/i.test(device.udid));
    if (phone) return phone.udid;
  }
  throw new Error('No available iPhone iOS simulator; install an iOS runtime on the macOS runner.');
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  if (process.argv.length !== 3) throw new Error('Usage: node scripts/select-ios-simulator.mjs <simctl-json>');
  console.log(selectSimulator(JSON.parse(fs.readFileSync(process.argv[2], 'utf8'))));
}
