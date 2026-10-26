"""Model-free regression for frozen offline Round 3 evidence."""
import importlib.util
from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
loader=importlib.util.spec_from_file_location('round3',ROOT/'scripts/qualify-natural-images.py')
m=importlib.util.module_from_spec(loader)
loader.loader.exec_module(m)

class NaturalImages(unittest.TestCase):
    def test_rights_identity_and_no_split_leakage(self):
        sources=m.read(m.BASE/'sources.json')['photos']
        self.assertEqual(len(sources),32)
        self.assertEqual(len({p['original_sha256'] for p in sources}),32)
        self.assertEqual(len({p['rgb_sha256'] for p in sources}),32)
        self.assertEqual({p['split'] for p in sources},{'development','holdout'})
        for domain in {p['domain'] for p in sources}:
            for split in ('development','holdout'):
                self.assertEqual(sum(p['domain']==domain and p['split']==split for p in sources),2)
        for p in sources:
            self.assertEqual(p['license'],'CC0-1.0')
            self.assertTrue(p['source_page'].startswith('https://wordpress.org/photos/photo/'))
            self.assertEqual(p['license_evidence_url'],p['source_page'])
            self.assertTrue(p['author'])
            self.assertEqual(m.q.file_sha(m.BASE/(p['id']+'.png')),p['normalized_sha256'])
            self.assertEqual(m.q.sha(m.decode(m.BASE/(p['id']+'.png'))),p['rgb_sha256'])

    def test_frozen_bindings_precede_holdout(self):
        frozen=m.read(m.BASE/'frozen-policy.json')
        self.assertEqual(frozen['scope'],'OFFLINE_ONLY_NOT_ADMISSION')
        self.assertTrue(frozen['pre_holdout'])
        for path,digest in frozen['bindings'].items():
            self.assertEqual(m.q.file_sha(ROOT/path),digest)
        dev=m.read(ROOT/'docs/reports/evidence/phase-7-step-7-round3-development.json')
        self.assertEqual(frozen['slopes'],dev['slopes'])
        started=m.read(ROOT/'docs/reports/evidence/phase-7-step-7-round3-holdout-start.json')
        self.assertEqual(started['policy'],m.q.file_sha(m.BASE/'frozen-policy.json'))

    def test_recorded_results_without_model(self):
        cfg=m.read(m.BASE/'config.json');sources=m.read(m.BASE/'sources.json')['photos']
        for split in ('development','holdout'):
            result=m.read(ROOT/('docs/reports/evidence/phase-7-step-7-round3-'+split+'.json'))
            self.assertFalse(result['production_admitted'])
            self.assertEqual(result['config_sha256'],m.q.file_sha(m.BASE/'config.json'))
            self.assertEqual(result['source_sha256'],m.q.file_sha(m.BASE/'sources.json'))
            self.assertEqual(len(result['records']),928)
            allowed={p['id'] for p in sources if p['split']==split}
            self.assertEqual({r['base'] for r in result['records']},allowed)
            self.assertEqual(result['scores'],m.metrics(result['records'],result['slopes'],cfg))
            if split=='holdout':
                self.assertEqual(result['slopes'],m.read(m.BASE/'frozen-policy.json')['slopes'])
                for rule in cfg['rules']:
                    repeats=result['repeat_consistency'][rule['tool']]
                    self.assertEqual(repeats['cases'],116)
                    self.assertEqual(repeats['cases'],repeats['identical_recommendations'])

    def test_fail_closed_and_no_authority(self):
        cfg=m.read(m.BASE/'config.json')
        for rule in cfg['rules']:
            self.assertIsNone(m.prediction(0,rule,cfg))
            for value in (-2,-1,-.6,-.2,.2,.6,1,2):
                m.prediction(value,rule,cfg)  # every emitted native value checked
            with self.assertRaises(ValueError):m.prediction(float('nan'),rule,cfg)
            with self.assertRaises(ValueError):m.prediction(float('inf'),rule,cfg)
        text=(ROOT/'scripts/qualify-natural-images.py').read_text()
        for forbidden in ('urllib','requests.get','pixaura_editor','issue_approval','candidate_execute'):
            self.assertNotIn(forbidden,text)
        self.assertIn("marker.open('x')",text)

    def test_reference_report_uses_native_temperature_direction(self):
        spec=importlib.util.spec_from_file_location('analysis',ROOT/'scripts/analyze-natural-images.py')
        analysis=importlib.util.module_from_spec(spec);spec.loader.exec_module(analysis)
        cfg=m.read(m.BASE/'config.json')
        hold=m.read(ROOT/'docs/reports/evidence/phase-7-step-7-round3-holdout.json')
        derived=m.read(ROOT/'docs/reports/evidence/phase-7-step-7-round3-analysis.json')
        self.assertEqual(derived['corrected_reference'],analysis.corrected_reference(hold,cfg))
        self.assertEqual(derived['nonactionable_false_positives'],analysis.nonactionable_false_positives(hold,cfg))
        self.assertEqual(derived['corrected_reference']['adjust.temperature']['combination_direction_correct'],0)
        self.assertFalse(derived['production_admitted'])

    def test_metric_repeatability_on_real_photo(self):
        cfg=m.read(m.BASE/'config.json');photo=m.read(m.BASE/'sources.json')['photos'][0]
        rgb=m.decode(m.BASE/(photo['id']+'.png'))
        for rule in cfg['rules']:
            self.assertEqual(m.metric(rgb,rule['tool']),m.metric(rgb,rule['tool']))

if __name__=='__main__':unittest.main()
