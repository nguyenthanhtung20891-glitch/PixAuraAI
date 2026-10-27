import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
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
