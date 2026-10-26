#!/usr/bin/env python3
"""Round 3 offline paired-reference experiment. No editing or network APIs."""
import argparse
import importlib.util
import json
import math
from pathlib import Path
import random
import statistics
import struct
import subprocess
import sys
import time
import zlib

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
loader = importlib.util.spec_from_file_location('round2', ROOT/'scripts/qualify-intelligence.py')
q = importlib.util.module_from_spec(loader)
loader.loader.exec_module(q)
BASE = ROOT/'docs/benchmarks/natural-images-v1'

def read(path): return json.loads(Path(path).read_text())
def write(path,value): Path(path).write_text(json.dumps(value,indent=2,allow_nan=False)+'\n')

def decode(path):
    data=Path(path).read_bytes(); assert data[:8]==b'\x89PNG\r\n\x1a\n'
    offset=8; compressed=b''
    while offset<len(data):
        n=struct.unpack('>I',data[offset:offset+4])[0];kind=data[offset+4:offset+8];chunk=data[offset+8:offset+8+n];offset+=12+n
        if kind==b'IHDR': assert struct.unpack('>IIBBBBB',chunk)==(224,224,8,2,0,0,0)
        if kind==b'IDAT': compressed+=chunk
    raw=zlib.decompress(compressed);assert len(raw)==224*(1+224*3)
    assert all(raw[y*673]==0 for y in range(224))
    return b''.join(raw[y*673+1:(y+1)*673] for y in range(224))

def transform(pixels,tool,amount):
    # amount > 0 is the known condition needing the rule's positive correction.
    d=amount;out=[]
    if tool=='detail.sharpen':
        smooth=pixels
        for _ in range(3): smooth=q.blur(smooth,224)
        for p,b in zip(pixels,smooth):
            out.append(tuple(min(1,max(0,c+(s-c)*d if d>=0 else c+(c-s)*(-d))) for c,s in zip(p,b)))
        return out
    for p in pixels:
        y=q.luma(p)
        if tool=='adjust.exposure': v=[c*2**(-.5*d) for c in p]
        elif tool=='adjust.brightness':v=[c-.10*d for c in p]
        elif tool=='adjust.contrast':v=[.18+(c-.18)*2**(-.25*d) for c in p]
        elif tool=='adjust.highlights':v=[c*2**(.5*d*(y>.5)) for c in p]
        elif tool=='adjust.shadows':v=[c*2**(-.5*d*(y<.25)) for c in p]
        elif tool=='adjust.saturation':v=[y+(c-y)*1.2**(-d) for c in p]
        else:v=[p[0]*(1+.18*d),p[1],p[2]*(1-.18*d)]
        out.append(tuple(min(1,max(0,c)) for c in v))
    return out

def metric(rgb,tool):
    p=[tuple(q.linear(c) for c in rgb[i:i+3]) for i in range(0,len(rgb),3)]
    ys=[q.luma(v) for v in p]
    if tool in ('adjust.exposure','adjust.brightness'):return statistics.fmean(ys)
    if tool=='adjust.contrast':return statistics.pstdev(ys)
    if tool=='adjust.highlights':return statistics.fmean(max(0,y-.5) for y in ys)
    if tool=='adjust.shadows':return statistics.fmean(min(y,.25) for y in ys)
    if tool=='adjust.saturation':return statistics.fmean(max(v)-min(v) for v in p)
    if tool=='adjust.temperature':return statistics.fmean(v[0]-v[2] for v in p)
    return statistics.fmean(abs(ys[i]-ys[i-1]) for i in range(1,len(ys)) if i%224)

def cases(split,config):
    photos=[p for p in read(BASE/'sources.json')['photos'] if p['split']==split]
    for photo in photos:
        rgb=decode(BASE/(photo['id']+'.png'))
        original=[tuple(q.linear(c) for c in rgb[i:i+3]) for i in range(0,len(rgb),3)]
        for rule in config['rules']:
            for amount in config['levels']:
                changed=rgb if amount==0 else q.pixels8(transform(original,rule['tool'],amount))
                yield {'id':photo['id']+':'+rule['tool']+':'+str(amount),'base':photo['id'],'domain':photo['domain'],
                       'tool':rule['tool'],'amount':amount,'variant':'level','pixels':changed,'reference':rgb}
            if photo.get('stress'):
                a=transform(original,rule['tool'],1);b=transform(original,rule['tool'],-1)
                conflict=q.pixels8([a[i] if i%224<112 else b[i] for i in range(224*224)])
                other='adjust.temperature' if rule['tool']!='adjust.temperature' else 'adjust.exposure'
                combo=q.pixels8(transform(transform(original,rule['tool'],1),other,1))
                for name,pixels,target in [('conflict',conflict,0),('combination',combo,1)]:
                    yield {'id':photo['id']+':'+rule['tool']+':'+name,'base':photo['id'],'domain':photo['domain'],
                           'tool':rule['tool'],'amount':target,'variant':name,'pixels':pixels,'reference':rgb}

def estimate(evidence,slope):return evidence*slope
def prediction(value,rule,config):
    if not math.isfinite(value):raise ValueError('nonfinite evidence')
    if abs(value)<config['abstention']:return None
    if value<0 and rule['opposite'] is None:return None
    strength=config['bins'][0] if abs(value)<.45 else config['bins'][1] if abs(value)<1 else config['bins'][2]
    endpoint=rule['positive'] if value>0 else rule['opposite']
    result=round(rule['neutral']+(endpoint-rule['neutral'])*strength)
    registry=read(ROOT/'tests/fixtures/agent-tools-v1.json')
    q.validate_value(result,rule,registry)
    return result

def fit(records,config):
    slopes={}
    for method in ('metric','candidate'):
        slopes[method]={}
        for rule in config['rules']:
            rows=[r for r in records if r['tool']==rule['tool'] and r['variant']=='level']
            denominator=math.fsum(r[method]**2 for r in rows)
            slopes[method][rule['tool']]=math.fsum(r[method]*r['amount'] for r in rows)/denominator if denominator>1e-20 else 0
    return slopes

def metrics(records,slopes,config):
    result={}
    for method in ('reference','metric','candidate'):
        result[method]={}
        for rule in config['rules']:
            rows=[r for r in records if r['tool']==rule['tool']];answers=[]
            for r in rows:
                v=1 if method=='reference' else estimate(r[method],slopes[method][rule['tool']])
                action=rule['reference'] if method=='reference' else prediction(v,rule,config)
                direction=0 if abs(v)<config['abstention'] else 1 if v>0 else -1
                expected=0 if r['amount']==0 else 1 if r['amount']>0 else -1
                answers.append((r,direction==expected,action,direction))
            regular=[a for a in answers if a[0]['variant']=='level'];positive=[a for a in regular if a[0]['amount']!=0]
            error=[]
            for r,correct,action,direction in regular:
                if action is not None and (r['amount']>=0 or rule['opposite'] is not None):
                    target=rule['neutral']+(rule['positive' if r['amount']>=0 else 'opposite']-rule['neutral'])*abs(r['amount'])
                    error.append(abs(action-target))
            result[method][rule['tool']]={'direction_correct':sum(a[1] for a in positive),'direction_total':len(positive),
              'weak_correct':sum(a[1] for a in positive if abs(a[0]['amount'])==.2),'weak_total':sum(abs(a[0]['amount'])==.2 for a in positive),
              'strong_correct':sum(a[1] for a in positive if abs(a[0]['amount'])==1),'strong_total':sum(abs(a[0]['amount'])==1 for a in positive),
              'abstentions':sum(a[2] is None for a in regular),'false_positives':sum(a[2] is not None and a[0]['amount']==0 for a in regular),
              'false_negatives':sum(a[2] is None and a[0]['amount']!=0 and not(a[0]['amount']<0 and rule['opposite'] is None) for a in regular),
              'invalid_output':0,'parameter_unit':rule['parameter'],'parameter_error_samples':len(error),'mean_native_error':statistics.fmean(error) if error else None,
              'conflict_abstentions':sum(a[2] is None for a in answers if a[0]['variant']=='conflict'),
              'combination_direction_correct':sum(a[1] for a in answers if a[0]['variant']=='combination'),
              'domains':{d:{'correct':sum(a[1] for a in positive if a[0]['domain']==d),'total':sum(a[0]['domain']==d for a in positive)} for d in sorted({r['domain'] for r in rows})}}
    # Paired bootstrap resamples BASE PHOTOGRAPHS, never correlated derivatives.
    intervals={};rng=random.Random(73003)
    for rule in config['rules']:
        rows=[r for r in records if r['tool']==rule['tool'] and r['variant']=='level' and r['amount']!=0]
        bybase={}
        for r in rows:
            target=1 if r['amount']>0 else -1
            def correct(method):
                v=estimate(r[method],slopes[method][rule['tool']]);return int((0 if abs(v)<config['abstention'] else 1 if v>0 else -1)==target)
            bybase.setdefault(r['base'],[]).append(correct('candidate')-correct('metric'))
        means=[statistics.fmean(v) for v in bybase.values()];samples=sorted(statistics.fmean(rng.choices(means,k=len(means))) for _ in range(2000))
        intervals[rule['tool']]={'candidate_minus_metric':statistics.fmean(means),'bootstrap_95_percentile':[samples[49],samples[1949]],'unit':'direction accuracy fraction','base_clusters':len(means),'replicates':2000,'seed':73003}
    return {'methods':result,'paired_differences':intervals,'absolute_auto_correction':'ABSOLUTE AUTO-CORRECTION STILL UNQUALIFIED'}

def run(args):
    config=read(BASE/'config.json');out=Path(args.output).resolve();assert out.is_relative_to(ROOT);out.mkdir(parents=True,exist_ok=True)
    frozen=read(BASE/'frozen-policy.json') if args.split=='holdout' else None
    if frozen:
        for path,digest in frozen['bindings'].items():assert q.file_sha(ROOT/path)==digest
        marker=out/'holdout-started.json'
        with marker.open('x') as f:json.dump({'policy':q.file_sha(BASE/'frozen-policy.json'),'started_unix':time.time()},f)
    model=ROOT/'build/phase7-step7-audit/embeddinggemma-2-text-vision-440m.litertlm'
    runtime=ROOT/'build/phase7-step7-audit/runtime'
    assert q.file_sha(model)==config['model_sha256']
    assert q.file_sha(runtime.parent/'runtime-api.whl')==config['runtime_wheel_sha256']
    sys.path.insert(0,str(runtime));import litert_lm
    cache=out/'cache';cache.mkdir(exist_ok=True);before=q.memory();start=time.perf_counter()
    engine=litert_lm.EmbeddingEngine(str(model),backend=litert_lm.Backend.CPU(thread_count=1),vision_backend=litert_lm.Backend.CPU(thread_count=1),audio_backend=None,cache_dir=str(cache),min_input_length=128,max_input_length=128,vision_tokens_per_image=70)
    load=time.perf_counter()-start;records=[];latencies=[];metric_times=[];vector_cache={};repeat_max=0
    def embed(rgb,repeat=0):
        key=q.sha(rgb)+':'+str(repeat)
        if key not in vector_cache:
            file=out/'input.png';file.write_bytes(q.png(rgb,224));t=time.perf_counter()
            v=engine.compute_embedding(litert_lm.Content.ImageFile(str(file)),options=litert_lm.EmbeddingOptions(normalize=True,output_size=768,vision_tokens_per_image=70,input_overflow_strategy=litert_lm.InputOverflowStrategy.ERROR)).embedding
            latencies.append(time.perf_counter()-t);assert len(v)==768 and all(math.isfinite(x) and abs(x)<=1.0001 for x in v)
            vector_cache[key]=v
        return vector_cache[key]
    try:
        axes={}
        for rule in config['rules']:
            vectors=[]
            for text in rule['prototypes'][:2]:
                v=engine.compute_embedding('task: classification | text: '+text,options=litert_lm.EmbeddingOptions(normalize=True,output_size=768,input_overflow_strategy=litert_lm.InputOverflowStrategy.ERROR)).embedding
                assert len(v)==768 and all(math.isfinite(x) for x in v);vectors.append(v)
            axes[rule['tool']]=[a-b for a,b in zip(*vectors)]
        for i,c in enumerate(cases(args.split,config)):
            t=time.perf_counter();m=metric(c['pixels'],c['tool'])-metric(c['reference'],c['tool']);metric_times.append(time.perf_counter()-t)
            axis=axes[c['tool']];v=embed(c['pixels']);base=embed(c['reference'])
            evidence=math.fsum((a-b)*axis[j] for j,(a,b) in enumerate(zip(v,base)))
            repeat=evidence
            if args.split=='holdout':
                v2=embed(c['pixels'],1);b2=embed(c['reference'],1);repeat=math.fsum((a-b)*axis[j] for j,(a,b) in enumerate(zip(v2,b2)))
                repeat_max=max(repeat_max,max(abs(a-b) for a,b in zip(v,v2)))
            assert math.isfinite(evidence) and math.isfinite(m)
            records.append({k:v for k,v in c.items() if k not in ('pixels','reference')}|{'metric':m,'candidate':evidence,'candidate_repeat':repeat})
            if i%56==0:print(args.split,i+1,flush=True)
        active=q.memory()
    finally:engine.close()
    js="import {deterministicBaseline as b} from './scripts/intelligence-admission.mjs';let s='';for await(const c of process.stdin)s+=c;console.log(JSON.stringify(JSON.parse(s).map(x=>b.propose(x).actions[0].value)));"
    requests=[{'request':{'tool':r['tool'],'value':next(rule['reference'] for rule in config['rules'] if rule['tool']==r['tool'])}} for r in records]
    reference=json.loads(subprocess.run(['node','--input-type=module','-e',js],cwd=ROOT,input=json.dumps(requests),capture_output=True,text=True,check=True).stdout)
    assert reference==[r['request']['value'] for r in requests]
    slopes=frozen['slopes'] if frozen else fit(records,config)
    repeat_consistency={}
    if args.split=='holdout':
        for rule in config['rules']:
            rows=[r for r in records if r['tool']==rule['tool']]
            slope=slopes['candidate'][rule['tool']]
            repeat_consistency[rule['tool']]={'cases':len(rows),'identical_recommendations':sum(prediction(r['candidate']*slope,rule,config)==prediction(r['candidate_repeat']*slope,rule,config) for r in rows)}
    result={'version':1,'scope':'OFFLINE_ONLY_NO_ADMISSION','split':args.split,'config_sha256':q.file_sha(BASE/'config.json'),'source_sha256':q.file_sha(BASE/'sources.json'),
      'records':records,'slopes':slopes,'scores':metrics(records,slopes,config),'load_seconds':load,'first_inference_seconds':latencies[0],
      'warm_seconds':{'median':statistics.median(latencies[1:]),'p95':sorted(latencies[1:])[int(.95*(len(latencies)-2))]},
      'metric_seconds':{'median':statistics.median(metric_times),'maximum':max(metric_times)},'memory':{'before':before,'active':active,'after_close':q.memory()},
      'embedding_repeat_max_difference':repeat_max if args.split=='holdout' else None,'repeat_consistency':repeat_consistency,'embedding_calls':len(latencies),'production_admitted':False}
    write(out/'results.json',result)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--split',choices=['development','holdout'],required=True);p.add_argument('--output',required=True);run(p.parse_args())
