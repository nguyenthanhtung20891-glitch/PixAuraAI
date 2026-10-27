#!/usr/bin/env python3
"""Inspect the actual unsigned release app; no extraction or executable loading."""
import importlib.util
from pathlib import Path
import sys
sys.dont_write_bytecode=True
spec=importlib.util.spec_from_file_location('apk',Path(__file__).with_name('check-android-release.py'))
apk=importlib.util.module_from_spec(spec);spec.loader.exec_module(apk)

def scan(app):
    app=Path(app)
    if app.is_symlink() or not app.is_dir():raise ValueError('regular app directory required')
    count=total=0
    for path in sorted(app.rglob('*')):
        if path.is_symlink():raise ValueError('unreviewed app symlink')
        if path.is_dir():continue
        if not path.is_file():raise ValueError('regular app file required')
        relative=path.relative_to(app).as_posix()
        if path.suffix.lower() in apk.MODEL_SUFFIXES or any(p in ('benchmarks','absolute-images-v1','natural-images-v1') for p in path.parts):
            raise ValueError('unadmitted model/research asset')
        size=path.stat().st_size;total+=size;count+=1
        if size>128*1024*1024 or total>512*1024*1024:raise ValueError('release app scan bounds')
        data=path.read_bytes()
        for marker in apk.MARKERS:
            if marker in data or (marker.isascii() and marker.decode('ascii').encode('utf-16le') in data):
                raise ValueError('forbidden release marker in '+relative)
    if not (app/'Info.plist').is_file() or not (app/'PixAuraAI').is_file():raise ValueError('missing app identity/executable')
    return {'files':count,'bytes':total,'model_assets':0}

if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('one final release .app required')
    print('Apple final release isolation PASS',scan(sys.argv[1]))
