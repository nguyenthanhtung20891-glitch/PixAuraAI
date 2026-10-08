import fs from 'node:fs';
import { pathToFileURL } from 'node:url';

export function validateEvidence(json, run, sha, testExit) {
  if (!/^[0-9a-f]{32}$/.test(run) || !/^[0-9a-f]{40}$/.test(sha)) throw Error('Invalid invocation identity');
  if (!json || Buffer.byteLength(json, 'utf8') > 16384) throw Error('Missing or oversized evidence');
  const e = JSON.parse(json.replace(/^\uFEFF/, ''));
  if (e.validation_run !== run) throw Error('Stale or mismatched validation_run');
  if (e.source_sha !== sha) throw Error('Mismatched source SHA');
  if (testExit !== 0 || e.status !== 'PASS' || e.hardware !== true ||
      e.controlled_hardware_gate !== true || e.pipeline !== true || e.dispatch !== true ||
      e.parity_passed !== 14 || e.test_count !== 14 || e.expected_test_count !== 14) {
    throw Error('Hardware certification did not PASS all 14 actual GPU cases');
  }
  return e;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const [file, run, sha, exit] = process.argv.slice(2);
    if (fs.statSync(file).size > 16384) throw Error('Oversized evidence');
    const e = validateEvidence(fs.readFileSync(file, 'utf8'), run, sha, Number(exit));
    console.log(`Vulkan hardware validation: ${e.status} ${e.parity_passed}/${e.test_count} parity cases; GPU=${e.gpu}`);
  } catch (error) {
    console.error(`FAIL: ${error.message}`);
    process.exitCode = 1;
  }
}
