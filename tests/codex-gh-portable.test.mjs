import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';

// Fake subprocess boundary only, never a GitHub authentication or CI receipt.
function invoke(args, overrides = {}) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'pixaura-gh-inspection-'));
  const trace = path.join(root, 'trace.jsonl');
  fs.writeFileSync(path.join(root, 'gh'), `#!/usr/bin/env node
const fs = require('node:fs');
const args = process.argv.slice(2);
fs.appendFileSync(process.env.INSPECTION_TRACE, JSON.stringify({args, debug: process.env.GH_DEBUG, config: process.env.GH_CONFIG_DIR})+'\\n');
if (args[0] === 'auth') process.stdout.write('  - Token: synthetic-not-a-credential\\n');
else if (args.includes('.run_id')) process.stdout.write(process.env.INSPECTION_JOB_RUN+'\\n');
else process.stdout.write('{}\\n');
process.exit(Number(process.env.INSPECTION_EXIT || '0'));
`, { mode: 0o700 });
  try {
    const result = spawnSync('python3', ['scripts/codex-gh.py', ...args], {
      encoding: 'utf8', env: { ...process.env, PATH: root+path.delimiter+process.env.PATH,
        INSPECTION_TRACE: trace, INSPECTION_JOB_RUN: '123', GH_DEBUG: 'api',
        GH_CONFIG_DIR: 'synthetic-preserved-configuration', ...overrides }
    });
    const calls = fs.existsSync(trace) ? fs.readFileSync(trace, 'utf8').trim().split('\n').filter(Boolean).map(JSON.parse) : [];
    return { ...result, calls };
  } finally { fs.rmSync(root, { recursive: true, force: true }); }
}

test('portable CI inspection rejects settings, foreign repositories and malformed input before invoking gh', () => {
  for (const args of [[], ['api', 'foreign'], ['run-view', '123', '--repo', 'foreign/repo'],
    ['auth-status', 'extra'], ['run-list', '--repo'], ['run-list', 'abc1234;echo injected'],
    ['run-view', '0'], ['run-view', '-1'], ['run-view', '1'.repeat(21)],
    ['run-failed', '123', '--job'], ['run-watch', '../123'], ['run-rerun', '123']]) {
    const result = invoke(args);
    assert.equal(result.status, 2, JSON.stringify(args));
    assert.deepEqual(result.calls, []);
  }
});
test('portable CI inspection binds SHA/repository and emits attempt and job provenance', () => {
  const sha = 'a'.repeat(40);
  const listing = invoke(['run-list', sha]);
  assert.equal(listing.status, 0);
  assert.deepEqual(listing.calls[0].args.slice(0, 6), ['run','list','--repo','nguyenthanhtung20891-glitch/PixAuraAI','--commit',sha]);
  const result = invoke(['run-view', '123']);
  assert.equal(result.status, 0); assert.equal(result.calls.length, 2);
  assert.equal(result.calls[0].args[1], 'repos/nguyenthanhtung20891-glitch/PixAuraAI/actions/runs/123');
  assert.match(result.calls[0].args[3], /run_attempt/);
  assert.ok(result.calls[1].args.includes('headSha,status,conclusion,jobs,url'));
  for (const call of result.calls) {
    assert.equal(call.debug, undefined); assert.equal(call.config, 'synthetic-preserved-configuration');
  }
});
test('portable CI inspection refuses foreign-run job logs and preserves command failures', () => {
  const foreign = invoke(['run-failed', '123', '456'], { INSPECTION_JOB_RUN: '999' });
  assert.equal(foreign.status, 2); assert.equal(foreign.calls.length, 1);
  const own = invoke(['run-failed', '123', '456']);
  assert.equal(own.status, 0); assert.equal(own.calls.length, 2);
  assert.equal(own.calls[1].args[1], 'repos/nguyenthanhtung20891-glitch/PixAuraAI/actions/jobs/456/logs');
  const failed = invoke(['run-view', '123'], { INSPECTION_EXIT: '7' });
  assert.equal(failed.status, 7); assert.equal(failed.calls.length, 1);
});
test('portable authentication status redacts credential descriptions', () => {
  const result = invoke(['auth-status']); assert.equal(result.status, 0);
  assert.doesNotMatch(result.stdout, /synthetic-not-a-credential/);
  assert.match(result.stdout, /Token: \[redacted\]/);
});
test('portable inspection cache refuses a symlink escape without writing the target', () => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'pixaura-gh-cache-'));
  const owner = path.join(root, 'owner'); const outside = path.join(root, 'outside');
  fs.mkdirSync(owner); fs.mkdirSync(outside); fs.symlinkSync(outside, path.join(owner, 'build'));
  try {
    const result = spawnSync('python3', ['-c', `import importlib.util,pathlib,sys
spec=importlib.util.spec_from_file_location('inspection','scripts/codex-gh.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
try: m.cache_directory(pathlib.Path(sys.argv[1]))
except ValueError: raise SystemExit(2)
`, owner], { encoding: 'utf8', env: { ...process.env, PYTHONDONTWRITEBYTECODE: '1' } });
    assert.equal(result.status, 2); assert.deepEqual(fs.readdirSync(outside), []);
  } finally { fs.rmSync(root, { recursive: true, force: true }); }
});
