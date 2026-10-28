import importlib.util
import sys
sys.dont_write_bytecode=True
import tempfile
import unittest
import zipfile
from pathlib import Path

spec=importlib.util.spec_from_file_location('scanner',Path(__file__).parents[1]/'scripts/check-android-release.py')
scanner=importlib.util.module_from_spec(spec)
spec.loader.exec_module(scanner)

class ReleaseScanner(unittest.TestCase):
    def test_packaged_surfaces(self):
        root=Path(__file__).parents[1]/'build'
        root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=root) as directory:
            apk=Path(directory)/'release.apk'
            def write(extra=None):
                with zipfile.ZipFile(apk,'w',zipfile.ZIP_DEFLATED) as z:
                    z.writestr('classes.dex',b'dex\n035\0owned release fixture')
                    z.writestr('lib/arm64-v8a/libpixaura_core.so',b'ELF owned fixture')
                    if extra: z.writestr(*extra)
            write();self.assertEqual(scanner.scan(apk)['dex_files'],1)
            for marker in (b'ReviewPreviewTestHooks', b'reviewPreviewTestControl', b'review-preview-test-pause', b'review-preview-test-failure'):
                for member in ('classes2.dex', 'lib/arm64-v8a/test.so', 'resources.arsc'):
                    write((member, marker))
                    with self.assertRaises(ValueError): scanner.scan(apk)
            for name in ('nima.hdf5','ffcc.npz','mobile.pth','weights.pb'):
                write(('res/'+name,b'unadmitted artifact'))
                with self.assertRaises(ValueError):scanner.scan(apk)
            write(('assets/dexopt/baseline.prof',b'pro\x00010\x00'));self.assertEqual(scanner.scan(apk)['dex_files'],1)
            for extra in [('assets/test.txt',b'privileged-test-provider'),('classes2.dex',b'AgentPlatformHooks'),('lib/armeabi-v7a/test.so',b'pixaura_test_agent'),('resources.arsc','auto-approval-hook'.encode('utf-16le')),('assets/candidate.litertlm',b'not admitted'),('assets/dexopt/baseline.prof',b'fake model'),('res/raw/unknown.bin',b'not reviewed'),('../escape',b'x')]:
                write(extra)
                with self.assertRaises(ValueError):scanner.scan(apk)

if __name__=='__main__': unittest.main()
