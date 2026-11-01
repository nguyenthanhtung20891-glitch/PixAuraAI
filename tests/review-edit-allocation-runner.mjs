// Engineering-only fault-injection orchestration. No production dependency.
import {execFile} from 'node:child_process';
import assert from 'node:assert/strict';
const [executable, root] = process.argv.slice(2);
assert.ok(executable && root);
// Separate processes preserve all MSVC debug checks without a shared CRT heap
// lock. Each worker/editor has its own allocator controls and temporary root.
const runs = await Promise.allSettled(Array.from({length:4}, (_,worker) => new Promise((resolve,reject) => {
 execFile(executable,[root,String(worker)],{encoding:'utf8',maxBuffer:1024*1024},(error,stdout,stderr)=>{
  if(error){process.stderr.write(stdout+stderr);reject(error);return;}
  try {
  const rows=[...stdout.matchAll(/Edit allocation worker=(\d+) operation=(\d+) failures=(\d+) first_success=(\d+) cumulative_requested_max=(\d+) result_bytes=(\d+)/g)].map(m=>m.slice(1).map(Number));
  assert.equal(rows.length,5);for(let kind=0;kind<5;kind++){
   const [w,k,failures,first,,bytes]=rows[kind];assert.equal(w,worker);assert.equal(k,kind);assert.equal(bytes,480);
   assert.equal(first%4,worker);assert.ok(failures>0);assert.equal(failures,(first-worker)/4);
  }
  resolve(rows);
  } catch(failure) { reject(failure); }
 });
})));
// Join ALL children before reporting an error or aggregating their evidence.
for(const r of runs)if(r.status==='rejected')throw r.reason;
for(let kind=0;kind<5;kind++){
 const rows=runs.map(r=>r.value[kind]),first=Math.min(...rows.map(r=>r[3])),last=Math.max(...rows.map(r=>r[3]));
 const failures=rows.reduce((sum,r)=>sum+r[2],0),maximum=Math.max(...rows.map(r=>r[4]));
 // All indices [0,first_success) failed, once each: no gaps or sampling.
 assert.ok(last-first<4);assert.equal(failures,first);
 console.log(`Edit allocation operation=${kind} failures=${failures} cumulative_requested_max=${maximum} result_bytes=480 workers=4 contiguous_prefix=${first}`);
}
console.log('Edit allocation: outputs atomic; ordinary document/history unchanged; no image resources; every failure index retained; all isolated children joined');
