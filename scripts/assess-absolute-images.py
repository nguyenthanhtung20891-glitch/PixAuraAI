#!/usr/bin/env python3
"""Round 4 OFFLINE assessment. Reads pinned photos; has no editor/model/network API."""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import statistics
import sys
import time

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('round3', ROOT/'scripts/qualify-natural-images.py')
r3 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(r3)
BASE = ROOT/'docs/benchmarks/absolute-images-v1'
TOOLS = [r['tool'] for r in r3.read(r3.BASE/'config.json')['rules']]

def digest(data): return hashlib.sha256(data).hexdigest()
def canonical(value): return json.dumps(value, sort_keys=True, separators=(',', ':'), allow_nan=False)+'\n'

def images():
    """No reference pixels are passed to measurements or policy."""
    photos = r3.read(r3.BASE/'sources.json')['photos']
    for p in photos:
        yield {'id':p['id'], 'base':p['id'], 'domain':p['domain'], 'kind':'natural'}, r3.decode(r3.BASE/(p['id']+'.png'))
    recipes = r3.read(BASE/'recipes.json')['derivatives']
    for p in recipes:
        photo = next(v for v in photos if v['id']==p['base'])
        rgb = r3.decode(r3.BASE/(p['base']+'.png'))
        linear = [tuple(r3.q.linear(c) for c in rgb[i:i+3]) for i in range(0,len(rgb),3)]
        rgb = r3.q.pixels8(r3.transform(linear,p['tool'],p['amount']))
        yield {'id':p['id'], 'base':p['base'], 'domain':photo['domain'], 'kind':'reviewed_derivative'}, rgb

def measurements(rgb):
    if not isinstance(rgb, bytes) or len(rgb)!=224*224*3:
        raise ValueError('fixed 224 RGB8 engineering input required')
    ps=[tuple(r3.q.linear(c) for c in rgb[i:i+3]) for i in range(0,len(rgb),3)]
    y=[r3.q.luma(p) for p in ps]; ordered=sorted(y)
    pct=lambda f: ordered[int((len(ordered)-1)*f)]
    edges=[abs(y[i]-y[i-1]) for i in range(1,len(y)) if i%224]
    return {'p10':pct(.1),'p50':pct(.5),'p90':pct(.9),'p95':pct(.95),'p99':pct(.99),
            'mean':statistics.fmean(y),'std':statistics.pstdev(y),
            'black':sum(v<.01 for v in y)/len(y),'white':sum(v>.98 for v in y)/len(y),
            'saturation':statistics.fmean((max(p)-min(p))/max(p) if max(p)>0 else 0 for p in ps),
            'red_blue':statistics.fmean(p[0]-p[2] for p in ps),
            'edge_mean':statistics.fmean(edges)}

def policy(m, cfg):
    if cfg.get('version')!=1 or cfg.get('scope')!='OFFLINE_HEURISTIC_FEASIBILITY_NOT_PRODUCTION':
        raise ValueError('unknown policy')
    if set(m)!=set(cfg['measurement_fields']) or any(not isinstance(v,(int,float)) or isinstance(v,bool) or not math.isfinite(v) for v in m.values()):
        raise ValueError('malformed measurements')
    if any(m[k]<0 or m[k]>1 for k in m if k!='red_blue') or not -1<=m['red_blue']<=1:
        raise ValueError('out of range measurements')
    actions=[]
    # Fixed feasibility heuristics. Confidence is an evidence tier, NEVER probability.
    if m['p95']<cfg['dark_p95'] and m['p50']<cfg['dark_p50'] and m['p99']>cfg['dark_signal_floor']:
        actions.append({'tool':'adjust.exposure','value':500,'direction':1,'confidence':'HEURISTIC_LOW','reason':'globally_dark_with_some_signal'})
    elif m['p10']>cfg['bright_p10']:
        actions.append({'tool':'adjust.exposure','value':-500,'direction':-1,'confidence':'HEURISTIC_LOW','reason':'globally_bright'})
    elif m['white']>cfg['white_fraction'] and m['p50']>cfg['white_p50']:
        actions.append({'tool':'adjust.highlights','value':-300,'direction':-1,'confidence':'HEURISTIC_LOW','reason':'widespread_near_white'})
    elif m['black']>cfg['shadow_fraction'] and m['p95']>cfg['shadow_p95'] and m['mean']<cfg['shadow_mean']:
        actions.append({'tool':'adjust.shadows','value':300,'direction':1,'confidence':'HEURISTIC_LOW','reason':'deep_shadows_with_bright_region'})
    registry=r3.read(ROOT/'tests/fixtures/agent-tools-v1.json')
    rules=r3.read(r3.BASE/'config.json')['rules']
    for action in actions:
        r3.q.validate_value(action['value'],next(r for r in rules if r['tool']==action['tool']),registry)
    return {'recommendations':actions,'abstain_tools':[t for t in TOOLS if t not in [a['tool'] for a in actions]],
            'confidence_semantics':'uncalibrated heuristic evidence; not a probability or approval',
            'production_admitted':False}

def evaluate(labels, cfg):
    records=[]
    for image,rgb in images():
        start=time.perf_counter(); m=measurements(rgb); result=policy(m,cfg); elapsed=time.perf_counter()-start
        records.append({**image,'rgb_sha256':digest(rgb),'measurements':m,'result':result,'seconds':elapsed})
    return {'version':1,'scope':'OFFLINE_ONLY_NOT_ADMISSION','production_admitted':False,
            'labels_sha256':digest((BASE/'labels.json').read_bytes()),
            'policy_sha256':digest((BASE/'policy.json').read_bytes()),
            'records':records,'scores':score(records,labels),
            'natural_scores':score([r for r in records if r['kind']=='natural'],labels,subset=True),
            'derivative_scores':score([r for r in records if r['kind']!='natural'],labels,subset=True)}

def score(records, labels, subset=False):
    targets={i['id']:i for i in labels['images']}; scores={}
    if not ({r['id'] for r in records}<=set(targets) if subset else set(targets)=={r['id'] for r in records}): raise ValueError('label/image mismatch')
    for tool in TOOLS:
        s={k:0 for k in ['positive','negative','acceptable','ambiguous','correct_direction','missed','wrong_direction','false_positive','abstained','excluded_emissions','invalid']}
        domains={}
        for r in records:
            label=targets[r['id']]['labels'][tool]; cls=label['class']
            if cls not in ('positive','negative','acceptable','ambiguous'):raise ValueError('unknown label')
            s[cls]+=1; action=next((a for a in r['result']['recommendations'] if a['tool']==tool),None)
            d=domains.setdefault(r['domain'],{'scored':0,'false_positive':0,'correct_direction':0,'missed':0})
            if cls=='ambiguous': s['excluded_emissions']+=int(action is not None);continue
            d['scored']+=1
            if not action:s['abstained']+=1
            if cls in ('positive','negative'):
                if not action:s['missed']+=1;d['missed']+=1
                elif action['direction']==(1 if cls=='positive' else -1):s['correct_direction']+=1;d['correct_direction']+=1
                else:s['wrong_direction']+=1
            elif action:s['false_positive']+=1;d['false_positive']+=1
        s['domains']=domains;scores[tool]=s
    return scores

def main():
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    if 'build' not in args.output.resolve().relative_to(ROOT).parts:raise ValueError('write only ignored build evidence')
    freeze=r3.read(BASE/'review-freeze.json')
    for path,expected in freeze['bindings'].items():
        if digest((ROOT/path).read_bytes())!=expected:raise ValueError('frozen input mismatch: '+path)
    marker=args.output.with_suffix('.start.json')
    with marker.open('x',encoding='utf-8') as stream:
        stream.write(canonical({'started_utc':datetime.now(timezone.utc).isoformat(),'freeze_sha256':digest((BASE/'review-freeze.json').read_bytes()),'scope':'OFFLINE_ONLY_NOT_ADMISSION'}))
    result=evaluate(r3.read(BASE/'labels.json'),r3.read(BASE/'policy.json'))
    args.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(json.dumps(result['scores'],indent=2))
if __name__=='__main__':main()
