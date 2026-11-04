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
 return next?.after_sha256_lf??after;
}
export function withoutReviewEditIntegration(source){
 return source.replace('#include "review_edit_state.hpp"\n','').replace('    ReviewEditState review_edit;\n','')
  .replace('        review_edit.abandon();\n','').replace('#include "review_edit_boundary.hpp"\n','')
  .replace('#include "pixaura/result_review.h"\n','').replace('#include "result_review_boundary.hpp"\n','')
  .replace('#include "pixaura/export.h"\n','').replace('#include "export_boundary.hpp"\n','');
}
