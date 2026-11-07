import fs from 'node:fs';import assert from 'node:assert/strict';
const manifest=JSON.parse(fs.readFileSync('docs/reports/phase-8-step-4-v1.json','utf8'));
const resultManifest=JSON.parse(fs.readFileSync('docs/reports/phase-8-step-5-v1.json','utf8'));
// Narrow successor chain only. Historical manifests/evidence stay immutable.
export function reviewEditHash(path, expected){
 const change=manifest.changes[path];if(change)assert.equal(change.before_sha256_lf,expected,path+' Step 4 predecessor');
 return change?.after_sha256_lf??expected;
}
export function resultReviewHash(path, expected){
 const change=resultManifest.changes[path];if(change)assert.equal(change.before_sha256_lf,expected,path+' Step 5 predecessor');
 const after=change?.after_sha256_lf??expected;
 const exportManifest=JSON.parse(fs.readFileSync('docs/reports/phase-9-step-2-v1.json','utf8'));
 const next=exportManifest.changes[path];if(next)assert.equal(next.before_sha256_lf,after,path+' export predecessor');
 const encoded=next?.after_sha256_lf??after;
 const android=JSON.parse(fs.readFileSync('docs/reports/phase-9-step-3-v1.json','utf8'));
 const saved=android.changes[path];if(saved)assert.equal(saved.before_sha256_lf,encoded,path+' android export predecessor');
 const stored=saved?.after_sha256_lf??encoded;
 const apple=JSON.parse(fs.readFileSync('docs/reports/phase-9-step-4-v1.json','utf8'));
 const shared=apple.changes[path];if(shared)assert.equal(shared.before_sha256_lf,stored,path+' apple export predecessor');
 const presented=shared?.after_sha256_lf??stored;
 const empty=JSON.parse(fs.readFileSync('docs/reports/phase-10-step-2-v1.json','utf8'));
 const opened=empty.changes[path];if(opened)assert.equal(opened.before_sha256_lf,presented,path+' empty predecessor');
 const shown=opened?.after_sha256_lf??presented;
 const locale=JSON.parse(fs.readFileSync('docs/reports/phase-10-step-3-v1.json','utf8'));
 const translated=locale.changes[path];if(translated)assert.equal(translated.before_sha256_lf,shown,path+' locale predecessor');
 const localized=translated?.after_sha256_lf??shown;
 const metal=JSON.parse(fs.readFileSync('docs/reports/phase-10.1-metal-v1.json','utf8'));
 const family=metal.changes[path];if(family)assert.equal(family.before_sha256_lf,localized,path+' metal family predecessor');
 const admitted=family?.after_sha256_lf??localized;
 const capture=JSON.parse(fs.readFileSync('docs/reports/phase-10.1-swift-capture-v1.json','utf8'));
 const owned=capture.changes[path];if(owned)assert.equal(owned.before_sha256_lf,admitted,path+' swift capture predecessor');
 return owned?.after_sha256_lf??admitted;
}
export function withoutReviewEditIntegration(source){
 return source.replace('#include "review_edit_state.hpp"\n','').replace('    ReviewEditState review_edit;\n','')
  .replace('        review_edit.abandon();\n','').replace('#include "review_edit_boundary.hpp"\n','')
  .replace('#include "pixaura/result_review.h"\n','').replace('#include "result_review_boundary.hpp"\n','')
  .replace('#include "pixaura/export.h"\n','').replace('#include "export_boundary.hpp"\n','');
}
