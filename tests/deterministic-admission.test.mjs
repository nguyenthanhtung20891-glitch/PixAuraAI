import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
import {execFileSync} from 'node:child_process';
const read=p=>JSON.parse(fs.readFileSync(p,'utf8'));
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const canonical=v=>JSON.stringify(v,(_,x)=>x&&typeof x==='object'&&!Array.isArray(x)?Object.fromEntries(Object.entries(x).sort(([a],[b])=>a<b?-1:a>b?1:0)):x)+'\n';
test('production deterministic admission binds exact existing code and all eleven goal goldens',()=>{
 const m=read('docs/admission/deterministic-mvp-provider-v1.json');
 assert.equal(m.provider,'reference.assistance.v1');assert.equal(m.planner,'reference.deterministic.v1');
 assert.deepEqual(m.model_artifacts,[]);assert.deepEqual(m.production_dependencies_added,[]);
 for(const [path,digest] of Object.entries(m.source_identity.sha256))assert.equal(hash(fs.readFileSync(path,'utf8').replaceAll('\r\n','\n')),digest,path);
 const c=read('tests/fixtures/reference-planner-v1.json');assert.equal(c.mappings.length,11);
 assert.equal(c.cases.length,39);assert.equal(c.negative.length,42);
 const streams=[];let fnv=14695981039346656037n;
 for(const f of c.cases)for(const kind of [1,2,3]){
  const payload=JSON.parse(kind===3?f.expected_plan:f.input);
  const bytes=canonical({binding:payload.binding,envelope_version:1,kind,payload,proposal_id:'proposal.1',provider_id:m.provider,provider_version:1,registry_version:1});
  streams.push(bytes);for(const b of Buffer.from(bytes)){fnv^=BigInt(b);fnv=BigInt.asUintN(64,fnv*1099511628211n);}
 }
 assert.equal(streams.length,m.envelopes);assert.equal(hash(streams.join('')),m.golden_envelope_stream_sha256);
 assert.equal(fnv.toString(16).padStart(16,'0'),m.golden_fnv64);
 assert.ok(fs.readFileSync('packages/core/tests/assistance_test.cpp','utf8').includes('0x'+m.golden_fnv64));
 assert.ok(fs.readFileSync('packages/core/tests/assistance_test.cpp','utf8').includes('test_owner_bytes='));
 assert.ok(fs.readFileSync('packages/core/tests/document_allocation_test.cpp','utf8').includes('production_owner_bytes='));
 const defs=fs.readFileSync('CMakeLists.txt','utf8').match(/target_compile_definitions\(document_allocation_test PRIVATE ([^)]+)\)/)?.[1];
 assert.ok(defs&&!defs.includes('PIXAURA_ASSISTANCE_TESTING'),'shipping layout measurement must exclude checkpoint fields');
});
test('admission preserves separate provider/planner identity and model-free scope',()=>{
 const c=fs.readFileSync('docs/contracts/deterministic-mvp-provider-admission-v1.md','utf8');
 for(const term of ['reference.assistance.v1','reference.deterministic.v1','HUMAN','NONE REQUIRED','NONE ADMITTED','stop -> join','2048','8192','12288','NO MODEL WORKLOAD'])assert.ok(c.includes(term),term);
 const provider=fs.readFileSync('packages/core/src/assistance.hpp','utf8');
 for(const term of ['std::try_to_lock','std::mutex active_, fence_','std::numeric_limits<uint64_t>::max()'])assert.ok(provider.includes(term),term);
 for(const term of ['std::thread','std::async','filesystem','executeAgentPlan','approval_issuer','candidate_handle'])assert.ok(!provider.includes(term),term);
});
test('admission certification retains platform isolation and does not open Step 8',()=>{
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md']){
  const s=fs.readFileSync(p,'utf8').split('<!-- phase7-step7-round5 -->')[1]?.split('<!-- phase7-step7-round5-history -->')[0];assert.ok(s,p);
  for(const t of ['Step 8 NOT STARTED','Phase 8 NOT STARTED','NONE REQUIRED / NONE ADMITTED','reference.assistance.v1','Vulkan/Metal UNKNOWN'])assert.ok(s.includes(t),p+t);
 }
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(fs.readFileSync(p,'utf8').includes('tests/deterministic-admission.test.mjs'));
 assert.ok(fs.readFileSync('scripts/check-ios-shell.sh','utf8').includes('scripts/check-apple-release.py'));
});

test('Phase 7 closure preserves the complete frozen runtime and contract inventory',()=>{
 const m=read('docs/reports/phase-7-closure-v1.json');
 assert.equal(m.runtime_authority,false);assert.equal(m.baseline,'54dcc9292f43ab49335a99df3a4145d0279c7980');
 const paths=execFileSync('git',['ls-files'],{encoding:'utf8'}).trim().split('\n').filter(p=>m.roots.some(r=>p.startsWith(r))||m.exact.includes(p)||(/^(platforms\/android\/|platforms\/ios\/)/.test(p)&&/\.(gradle\.kts|toml|lockfile|pbxproj)$/.test(p))).sort();
 // Historical Phase 7 remains immutable. Phase 8 Step 2 authorizes only the
 // individually bound preview changes/additions; the new preview test fixes the
 // exact allowlist and checks both sides, not a blanket runtime exemption.
 const preview=read('docs/reports/phase-8-step-2-v1.json');
 const inspection=read('docs/reports/phase-8-step-3-v1.json');
 const added=new Set(['docs/contracts/full-review-experience-v1.md',...Object.keys(preview.additions),...Object.keys(inspection.additions)]);
 assert.deepEqual(paths.filter(p=>!added.has(p)),Object.keys(m.sha256).sort(),'no unreviewed runtime/contract addition or removal');
 for(const [p,digest] of Object.entries(m.sha256)){
  const b=fs.readFileSync(p),s=b.toString('utf8');
  const change=preview.changes[p];if(change)assert.equal(change.before_sha256_lf,digest,p);
  const before=change?.after_sha256_lf??digest;const next=inspection.changes[p];if(next)assert.equal(next.before_sha256_lf,before,p);
  assert.equal(hash(Buffer.from(s,'utf8').equals(b)?Buffer.from(s.replaceAll('\r\n','\n')):b),next?.after_sha256_lf??before,p);
 }
 for(const c of read('docs/reports/phase-6-closure-v1.json').contracts)assert.equal(hash(fs.readFileSync(c.path,'utf8').replaceAll('\r\n','\n')),c.sha256_lf,c.path);
 assert.equal(m.original_contract_freezes.length,8);
 for(const c of m.original_contract_freezes){assert.match(c.certified_origin,/^[a-f0-9]{40}$/);assert.equal(m.sha256[c.path],c.sha256_lf,c.path);}
 assert.equal(m.phase8,'NOT_STARTED');
});

test('Phase 7 integrated history requires exact closure SHAs and every required job',()=>{
 const h=read('docs/reports/evidence/phase-7-certified-steps.json');
 assert.equal(h.runtime_authority,false);assert.equal(h.steps.length,7);
 for(const [i,s] of h.steps.entries()){
  assert.equal(s.step,i+1);assert.match(s.closure_sha,/^[a-f0-9]{40}$/);assert.equal(s.runs.length,2);
  for(const [j,r] of s.runs.entries()){
   assert.equal(r.head_sha,s.closure_sha);assert.equal(r.attempt,1);
   assert.equal(r.status,'completed');assert.equal(r.conclusion,'success');
   assert.equal(r.name,j===0?'Foundation boundaries':'Native application shells');
   assert.equal(r.jobs.length,j===0?7:3);assert.equal(new Set(r.jobs.map(x=>x.id)).size,r.jobs.length);
   for(const job of r.jobs){assert.equal(job.status,'completed');assert.equal(job.conclusion,'success');}
  }
 }
 const m=read('docs/admission/deterministic-mvp-provider-v1.json');
 assert.equal(m.status,'ADMITTED');assert.equal(m.certified_closure_sha,h.steps[6].closure_sha);
 assert.deepEqual(m.model_artifacts,[]);assert.deepEqual(m.production_dependencies_added,[]);
});

test('Phase 7 current closure authority preserves explicit requests and Phase 8 boundary',()=>{
 const states=[];
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-7-roadmap.md']){
  const s=fs.readFileSync(p,'utf8').split('<!-- phase7-step8-current -->')[1]?.split('<!-- phase7-step8-history -->')[0];assert.ok(s,p);
  for(const t of ['Steps 1-7 COMPLETE / FULL PASS','Phase 8 NOT STARTED','reference.assistance.v1','reference.deterministic.v1','NONE REQUIRED / NONE ADMITTED','HUMAN Apply','navigation-only','free-form Intent v1 uninterpreted','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(t),p+': '+t);
  const state=s.match(/Step 8 (AUTHORIZED \/ IN PROGRESS|CLOSURE CERTIFICATION PENDING)/)?.[1];assert.ok(state,p);states.push(state);
 }
 assert.equal(new Set(states).size,1);
 const m=read('docs/reports/phase-7-closure-v1.json');
 if(m.status==='CLOSURE_CERTIFICATION_PENDING'){
  assert.match(m.certified_audit_sha,/^[a-f0-9]{40}$/);assert.equal(m.audit_certification.runs.length,2);
  for(const [i,r] of m.audit_certification.runs.entries()){
   assert.equal(r.head_sha,m.certified_audit_sha);assert.equal(r.attempt,1);assert.equal(r.status,'completed');assert.equal(r.conclusion,'success');
   assert.equal(r.jobs.length,i===0?7:3);for(const j of r.jobs){assert.equal(j.status,'completed');assert.equal(j.conclusion,'success');}
  }
 }else assert.equal(m.status,'AUDIT_IN_PROGRESS');
});
