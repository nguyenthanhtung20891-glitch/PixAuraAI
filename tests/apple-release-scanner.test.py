import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
sys.dont_write_bytecode=True
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('apple',root/'scripts/check-apple-release.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class AppleRelease(unittest.TestCase):
    def test_final_app_and_negative_assets(self):
        with tempfile.TemporaryDirectory(dir=root/'build') as folder:
            app=Path(folder)/'PixAuraAI.app';app.mkdir()
            (app/'Info.plist').write_bytes(b'owned fixture');(app/'PixAuraAI').write_bytes(b'owned executable fixture')
            self.assertEqual(m.scan(app)['files'],2)
            for name,data in [('nima.hdf5',b'x'),('ffcc.npz',b'x'),('mobile.pth',b'x'),('weights.pb',b'x'),('research.txt',b'OFFLINE_HEURISTIC_FEASIBILITY_NOT_PRODUCTION'),('bypass.txt','fake-human-apply'.encode('utf-16le')),('hook.bin',b'pixaura_test_agent')]:
                p=app/name;p.write_bytes(data)
                with self.assertRaises(ValueError):m.scan(app)
                p.unlink()
            (app/'PixAuraAI').unlink()
            with self.assertRaises(ValueError):m.scan(app)
if __name__=='__main__':unittest.main()
