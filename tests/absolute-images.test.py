"""Round 4 model-free evidence, abstention and regression checks."""
import copy
import importlib.util
import json
from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
loader=importlib.util.spec_from_file_location('absolute',ROOT/'scripts/assess-absolute-images.py')
m=importlib.util.module_from_spec(loader);loader.loader.exec_module(m)

class AbsoluteImages(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.labels=m.r3.read(m.BASE/'labels.json')
        cls.cfg=m.r3.read(m.BASE/'policy.json')
        cls.recorded=m.r3.read(ROOT/'docs/reports/evidence/phase-7-step-7-round4-absolute.json')

    def test_frozen_review_precedes_scoring(self):
        freeze=m.r3.read(m.BASE/'review-freeze.json')
        self.assertTrue(freeze['labels_frozen_before_measurement'])
        for path,digest in freeze['bindings'].items():self.assertEqual(m.digest((ROOT/path).read_bytes()),digest,path)
        marker=m.r3.read(ROOT/'docs/reports/evidence/phase-7-step-7-round4-start.json')
        self.assertEqual(marker['freeze_sha256'],m.digest((m.BASE/'review-freeze.json').read_bytes()))
        self.assertEqual(self.recorded['labels_sha256'],m.digest((m.BASE/'labels.json').read_bytes()))
        self.assertEqual(self.recorded['policy_sha256'],m.digest((m.BASE/'policy.json').read_bytes()))

    def test_corpus_and_review_limitations(self):
        images=self.labels['images']
        self.assertEqual(len(images),48)
        self.assertEqual(len({i['id'] for i in images}),48)
        self.assertEqual(sum(i['kind']=='natural' for i in images),32)
        self.assertEqual(len({i['domain'] for i in images}),8)
        self.assertIn('NOT_HUMAN_GROUND_TRUTH',self.labels['status'])
        self.assertFalse(self.labels['parameter_goldens'])
        for i in images:
            self.assertEqual(set(i['labels']),set(m.TOOLS))
            for label in i['labels'].values():
                self.assertIn(label['class'],('positive','negative','acceptable','ambiguous'))
                self.assertIn(label['confidence'],('low','medium'))
                self.assertGreater(len(label['rationale']),30)

    def test_reproduce_all_images_without_paired_reference_or_model(self):
        before={p:m.digest(p.read_bytes()) for p in m.r3.BASE.glob('*.png')}
        labels={i['id']:i for i in self.labels['images']}
        for meta,rgb in m.images():
            r=next(r for r in self.recorded['records'] if r['id']==meta['id'])
            self.assertEqual(m.digest(rgb),labels[meta['id']]['rgb_sha256'])
            self.assertEqual(m.measurements(rgb),r['measurements'])
            self.assertEqual(m.policy(r['measurements'],self.cfg),r['result'])
            self.assertLessEqual(len(r['result']['recommendations']),1)
            self.assertFalse(r['result']['production_admitted'])
        self.assertEqual(before,{p:m.digest(p.read_bytes()) for p in before})

    def test_scores_include_ambiguous_exclusion_and_strata(self):
        for name,kind in [('scores',None),('natural_scores','natural'),('derivative_scores','reviewed_derivative')]:
            records=[r for r in self.recorded['records'] if kind is None or r['kind']==kind]
            self.assertEqual(self.recorded[name],m.score(records,self.labels,subset=kind is not None))
        bad=copy.deepcopy(self.labels);bad['images'][0]['labels'][m.TOOLS[0]]['class']='unknown'
        with self.assertRaises(ValueError):m.score(self.recorded['records'],bad)

    def test_invalid_measurements_and_unknown_policy_fail_closed(self):
        v=dict(self.recorded['records'][0]['measurements'])
        for invalid in (float('nan'),float('inf'),-1,2,True,'0'):
            bad={**v,'p50':invalid}
            with self.assertRaises(ValueError):m.policy(bad,self.cfg)
        with self.assertRaises(ValueError):m.policy({**v,'unexpected':0},self.cfg)
        with self.assertRaises(ValueError):m.policy(v,{**self.cfg,'version':2})
        for rgb in (b'',b'x'*100,'not pixels',bytearray(224*224*3)):
            with self.assertRaises(ValueError):m.measurements(rgb)

    def test_intent_ambiguous_scenes_abstain_without_histogram_normalization(self):
        for photo in ('39','223','318','405','136','413','419','510','218'):
            r=next(r for r in self.recorded['records'] if r['id']==photo)
            self.assertEqual(r['result']['recommendations'],[],photo)
        # No RGB balance or edge statistic is silently turned into a Kelvin/sharpen edit.
        for r in self.recorded['records']:
            self.assertTrue(all(a['tool'] not in self.cfg['unsupported_decisions'] for a in r['result']['recommendations']))

    def test_shortlist_not_admitted_and_no_fabricated_probe(self):
        landscape=m.r3.read(m.BASE/'landscape.json')
        shortlist=[c for c in landscape['candidates'] if c['shortlisted']]
        self.assertEqual(len(shortlist),3)
        for c in landscape['candidates']:
            self.assertFalse(c['production_admitted'])
            self.assertFalse(c['learned_inference_executed'])
            self.assertFalse(c['complete_provenance'])
            self.assertEqual(c['pix_aura_quality'],'NOT_VERIFIED')
        artifact=landscape['nima_original_artifact']
        self.assertEqual(artifact['bytes'],13174216)
        self.assertEqual(artifact['sha256'],'78c8bb95098fd27695de4147465f4fd504d0ca5e11699a487534854b5aeb3036')
        self.assertFalse(artifact['load_or_inference_executed'])

    def test_authority_remains_blocked(self):
        report=(ROOT/'docs/reports/phase-7-step-7.md').read_text(encoding='utf-8')
        current=report.split('## Round 4')[1].split('## Round 3')[0]
        for term in ('NEW_CANDIDATE_SEARCH_REQUIRED','DISQUALIFIED FOR THE CURRENT PIXAURAAI MVP','Production model UNRESOLVED','Step 8 NOT STARTED','Phase 8 NOT STARTED','ADMISSION BLOCKED'):
            self.assertIn(term,current)

if __name__=='__main__':unittest.main()
