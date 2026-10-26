#!/usr/bin/env python3
"""Offline qualification only. Optional existing local runtime; no downloads/edit API."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import resource
import statistics
import struct
import subprocess
import sys
import time
import zlib

ROOT=Path(__file__).resolve().parents[1]
SPEC=ROOT/'docs/benchmarks/intelligence-qualification-v1.json'

def canonical(v): return json.dumps(v,sort_keys=True,separators=(',',':'),allow_nan=False)+'\n'
def sha(b): return hashlib.sha256(b).hexdigest()
def file_sha(path):
    digest=hashlib.sha256()
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda:stream.read(1048576),b''):digest.update(chunk)
    return digest.hexdigest()
def srgb(v): return 12.92*v if v<=0.0031308 else 1.055*v**(1/2.4)-0.055
def linear(v):
    v=v/255
    return v/12.92 if v<=0.04045 else ((v+0.055)/1.055)**2.4
def luma(p): return .2126*p[0]+.7152*p[1]+.0722*p[2]

def base(name,size):
    out=[]
    for y in range(size):
        for x in range(size):
            if name=='chart':
                colors=((.12,.18,.24),(.30,.20,.10),(.15,.30,.18),(.35,.35,.35))
                p=colors[((x//28)+(y//28))%4]
                shade=.65+.7*x/(size-1)
                p=tuple(c*shade for c in p)
            else:
                p=(.18,.32,.55) if y<size*.48 else (.10,.24,.08)
                if (x-size*.28)**2+(y-size*.22)**2<(size*.09)**2: p=(.85,.80,.60)
                if size*.55<x<size*.78 and size*.42<y<size*.78: p=(.45,.10,.07)
                if (x//5+y//5)%2: p=tuple(c*.88 for c in p)
            out.append(p)
    return out

def blur(pixels,size):
    return [tuple(sum(pixels[min(size-1,max(0,y+dy))*size+min(size-1,max(0,x+dx))][c]
                      for dy in (-1,0,1) for dx in (-1,0,1))/9 for c in range(3))
            for y in range(size) for x in range(size)]

def perturb(pixels,size,tool,direction,strength=1):
    if tool=='detail.sharpen':
        if direction>0:
            original=pixels
            for _ in range(3): pixels=blur(pixels,size)
            return [tuple(a*(1-strength)+b*strength for a,b in zip(p,q)) for p,q in zip(original,pixels)]
        return [tuple(min(1,max(0,c+(.20 if (i*37+cidx*13)%7<3 else -.20))) for cidx,c in enumerate(p)) for i,p in enumerate(pixels)]
    out=[]
    for p in pixels:
        y=luma(p)
        if tool=='adjust.exposure': q=tuple(c*2**(-direction*.5*strength) for c in p)
        elif tool=='adjust.brightness': q=tuple(c-direction*.10*strength for c in p)
        elif tool=='adjust.contrast': q=tuple(.18+(c-.18)*2**(-direction*.25*strength) for c in p)
        elif tool=='adjust.highlights': q=tuple(c*2**(direction*.5*strength*(1 if y>.25 else 0)) for c in p)
        elif tool=='adjust.shadows': q=tuple(c*2**(-direction*.5*strength*(1 if y<.25 else 0)) for c in p)
        elif tool=='adjust.saturation':
            ratio=(1/1.2 if direction>0 else 1.25) if strength==1 else (1-.02*direction)
            q=tuple(y+(c-y)*ratio for c in p)
        else: q=(p[0]*(1+direction*.18*strength),p[1],p[2]*(1-direction*.18*strength))
        out.append(tuple(min(1,max(0,c)) for c in q))
    return out

def pixels8(pixels): return bytes(round(255*srgb(c)) for p in pixels for c in p)
def png(rgb,size):
    def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
    rows=b''.join(b'\0'+rgb[y*size*3:(y+1)*size*3] for y in range(size))
    return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',size,size,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rows,9))+chunk(b'IEND',b'')

def corpus(spec):
    size=spec['size'];assert size==224
    cases=[]
    for name in spec['bases']:
        original=base(name,size)
        for rule in spec['rules']:
            for variant in spec['variants']:
                expected=rule['positive'] if variant in ('positive','combination') else rule['opposite'] if variant=='opposite' else None
                pixels=original
                if variant in ('positive','opposite','combination','boundary'):
                    pixels=perturb(original,size,rule['tool'],-1 if variant=='opposite' else 1,.05 if variant=='boundary' else 1)
                if variant=='combination':pixels=perturb(pixels,size,'adjust.exposure' if rule['tool']=='adjust.saturation' else 'adjust.saturation',1)
                if variant=='conflict':
                    a=perturb(original,size,rule['tool'],1);b=perturb(original,size,rule['tool'],-1)
                    pixels=[a[i] if i%size<size//2 else b[i] for i in range(size*size)]
                image=png(pixels8(pixels),size)
                cases.append({'id':f'{name}-{rule["tool"]}-{variant}','tool':rule['tool'],'variant':variant,
                              'expected':expected,'image_sha256':sha(image),'png':image,'pixels':pixels8(pixels)})
    assert len(cases)==96
    return cases

def direction(rgb,size,tool):
    pixels=[tuple(linear(c) for c in rgb[i:i+3]) for i in range(0,len(rgb),3)]
    ys=[luma(p) for p in pixels];mean=statistics.fmean(ys);ordered=sorted(ys)
    if tool in ('adjust.exposure','adjust.brightness'):return 1 if mean<.20 else -1 if mean>.35 else 0
    if tool=='adjust.contrast':
        sd=statistics.pstdev(ys);return 1 if sd<.12 else -1 if sd>.25 else 0
    if tool=='adjust.highlights':return 1 if ordered[int(.95*(len(ys)-1))]>.85 else -1 if ordered[int(.95*(len(ys)-1))]<.45 else 0
    if tool=='adjust.shadows':return 1 if ordered[int(.05*(len(ys)-1))]<.04 else -1 if ordered[int(.05*(len(ys)-1))]>.12 else 0
    if tool=='adjust.saturation':
        saturation=statistics.fmean((max(p)-min(p))/max(p) if max(p) else 0 for p in pixels)
        return 1 if saturation<.25 else -1 if saturation>.65 else 0
    if tool=='adjust.temperature':
        cast=statistics.fmean(p[0]-p[2] for p in pixels);return 1 if cast>.08 else -1 if cast<-.08 else 0
    gradient=statistics.fmean(abs(ys[i]-ys[i-1]) for i in range(1,len(ys)) if i%size)
    return 1 if gradient<.025 else 0

def calibration(rgb,size,rule):
    d=direction(rgb,size,rule['tool'])
    left=b''.join(rgb[y*size*3:y*size*3+size//2*3] for y in range(size))
    right=b''.join(rgb[y*size*3+size//2*3:(y+1)*size*3] for y in range(size))
    a=direction(left,size//2,rule['tool']);b=direction(right,size//2,rule['tool'])
    if a*b<0:return None
    return rule['positive'] if d>0 else rule['opposite'] if d<0 else None

def decide(scores,value,rule,policy,margin=None):
    assert len(scores)==3 and all(math.isfinite(s) and -1.0001<=s<=1.0001 for s in scores)
    ranking=sorted(range(3),key=lambda i:(-scores[i],i));first,second=ranking[:2]
    if first==2 or scores[first]<policy['minimum_cosine'] or scores[first]-scores[second]<(policy['minimum_margin'] if margin is None else margin):return None
    proposed=rule['positive'] if first==0 else rule['opposite']
    return proposed if proposed is not None and proposed==value else None

def validate_value(value,rule,registry):
    if value is None:return
    p=next(t for t in registry['tools'] if t['tool_id']=='pixaura.'+rule['tool'].split('.')[1])['parameters'][0]
    assert type(value) is int and p['minimum']<=value<=p['maximum'] and value!=p['default']

def summarize(cases,values,spec):
    result={}
    for rule in spec['rules']:
        rows=[(c,v) for c,v in zip(cases,values) if c['tool']==rule['tool']]
        positives=sum(c['expected'] is not None for c,v in rows);negatives=len(rows)-positives
        emitted=[(c,v) for c,v in rows if v is not None]
        selected=sum(c['expected'] is not None for c,v in emitted)
        correct=sum(c['expected'] is not None and (v-rule['neutral'])*(c['expected']-rule['neutral'])>0 for c,v in emitted)
        fp=sum(c['expected'] is None for c,v in emitted);fn=sum(c['expected'] is not None and v is None for c,v in rows)
        errors=[abs(v-c['expected']) for c,v in emitted if c['expected'] is not None]
        result[rule['tool']]={'cases':len(rows),'positive_cases':positives,'negative_cases':negatives,'selection':selected,'direction_correct':correct,
          'abstention_correct':negatives-fp,'false_positives':fp,'false_negatives':fn,'false_positive_rate':fp/negatives,'false_negative_rate':fn/positives,
          'invalid_output':0,'parameter_unit':rule['parameter'],'parameter_errors':errors,'mean_parameter_error':statistics.fmean(errors) if errors else None,
          'conflict_abstentions':sum(c['variant']=='conflict' and v is None for c,v in rows)}
    return result

def memory():
    result={'peak_rss_bytes':resource.getrusage(resource.RUSAGE_SELF).ru_maxrss*1024}
    for line in Path('/proc/self/status').read_text().splitlines():
        if line.startswith(('VmRSS:','RssAnon:','RssFile:','VmHWM:')):
            k,v,*_=line.split();result[k.rstrip(':')+'_bytes']=int(v)*1024
    return result

def run(args):
    spec=json.loads(SPEC.read_text());cases=corpus(spec);out=Path(args.output).resolve();assert out.is_relative_to(ROOT);out.mkdir(parents=True,exist_ok=True)
    manifest=[{k:v for k,v in c.items() if k not in ('png','pixels')} for c in cases]
    frozen={'spec_sha256':sha(SPEC.read_bytes()),'generator_sha256':sha(Path(__file__).read_bytes()),'cases':manifest}
    # Write expectations before importing/executing the candidate runtime.
    frozen_path=out/'frozen-corpus.json';frozen_path.write_text(canonical(frozen));frozen_hash=sha(frozen_path.read_bytes())
    for c in cases:(out/(c['id']+'.png')).write_bytes(c['png'])
    if args.freeze_only:
        print(canonical({'cases':len(cases),'frozen_corpus_sha256':frozen_hash}));return
    model=Path(args.model).resolve();assert model.is_file() and file_sha(model)==spec['model_sha256']
    runtime_root=Path(args.runtime_root).resolve()
    assert file_sha(runtime_root.parent/'runtime-api.whl')==spec['runtime_wheel_sha256']
    sys.path.insert(0,str(Path(args.runtime_root).resolve()));import litert_lm
    os.environ['TMPDIR']=str(out);(out/'cache').mkdir(exist_ok=True);before=memory();start=time.perf_counter()
    engine=litert_lm.EmbeddingEngine(str(model),backend=litert_lm.Backend.CPU(thread_count=1),vision_backend=litert_lm.Backend.CPU(thread_count=1),audio_backend=None,cache_dir=str(out/'cache'),min_input_length=128,max_input_length=128,vision_tokens_per_image=70)
    load=time.perf_counter()-start;after_load=memory()
    prototypes={};latencies=[];records=[];maximum_repeat_difference=0
    try:
        prototype_start=time.perf_counter()
        for rule in spec['rules']:
            vectors=[]
            for text in rule['prototypes']:
                vector=engine.compute_embedding(spec['policy']['prototype_prefix']+text,options=litert_lm.EmbeddingOptions(normalize=True,output_size=768,input_overflow_strategy=litert_lm.InputOverflowStrategy.ERROR)).embedding
                assert len(vector)==768 and all(math.isfinite(x) for x in vector);vectors.append(vector)
            prototypes[rule['tool']]=vectors
        prototype_seconds=time.perf_counter()-prototype_start
        calibration_seconds=[]
        for index,c in enumerate(cases):
            rule=next(r for r in spec['rules'] if r['tool']==c['tool']);start=time.perf_counter();metric=calibration(c['pixels'],spec['size'],rule);calibration_seconds.append(time.perf_counter()-start)
            vectors=[];scores=[];actions=[]
            for repeat in range(spec['repeats']):
                start=time.perf_counter();v=engine.compute_embedding(litert_lm.Content.ImageFile(str(out/(c['id']+'.png'))),options=litert_lm.EmbeddingOptions(normalize=True,output_size=768,vision_tokens_per_image=70,input_overflow_strategy=litert_lm.InputOverflowStrategy.ERROR)).embedding
                latencies.append(time.perf_counter()-start);assert len(v)==768 and all(math.isfinite(x) and abs(x)<=1.0001 for x in v)
                s=[math.fsum(a*b for a,b in zip(v,p)) for p in prototypes[c['tool']]]
                action=decide(s,metric,rule,spec['policy']);validate_value(action,rule,json.loads((ROOT/'tests/fixtures/agent-tools-v1.json').read_text()))
                vectors.append(v);scores.append(s);actions.append(action)
            maximum_repeat_difference=max(maximum_repeat_difference,max(abs(a-b) for a,b in zip(*vectors)))
            records.append({'id':c['id'],'scores':scores,'candidate':actions,'metric_only':metric})
            if index%12==0:print(f'Completed {index+1}/{len(cases)} cases',flush=True)
        after_inference=memory()
    finally:engine.close()
    after_close=memory()
    # Invoke the existing Round 1 deterministic baseline, not a new image-aware baseline.
    requests=[{'request':{'tool':c['tool'],'value':next(r['reference'] for r in spec['rules'] if r['tool']==c['tool'])}} for c in cases]
    js="import {deterministicBaseline as b} from './scripts/intelligence-admission.mjs';let s='';for await(const c of process.stdin)s+=c;console.log(JSON.stringify(JSON.parse(s).map(x=>b.propose(x).actions[0].value)));"
    baseline=json.loads(subprocess.run([args.node,'--input-type=module','-e',js],cwd=ROOT,input=json.dumps(requests),capture_output=True,text=True,check=True).stdout)
    candidate=[r['candidate'][0] for r in records];metric=[r['metric_only'] for r in records]
    summary={'version':1,'scope':'OFFLINE_ONLY_NOT_PRODUCTION_ADMISSION','frozen_corpus_sha256':frozen_hash,'model_sha256':spec['model_sha256'],'artifact_bytes':model.stat().st_size,
      'reference':summarize(cases,baseline,spec),'candidate':summarize(cases,candidate,spec),'metric_only':summarize(cases,metric,spec),
      'repeat_consistent_cases':sum(r['candidate'][0]==r['candidate'][1] for r in records),'maximum_embedding_repeat_difference':maximum_repeat_difference,
      'load_seconds':load,'prototype_embedding_seconds':prototype_seconds,'calibration_seconds':{'median':statistics.median(calibration_seconds),'maximum':max(calibration_seconds)},'first_image_seconds':latencies[0],'warm_image_seconds':{'minimum':min(latencies[1:]),'median':statistics.median(latencies[1:]),'maximum':max(latencies[1:]),'p95':sorted(latencies[1:])[int(.95*(len(latencies)-2))]},
      'memory':{'before_load':before,'after_load':after_load,'after_inference':after_inference,'after_close':after_close},'records':records,
      'natural_photo_status':'INSUFFICIENT REALISTIC QUALITY EVIDENCE','production_admitted':False}
    summary['sensitivity']={str(m):summarize(cases,[decide(r['scores'][0],r['metric_only'],next(rule for rule in spec['rules'] if rule['tool']==c['tool']),spec['policy'],m) for c,r in zip(cases,records)],spec) for m in spec['policy']['sensitivity_margins']}
    (out/'results.json').write_text(json.dumps(summary,indent=2,allow_nan=False)+'\n');print('Offline measurement complete; no admission',flush=True)

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--output',required=True);parser.add_argument('--freeze-only',action='store_true');parser.add_argument('--model');parser.add_argument('--runtime-root');parser.add_argument('--node',default='node')
    run(parser.parse_args())
