// Engineering evidence only. Bytes measured from built outputs, no estimates of zero cost.
import fs from 'node:fs';
import path from 'node:path';
const root=process.argv[2];
if(!root)throw Error('Build root required');
const walk=d=>fs.readdirSync(d,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(d,e.name)):[path.join(d,e.name)]);
const all=walk(root);
const relevant=all.filter(f=>/libpixaura_(sodium|core)\.(a|so|dylib|dll)$|pixaura_sodium\.lib$|model_(package|files)\.cpp\.(o|obj)$/.test(f));
const metrics=relevant.map(f=>({file:path.relative(root,f).replaceAll('\\','/'),bytes:fs.statSync(f).size}));
const text=JSON.stringify({configuration:'actual build outputs; archive/object sizes are not linked deltas; debug symbols vary',metrics},null,2)+'\n';
fs.writeFileSync(path.join(root,'model-footprint.json'),text);
console.log(text);
