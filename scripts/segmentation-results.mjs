// Bounded extraction of engineering JSON rows from native/hosted test logs.
import fs from 'node:fs';
import assert from 'node:assert/strict';
import readline from 'node:readline';
const [input,output,evidence]=process.argv.slice(2);
assert.ok(input&&output&&evidence,'Usage: node scripts/segmentation-results.mjs LOG OUTPUT.jsonl EVIDENCE');
assert.ok(fs.statSync(input).size<=64*1024*1024,'Log bound exceeded');
const probe=fs.openSync(input,'r'),bom=Buffer.alloc(2);fs.readSync(probe,bom,0,2,0);fs.closeSync(probe);
const encoding=bom[0]===255&&bom[1]===254?'utf16le':'utf8';
const fd=fs.openSync(output,'w');let count=0;const summary=new Map(),pending=new Map();
try{
    const lines=readline.createInterface({input:fs.createReadStream(input,{highWaterMark:4096,encoding}),crlfDelay:Infinity});
    for await(const line of lines){
        let encoded;const fragment=line.indexOf('PXSEG_PART ');
        if(fragment>=0){
            assert.ok(line.length<=8192,'Fragment line bound');
            const match=line.slice(fragment).match(/^PXSEG_PART ([1-9][0-9]{0,19}) ([0-4]) ([1-5]) (.*)$/);
            assert.ok(match,'Malformed fragment envelope');const [,id,indexText,totalText,body]=match,index=Number(indexText),total=Number(totalText);
            assert.ok(index<total&&body.length<=700,'Fragment size/index bound');
            let item=pending.get(id);if(!item){assert.equal(index,0,'Missing first fragment');assert.ok(pending.size<8,'Pending row bound');item={total,next:0,text:''};pending.set(id,item);}
            assert.equal(total,item.total);assert.equal(index,item.next,'Duplicate/out-of-order fragment');item.next++;item.text+=body;
            assert.ok(item.text.length<3500,'Reassembled row bound');if(item.next<total)continue;
            encoded=item.text;pending.delete(id);
        }else{const at=line.indexOf('PXSEG {');if(at<0)continue;encoded=line.slice(at+6);}
        assert.ok(line.length<=8192,'Result line bound');assert.ok(++count<=512,'Result count bound');
        const row=JSON.parse(encoded);assert.equal(row.evaluation_version,1);assert.equal(row.metric_version,1);
        assert.deepEqual(row.threshold,[1,2]);assert.ok(row.width>0&&row.width<=256&&row.height>0&&row.height<=256);
        assert.match(row.expected_sha256,/^[a-f0-9]{64}$/);assert.equal(row.production_selection,'UNRESOLVED');
        assert.equal(row.counts.length,4);assert.ok(row.counts.every(n=>Number.isSafeInteger(n)&&n>=0));
        const [tp,fp,fn,tn]=row.counts,total=tp+fp+fn+tn;assert.equal(total,row.width*row.height);
        const fractions={iou:[tp,tp+fp+fn],dice:[2*tp,2*tp+fp+fn],accuracy:[tp+tn,total],precision:[tp,tp+fp],recall:[tp,tp+fn]};
        for(const [name,[n,d]] of Object.entries(fractions)){
            const got=row[name];assert.equal(got.length,2);assert.ok(got.every(Number.isSafeInteger));assert.ok(got[1]>0);
            const empty=(name==='precision'?tp+fn:name==='recall'?tp+fp:0)?0:1;
            assert.equal(got[0]*(d||1),(d?n:empty)*got[1]);
        }
        if(row.provider_id){assert.match(row.manifest_sha256,/^[a-f0-9]{64}$/);assert.match(row.payload_sha256,/^[a-f0-9]{64}$/);assert.equal(row.warm_count,3);}
        fs.writeSync(fd,JSON.stringify({evidence,...row})+'\n');
        const key=row.provider_id??row.predictor;const s=summary.get(key)??{rows:0,iou_min:1,iou_max:0,warm_median_ns_min:null,warm_median_ns_max:0,model_bytes:row.model_bytes??null,package_bytes:row.package_bytes??null};
        const iou=row.iou[0]/row.iou[1];s.rows++;s.iou_min=Math.min(s.iou_min,iou);s.iou_max=Math.max(s.iou_max,iou);
        if(row.warm_median_ns!==undefined){s.warm_median_ns_min=Math.min(s.warm_median_ns_min??Infinity,row.warm_median_ns);s.warm_median_ns_max=Math.max(s.warm_median_ns_max,row.warm_median_ns);}summary.set(key,s);
    }
    assert.equal(pending.size,0,'Incomplete fragmented result');assert.ok(count>0,'No evaluation evidence observed');console.log(JSON.stringify({evidence,rows:count,providers:Object.fromEntries(summary)},null,2));
}finally{fs.closeSync(fd);}
