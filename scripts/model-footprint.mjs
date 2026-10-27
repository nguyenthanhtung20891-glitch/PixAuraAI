// Engineering evidence only. Bytes measured from built outputs, no estimates of zero cost.
import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
const root=process.argv[2];
if(!root)throw Error('Build root required');
const walk=d=>fs.readdirSync(d,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(d,e.name)):[path.join(d,e.name)]);
const all=walk(root);
const relevant=all.filter(f=>/libpixaura_(sodium|core)\.(a|so|dylib|dll)$|pixaura_sodium\.lib$|model_(package|files)\.cpp\.(o|obj)$|(?:document|manual_tools|editor_api)\.cpp\.(o|obj)$/.test(f));
const metrics=relevant.map(f=>({file:path.relative(root,f).replaceAll('\\','/'),bytes:fs.statSync(f).size}));
const assistanceSymbols=[];
if(process.platform==='linux')for(const f of all.filter(f=>/agent_(schema|execution)_release_object\.dir.*(?:document|editor_api)\.cpp\.o$/.test(f))){
    const result=spawnSync('nm',['-S','-C','--defined-only',f],{encoding:'utf8',maxBuffer:64*1024*1024});
    if(result.error||result.status!==0){assistanceSymbols.push({file:path.relative(root,f),status:'NOT_MEASURED',reason:'nm unavailable or failed'});continue;}
    const sizes=result.stdout.split('\n').map(line=>line.match(/^\s*[0-9a-f]+\s+([0-9a-f]+)\s+[TtWw]\s+.*pixaura::assistance::/)).filter(Boolean).map(match=>Number.parseInt(match[1],16));
    assistanceSymbols.push({file:path.relative(root,f),defined_text_symbols:sizes.length,attributed_text_bytes:sizes.reduce((a,b)=>a+b,0)});
}
const text=JSON.stringify({configuration:'actual build outputs; archive/object sizes are not linked deltas; debug symbols vary',metrics,
    assistance_symbols:assistanceSymbols,symbol_caveat:'Namespace-attributed text only; shared parser/callees excluded, weak symbols may coalesce; NOT isolated linked/package contribution. Empty on unsupported hosts.'},null,2)+'\n';
fs.writeFileSync(path.join(root,'model-footprint.json'),text);
console.log(text);
