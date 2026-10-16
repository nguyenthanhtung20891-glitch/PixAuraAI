// Repository-owned synthetic data. Independent integer geometry/digest oracle.
import fs from 'node:fs';
import crypto from 'node:crypto';
const names=['rectangle','ellipse','thin','disconnected','border','high-contrast','low-contrast','gradient','small','large','hole','alias-edge'];
const cases=[];
for(const size of [8,256])for(const [id,name] of names.entries()){
    const mask=Buffer.alloc(size*size);let foreground=0;const counts=[0,0,0,0];
    for(let row=0;row<size;row++)for(let col=0;col<size;col++){
        const q=size/4,inside=col>=q&&col<3*q&&row>=q&&row<3*q;
        const selected=[inside,(col+.5-size/2)**2+(row+.5-size/2)**2<=size*size/16,
            col===size/2||row===size/2,(col<q&&row<q)||(col>=3*q&&row>=3*q),col<size/2&&row<size/2,
            inside,inside,inside,col===size/2&&row===size/2,col>=1&&row>=1&&col<size-1&&row<size-1,
            inside&&!(col>=3*size/8&&col<5*size/8&&row>=3*size/8&&row<5*size/8),(col+row)%4<2][id];
        mask[row*size+col]=Number(selected);foreground+=Number(selected);
        const above=id===6||id===8?false:id===7?(selected||col>=size/2):selected;
        counts[selected?(above?0:2):(above?1:3)]++;
    }
    cases.push({id:name,size,pixels:mask.length,foreground,reference_counts:counts,expected_sha256:crypto.createHash('sha256').update(mask).digest('hex')});
}
const json={version:1,ownership:'PixAuraAI original generated synthetic geometry; no external images',selection:'UNRESOLVED',cases};
const header='// GENERATED TEST ONLY: independent geometry/digest oracle.\n#ifndef PIXAURA_SEGMENTATION_FIXTURES_HPP\n#define PIXAURA_SEGMENTATION_FIXTURES_HPP\nnamespace pixaura::segmentation {\nstruct GoldenMask {const char* id;uint32_t size;uint64_t foreground;const char* digest;Counts reference;};\ninline constexpr std::array<GoldenMask,24> masks{{\n'+cases.map(c=>`{"${c.id}",${c.size},${c.foreground},"${c.expected_sha256}",{${c.reference_counts.join(',')}}}`).join(',\n')+'\n}};\n}\n#endif\n';
for(const [path,value] of [['tests/fixtures/segmentation-evaluation-v1.json',JSON.stringify(json,null,2)+'\n'],['packages/core/tests/segmentation_fixtures.hpp',header]]){
    if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8').replaceAll('\r\n','\n')!==value)throw Error(`Stale ${path}`);}
    else fs.writeFileSync(path,value);
}
