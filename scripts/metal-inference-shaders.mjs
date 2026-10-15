// Engineering-only embedding of reviewed repository code, never model data.
import fs from 'node:fs';
const root='packages/core/swift/Sources/Shaders/';
const shader=name=>fs.readFileSync(root+name+'.metal','utf8').replace(/\r\n/g,'\n');
const text='// Generated from fixed repository Metal shaders. No model code.\n#ifndef PIXAURA_METAL_SHADERS_HPP\n#define PIXAURA_METAL_SHADERS_HPP\n'+
    ['exposure','segmentation'].map(n=>`static constexpr const char* ${n}_msl=R"PXMSL(${shader(n)})PXMSL";\n`).join('')+'#endif\n';
const file='packages/core/src/metal_shaders.hpp';
if(process.argv.includes('--check')){if(fs.readFileSync(file,'utf8')!==text)throw Error('Fixed Metal shader drift');}
else fs.writeFileSync(file,text);
console.log('Fixed exposure/segmentation Metal source identity PASS');
