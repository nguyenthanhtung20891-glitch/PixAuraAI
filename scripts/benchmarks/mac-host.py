#!/usr/bin/env python3
"""Reproducible synthetic host CPU observations; no mobile admission authority."""
import hashlib
import json
import pathlib
import statistics
import struct
import subprocess
import tempfile
import zlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/phase-10.1-host'
OUT.mkdir(parents=True, exist_ok=True)

def run(*args):
    return subprocess.check_output(args, cwd=ROOT, text=True).strip()

def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

def synthetic(width, height):
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            rows.extend(((x * 7 + y) % 256, (y * 11 + x) % 256, (x ^ y) % 256))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(rows, 6)) + chunk(b'IEND', b''))

with (OUT / 'build.log').open('w') as log:
    for args in [
        ['cmake', '-S', '.', '-B', str(OUT / 'release'), '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF',
         '-DCMAKE_C_COMPILER=' + run('xcrun', '--find', 'clang'), '-DCMAKE_CXX_COMPILER=' + run('xcrun', '--find', 'clang++')],
        ['cmake', '--build', str(OUT / 'release'), '--target', 'pixaura_core', '--parallel', '4'],
        ['xcrun', 'clang++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', '-fno-fast-math', '-ffp-contract=off',
         '-Ipackages/core/include', 'scripts/benchmarks/mac-host.cpp', '-L' + str(OUT / 'release'), '-lpixaura_core',
         '-Wl,-rpath,' + str(OUT / 'release'), '-o', str(OUT / 'mac-host')],
    ]:
        subprocess.run(args, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)

result = {
    'version': 1, 'source_sha': run('git', 'rev-parse', 'HEAD'),
    'scope': 'macOS host CPU; engineering harness working-tree digests recorded separately',
    'date_utc': run('date', '-u', '+%Y-%m-%dT%H:%M:%SZ'),
    'soc': run('sysctl', '-n', 'machdep.cpu.brand_string'), 'os': run('sw_vers'),
    'compiler': run('xcrun', 'clang++', '--version'), 'configuration': 'Release, O3 core; O2 harness; strict FP',
    'warmups': 2, 'measured_samples': 7, 'cases': [],
}
for width, height in [(1024, 768), (3000, 2000)]:
    data = synthetic(width, height)
    image = OUT / f'synthetic-{width}x{height}.png'
    image.write_bytes(data)
    project = pathlib.Path(tempfile.mkdtemp(prefix=f'projects-{width}x{height}-', dir=OUT))
    raw = run(str(OUT / 'mac-host'), str(image), str(project))
    (OUT / f'{width}x{height}.jsonl').write_text(raw + '\n')
    samples = [json.loads(line) for line in raw.splitlines()]
    measured = [sample for sample in samples if not sample['warmup']]
    summary = {}
    for key in measured[0]:
        if key.endswith('_us') or key == 'peak_rss_bytes':
            values = sorted(sample[key] for sample in measured)
            summary[key] = {'min': min(values), 'median': statistics.median(values), 'max': max(values)}
    result['cases'].append({'width': width, 'height': height, 'fixture_sha256': hashlib.sha256(data).hexdigest(),
                            'input_bytes': len(data), 'samples': samples, 'summary': summary})
result['harness_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in [ROOT / 'scripts/benchmarks/mac-host.cpp', pathlib.Path(__file__).resolve()]}
(OUT / 'measurements.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps({'result': str(OUT / 'measurements.json'), 'cases': [{k: v for k, v in c.items() if k != 'samples'} for c in result['cases']]}, indent=2))
