import fs from 'node:fs';
import assert from 'node:assert/strict';
// Inspect actual production-configuration objects/libraries, not just source text.
const testAnchor=Buffer.from('d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a','hex');
for(const file of process.argv.slice(2)){
    const b=fs.readFileSync(file);
    for(const marker of [testAnchor,Buffer.from('pixaura_model_test_boundary'),Buffer.from('pixaura_inference_test_boundary'),Buffer.from('pixaura_inference_android_test_boundary'),Buffer.from('pixaura_inference_apple_test_boundary'),Buffer.from('synthetic-segmentation'),Buffer.from('PXSEG'),Buffer.from('production_selection'),Buffer.from('AdmissionTestExecution'),Buffer.from('admission_test_')])
        assert.equal(b.includes(marker),false,`${file}: test trust/certification surface in production object`);
    for(const marker of ['pixaura_test_agent_durable','pixaura_test_agent_external_session','pixaura_test_agent_epoch_exhaust','agent_fail_transaction','pixaura_test_agent_storage_failure','agent_transaction_failure','pixaura_test_agent_candidate_state','pixaura_test_agent_execution_checkpoint','agent_checkpoint_state','provider_security_certification','certification_output_fault','cpu_execution_calls','PXSEC','pixaura_agent_schema_test_','Agent schema v1 goldens','agent_tools_golden','pixaura.test.only','Agent tools v1 registry=11 exact','reference_planner_fixtures','Reference planner v1 goldens='])
        assert.equal(b.includes(Buffer.from(marker)),false,`${file}: Step 7 test surface in production object`);
    for(const marker of ['pixaura_test_agent_platform_control','AgentPlatformHooks_nativeControl','platform_checkpoint'])
        assert.equal(b.includes(Buffer.from(marker)),false,`${file}: platform certification hook in release`);
    console.log(`Model production trust isolation PASS: ${b.length} bytes`);
}
assert.ok(process.argv.length>2,'No production object inspected');
