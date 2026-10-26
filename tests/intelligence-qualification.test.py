"""Offline evidence regression; no model/runtime dependency or network access."""
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import unittest

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
loader = importlib.util.spec_from_file_location('qualification', ROOT/'scripts/qualify-intelligence.py')
q = importlib.util.module_from_spec(loader)
loader.loader.exec_module(q)

def read(path):
    return json.loads((ROOT/path).read_text())

class Qualification(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.spec = read('docs/benchmarks/intelligence-qualification-v1.json')
        cls.cases = q.corpus(cls.spec)
        cls.results = read('docs/reports/evidence/phase-7-step-7-round2-results.json')

    def test_frozen_pixels_and_expected_semantics(self):
        frozen = read('docs/benchmarks/intelligence-qualification-v1-corpus.json')
        pixels = read('docs/benchmarks/intelligence-qualification-v1-pixels.json')
        self.assertEqual(len(self.cases), 96)
        self.assertEqual(len({c['tool'] for c in self.cases}), 8)
        self.assertEqual(frozen['spec_sha256'], q.file_sha(q.SPEC))
        self.assertEqual(frozen['generator_sha256'], q.file_sha(ROOT/'scripts/qualify-intelligence.py'))
        self.assertEqual(self.results['frozen_corpus_sha256'],
                         q.file_sha(ROOT/'docs/benchmarks/intelligence-qualification-v1-corpus.json'))
        self.assertEqual(len(frozen['cases']),len(self.cases))
        for actual, recorded in zip(self.cases, frozen['cases']):
            # PNG compression can differ across zlib versions; decoded RGB cannot.
            self.assertEqual({k: actual[k] for k in ('id','tool','variant','expected')},
                             {k: recorded[k] for k in ('id','tool','variant','expected')})
            self.assertEqual(q.sha(actual['pixels']), pixels[actual['id']])

    def test_native_bounds_and_abstention(self):
        registry = read('tests/fixtures/agent-tools-v1.json')
        for rule in self.spec['rules']:
            for value in (rule['positive'],rule['opposite'],rule['reference']):
                q.validate_value(value,rule,registry)
            self.assertIsNone(q.decide([0.2,0.2,0.2],rule['positive'],rule,self.spec['policy']))
            self.assertIsNone(q.decide([0.1,0.2,0.9],rule['positive'],rule,self.spec['policy']))
            self.assertIsNone(q.decide([0.9,0.2,0.1],None,rule,self.spec['policy']))
            with self.assertRaises(AssertionError):
                q.validate_value(10**30,rule,registry)
            for bad in ([float('nan'),0,0],[float('inf'),0,0],[2,0,0],[0,0]):
                with self.assertRaises(AssertionError):
                    q.decide(bad,rule['positive'],rule,self.spec['policy'])

    def test_recorded_scores_reproduce_summary_without_model(self):
        records = self.results['records']
        self.assertEqual(len(records),len(self.cases))
        values = []
        for case, record in zip(self.cases, records):
            self.assertEqual(case['id'],record['id'])
            rule = next(r for r in self.spec['rules'] if r['tool']==case['tool'])
            value = q.calibration(case['pixels'],self.spec['size'],rule)
            self.assertEqual(value,record['metric_only'])
            actions = [q.decide(s,value,rule,self.spec['policy']) for s in record['scores']]
            self.assertEqual(actions,record['candidate'])
            self.assertEqual(actions[0],actions[1])
            values.append(actions[0])
        self.assertEqual(q.summarize(self.cases,values,self.spec),self.results['candidate'])
        self.assertFalse(self.results['production_admitted'])
        self.assertEqual(self.results['natural_photo_status'],'INSUFFICIENT REALISTIC QUALITY EVIDENCE')

    def test_provenance_does_not_promote_missing_conversion(self):
        provenance = read('docs/reports/evidence/phase-7-step-7-round2-provenance.json')
        self.assertEqual(len(provenance['matrix']),20)
        self.assertEqual(provenance['current_artifact_verdict'],'CURRENT_ARTIFACT_PROVENANCE_FAILED')
        self.assertFalse(provenance['production_admitted'])
        self.assertTrue(any(r['status']=='UNPROVEN' for r in provenance['matrix']))

if __name__=='__main__':
    unittest.main()
