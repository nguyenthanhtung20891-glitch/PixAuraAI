import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import crypto from 'node:crypto';
const text=p=>fs.readFileSync(p,'utf8').replaceAll('\r\n','\n');
const read=p=>JSON.parse(text(p));
const hash=s=>crypto.createHash('sha256').update(s).digest('hex');
const path='docs/reports/phase-8-step-1-v1.json';
const titles=['Review Experience Contract & Information Architecture','Before/After Preview Foundation','Plan Inspection & Action Review','Editable Review Before Apply','History / Result Review Integration','Cross-Platform Polish & Phase 8 Closure'];

test('Phase 8 freezes exactly six scopes and authorizes only contract Step 1',()=>{
 const m=read(path);assert.equal(m.version,1);assert.equal(m.runtime_authority,false);assert.equal(m.production_implementation,false);
 assert.equal(m.scope,'DOCUMENTATION_GOVERNANCE_ONLY');assert.equal(m.step,1);assert.equal(m.phase,8);
 assert.equal(m.step2,'NOT_STARTED');assert.equal(m.roadmap.length,6);
 assert.deepEqual(m.roadmap.map(r=>r.name),titles);assert.deepEqual(m.roadmap.map(r=>r.step),[1,2,3,4,5,6]);
 assert.ok(['AUTHORIZED_IN_PROGRESS','CLOSURE_CERTIFICATION_PENDING'].includes(m.status));
 assert.equal(m.roadmap[0].status,m.status);for(const r of m.roadmap.slice(1))assert.equal(r.status,'NOT_STARTED');
 const plan=text('docs/plans/phase-8-roadmap.md');for(const [i,title] of titles.entries())assert.ok(plan.includes(`| ${i+1} | ${title} |`));
 assert.ok(plan.includes('No Step 7 without Product Owner / Principal Architect approval'));
});

test('review presentation graph cannot reach Apply by inspecting or previewing',()=>{
 const m=read(path),states=['NO_REVIEW','PLAN_REVIEW','PREVIEW_LOADING','PREVIEW_READY','PREVIEW_FAILED','STALE','APPLY_PENDING','APPLY_EXECUTING','APPLY_SUCCEEDED','APPLY_FAILED','CANCELLED'];
 assert.deepEqual(m.states,states);const tuples=new Set();
 for(const t of m.transitions){assert.ok(states.includes(t.from)&&states.includes(t.to));assert.ok(t.guard);const key=t.from+':'+t.event;assert.ok(!tuples.has(key));tuples.add(key);}
 const reached=new Set(['NO_REVIEW']);let changed=true;
 while(changed){changed=false;for(const t of m.transitions)if(t.event!=='human_apply'&&reached.has(t.from)&&!reached.has(t.to)){reached.add(t.to);changed=true;}}
 for(const s of ['APPLY_PENDING','APPLY_EXECUTING','APPLY_SUCCEEDED'])assert.ok(!reached.has(s),s);
 const apply=m.transitions.filter(t=>t.to==='APPLY_PENDING');assert.equal(apply.length,3);
 for(const t of apply){assert.equal(t.event,'human_apply');assert.equal(t.guard,'explicit_human_and_native_exact_current_review');assert.ok(['PLAN_REVIEW','PREVIEW_READY','PREVIEW_FAILED'].includes(t.from));}
 for(const t of m.transitions.filter(t=>t.to==='APPLY_EXECUTING'))assert.equal(t.guard,'existing_single_use_execution_admission');
 for(const t of m.transitions.filter(t=>t.to==='APPLY_SUCCEEDED'))assert.equal(t.guard,'valid_authoritative_publication_receipt');
 for(const s of ['STALE','APPLY_SUCCEEDED','APPLY_FAILED'])assert.ok(!m.transitions.some(t=>t.from===s&&t.to==='APPLY_PENDING'));
 assert.equal(m.transitions.find(t=>t.from==='CANCELLED'&&t.to==='PLAN_REVIEW').guard,'preview_scope_only_and_native_current');
});

test('editable review is a fresh untrusted candidate, never mutable approved authority',()=>{
 const m=read(path);assert.equal(m.review_authority,false);assert.equal(m.preview_authority,false);assert.equal(m.review_persisted,false);assert.equal(m.review_identity_is_capability,false);
 assert.equal(m.plan_limit_bytes,8192);assert.equal(m.plan_limit_actions,8);
 assert.deepEqual(m.forbidden_session_fields,['approval_token','approval_issuer','candidate_handle','revision_publisher','history_owner','durable_execution_token','cancellation_capability']);
 assert.deepEqual(m.review_edits.map(e=>e.operation),['parameter_change','action_remove','action_reorder','restore']);
 for(const e of m.review_edits){assert.equal(e.invalidates_old_review,true);assert.equal(e.requires_full_native_validation,true);assert.equal(e.requires_new_plan_and_review_identity,true);assert.equal(e.reuses_approval,false);}
 const s=text(m.contract);
 for(const rule of ['REVIEW != APPROVAL','PREVIEW != EXECUTION','PREVIEW != REVISION','UI STATE != NATIVE AUTHORITY','NEW INTENT / NEW PLAN CANDIDATE FOR VALIDATION','NEW exact canonical Plan bytes and NEW review identity','No action is assumed reorderable or commutative','Schema 3','outcome unconfirmed','Stop -> join ALL calls -> destroy'])assert.ok(s.includes(rule),rule);
});

test('new review contract is additive and Phase 7 certified authority remains immutable',()=>{
 const m=read(path);assert.equal(m.baseline,'fe01c4a4222eb5bed63adc840b140f73784e1c18');
 assert.deepEqual(m.approved_additive_contracts,['docs/contracts/full-review-experience-v1.md']);
 assert.equal(hash(text(m.contract)),m.contract_sha256_lf);
 assert.equal(hash(text(m.frozen_phase7_evidence)),m.phase7_closure_manifest_sha256_lf);
 const b=read(m.baseline_certification);assert.equal(b.sha,m.baseline);assert.equal(b.runs.length,2);
 for(const [i,r] of b.runs.entries()){
  assert.equal(r.head_sha,m.baseline);assert.equal(r.attempt,1);assert.equal(r.status,'completed');assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i===0?7:3);
  for(const j of r.jobs){assert.equal(j.status,'completed');assert.equal(j.conclusion,'success');}
 }
 assert.equal(m.provider,'reference.assistance.v1');assert.equal(m.planner,'reference.deterministic.v1');
 assert.equal(m.production_learned_model,'NONE_REQUIRED_NONE_ADMITTED_FOR_PHASE_7_MVP');
 for(const p of ['.github/workflows/foundation.yml','.github/workflows/native-shells.yml'])assert.ok(text(p).includes('tests/full-review-contract.test.mjs'));
});

test('all current authorities preserve Step 2 stop, model-free scope and trust',()=>{
 const m=read(path),states=[];
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md','docs/plans/phase-8-roadmap.md','docs/reports/phase-8-step-1.md']){
  const s=text(p).split('<!-- phase8-step1-current -->')[1]?.split('<!-- phase8-step1-history -->')[0];assert.ok(s,p);
  const state=s.match(/Step 1 (AUTHORIZED \/ IN PROGRESS|CLOSURE CERTIFICATION PENDING)/)?.[1];assert.ok(state,p);states.push(state);
  assert.ok(s.includes('Steps 2-6 NOT STARTED'),p);
 }
 assert.equal(new Set(states).size,1);assert.equal(states[0],m.status==='AUTHORIZED_IN_PROGRESS'?'AUTHORIZED / IN PROGRESS':'CLOSURE CERTIFICATION PENDING');
 for(const p of ['AGENTS.md','ARCHITECTURE.md','DECISIONS.md','QUALITY_GATES.md','ROADMAP.md']){
  const s=text(p).split('<!-- phase8-step1-current -->')[1].split('<!-- phase8-step1-history -->')[0];
  for(const t of ['Phase 7 CLOSED / COMPLETE / FULL PASS','HUMAN Apply','Schema 3 unchanged','reference.assistance.v1 ADMITTED','reference.deterministic.v1','NONE REQUIRED / NONE ADMITTED','Vulkan/Metal UNKNOWN','NOT WAIVED','NOT SATISFIED'])assert.ok(s.includes(t),p+t);
 }
 if(m.status==='CLOSURE_CERTIFICATION_PENDING'){
  assert.match(m.certified_implementation_sha,/^[a-f0-9]{40}$/);assert.equal(m.implementation_certification.runs.length,2);
  for(const [i,r] of m.implementation_certification.runs.entries()){
   assert.equal(r.head_sha,m.certified_implementation_sha);assert.equal(r.attempt,1);assert.equal(r.status,'completed');assert.equal(r.conclusion,'success');assert.equal(r.jobs.length,i===0?7:3);
   for(const j of r.jobs){assert.equal(j.status,'completed');assert.equal(j.conclusion,'success');}
  }
 }
});

test('review threat protections are explicit and future preview is not claimed implemented',()=>{
 const m=read(path),s=text(m.contract);assert.deepEqual(m.threats,'ABCDEFGHIJK'.split(''));
 for(const id of m.threats)assert.match(s,new RegExp('\\| '+id+' [^\\n]+\\|'));
 for(const t of ['BEFORE','AFTER PREVIEW','POST-APPLY RESULT REVIEW','no image upload','no screenshot/image telemetry','No production platform code/bridge changes in Step 1','Step 1 checks the contract/governance','Future steps','Manual','PREVIEW work class'])assert.ok(s.toLowerCase().includes(t.toLowerCase()),t);
});
