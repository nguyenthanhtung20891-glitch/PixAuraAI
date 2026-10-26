#!/usr/bin/env python3
"""Offline reporting only; never invokes the candidate runtime or editing APIs."""
import argparse
import copy
import importlib.util
import math
from pathlib import Path
import platform
import sys
import tracemalloc

sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('round3',ROOT/'scripts/qualify-natural-images.py')
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)

def corrected_reference(result,config):
    # The raw runner used positive direction for every explicit request. Temperature
    # reference=7000 is actually opposite to its positive correction=6000. Preserve
    # the raw record, correct only this independently derived presentation summary.
    reference=copy.deepcopy(result['scores']['methods']['reference'])
    for rule in config['rules']:
        direction=1 if (rule['reference']-rule['neutral'])*(rule['positive']-rule['neutral'])>0 else -1
        rows=[r for r in result['records'] if r['tool']==rule['tool']]
        nonzero=[r for r in rows if r['variant']=='level' and r['amount']!=0]
        def correct(r):return direction==(1 if r['amount']>0 else -1 if r['amount']<0 else 0)
        summary=reference[rule['tool']]
        summary['direction_correct']=sum(correct(r) for r in nonzero)
        summary['weak_correct']=sum(correct(r) for r in nonzero if abs(r['amount'])==.2)
        summary['strong_correct']=sum(correct(r) for r in nonzero if abs(r['amount'])==1)
        summary['combination_direction_correct']=sum(correct(r) for r in rows if r['variant']=='combination')
        summary['domains']={domain:{'correct':sum(correct(r) for r in nonzero if r['domain']==domain),'total':sum(r['domain']==domain for r in nonzero)} for domain in summary['domains']}
    return reference

def audit_pixels(config,result):
    unchanged=[];effective={r['tool']:{'requested_nonzero':0,'byte_observable':0,'metric_correct':0,'candidate_correct':0} for r in config['rules']}
    records={r['id']:r for r in result['records']}
    for c in m.cases('holdout',config):
        if c['variant']!='level' or c['amount']==0:continue
        counts=effective[c['tool']];counts['requested_nonzero']+=1
        if c['pixels']==c['reference']:unchanged.append(c['id']);continue
        counts['byte_observable']+=1;r=records[c['id']];target=1 if r['amount']>0 else -1
        for method in ('metric','candidate'):
            value=r[method]*result['slopes'][method][r['tool']]
            direction=0 if abs(value)<config['abstention'] else 1 if value>0 else -1
            counts[method+'_correct']+=direction==target
    return {'criterion':'exact decoded RGB byte equality; supplementary only, no records/labels/policies removed or changed','unchanged_nonzero_case_ids':unchanged,'effective_direction':effective}

def memory_probe(config):
    photo=m.read(m.BASE/'sources.json')['photos'][0];rgb=m.decode(m.BASE/(photo['id']+'.png'));peaks={}
    for rule in config['rules']:
        tracemalloc.start();value=m.metric(rgb,rule['tool']);current,peak=tracemalloc.get_traced_memory();tracemalloc.stop()
        assert math.isfinite(value);peaks[rule['tool']]=peak
    return {'measurement':'separate tracemalloc peak Python allocations for one 224x224 metric call, input RGB allocated before tracing; not native/process/device accounting','input_rgb_bytes':len(rgb),'maximum_live_pixel_records':224*224,'peak_python_bytes_by_tool':peaks}

def nonactionable_false_positives(result,config):
    values={}
    for method in ('reference','metric','candidate'):
        values[method]={}
        for rule in config['rules']:
            rows=[r for r in result['records'] if r['tool']==rule['tool'] and r['variant']=='level' and
                  (r['amount']==0 or (r['amount']<0 and rule['opposite'] is None))]
            emitted=sum((rule['reference'] if method=='reference' else m.prediction(r[method]*result['slopes'][method][rule['tool']],rule,config)) is not None for r in rows)
            values[method][rule['tool']]={'false_positives':emitted,'nonactionable_cases':len(rows)}
    return values

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--output',required=True);args=parser.parse_args()
    config=m.read(m.BASE/'config.json');hold=m.read(ROOT/'docs/reports/evidence/phase-7-step-7-round3-holdout.json')
    out=Path(args.output).resolve();assert out.is_relative_to(ROOT)
    result={'version':1,'scope':'OFFLINE_REPORTING_NOT_ADMISSION','reference_direction_erratum':'Raw reference temperature combination count assumed positive direction; actual explicit 7000 K has opposite direction to positive 6000 K. Balanced regular direction counts are unchanged. B/C policies, raw evidence, labels, slopes and thresholds unchanged; no model rerun.',
      'corrected_reference':corrected_reference(hold,config),'nonactionable_false_positives':nonactionable_false_positives(hold,config),'pixel_observability':audit_pixels(config,hold),'metric_memory':memory_probe(config),
      'environment':{'platform':platform.platform(),'python':platform.python_version(),'runtime':'existing LiteRT-LM API 0.18.0','backend':'CPU one thread text and vision, audio disabled',
                     'cpu_model':next(line.split(':',1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
                     'vm_memory_observation':Path('/proc/meminfo').read_text().splitlines()[0]+' (WSL-visible, not physical mobile RAM)'},
      'artifact_bytes':(ROOT/'build/phase7-step7-audit/embeddinggemma-2-text-vision-440m.litertlm').stat().st_size,'production_admitted':False}
    m.write(out,result)
