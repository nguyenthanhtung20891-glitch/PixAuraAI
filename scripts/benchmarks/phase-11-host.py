#!/usr/bin/env python3
"""Reproducible synthetic host CPU observations; no mobile admission authority."""
import hashlib
import json
import pathlib
import statistics
import math
import struct
import subprocess
import tempfile
import zlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/phase-11/host'
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
         '-Ipackages/core/include', 'scripts/benchmarks/phase-11-host.cpp', '-L' + str(OUT / 'release'), '-lpixaura_core',
         '-Wl,-rpath,' + str(OUT / 'release'), '-o', str(OUT / 'phase-11-host')],
    ]:
        subprocess.run(args, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)

result = {
    'version': 1, 'source_sha': run('git', 'rev-parse', 'HEAD'),
    'scope': 'macOS host CPU only; 100 complete lifetimes after 5 warmups; not mobile G8 qualification',
    'date_utc': run('date', '-u', '+%Y-%m-%dT%H:%M:%SZ'),
    'soc': run('sysctl', '-n', 'machdep.cpu.brand_string'), 'os': run('sw_vers'),
    'compiler': run('xcrun', 'clang++', '--version'), 'configuration': 'Release, O3 core; O2 harness; strict FP', 'percentile_method': 'nearest rank ceil(0.95*n)-1; p50 median; individual previews flattened from four per lifetime; live RSS from task_info after destruction; peak RSS cumulative Darwin bytes',
    'warmups': 5, 'measured_samples': 100, 'cases': [],
}
for width, height in [(1024, 768), (3000, 2000)]:
    data = synthetic(width, height)
    image = OUT / f'synthetic-{width}x{height}.png'
    image.write_bytes(data)
    project = pathlib.Path(tempfile.mkdtemp(prefix=f'projects-{width}x{height}-', dir=OUT))
    raw = run(str(OUT / 'phase-11-host'), str(image), str(project))
    (OUT / f'{width}x{height}.jsonl').write_text(raw + '\n')
    samples = [json.loads(line) for line in raw.splitlines()]
    measured = [sample for sample in samples if not sample['warmup']]
    summary = {}
    for key in measured[0]:
        if (key.endswith('_us') or key in ['peak_rss_bytes', 'live_rss_bytes']) and isinstance(measured[0][key], (int, float)):
            values = sorted(sample[key] for sample in measured)
            summary[key] = {'min': min(values), 'p50': statistics.median(values), 'p95_nearest_rank': values[math.ceil(0.95 * len(values)) - 1], 'max': max(values), 'samples': len(values)}
    previews = sorted(v for sample in measured for v in sample['preview_each_us'])
    summary['preview_individual_us'] = {'min': min(previews), 'p50': statistics.median(previews), 'p95_nearest_rank': previews[math.ceil(0.95 * len(previews)) - 1], 'max': max(previews), 'samples': len(previews)}
    result['cases'].append({'width': width, 'height': height, 'fixture_sha256': hashlib.sha256(data).hexdigest(),
                            'input_bytes': len(data), 'samples': samples, 'summary': summary})
result['native_source_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for root in [ROOT / 'packages/core/src', ROOT / 'packages/core/include'] for p in sorted(root.rglob('*')) if p.is_file()}
result['native_source_sha256']['CMakeLists.txt'] = hashlib.sha256((ROOT / 'CMakeLists.txt').read_bytes()).hexdigest()
result['working_tree_status_at_measurement'] = run('git', 'status', '--porcelain')
result['harness_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in [ROOT / 'scripts/benchmarks/phase-11-host.cpp', pathlib.Path(__file__).resolve()]}
(OUT / 'measurements.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps({'result': str(OUT / 'measurements.json'), 'cases': [{k: v for k, v in c.items() if k != 'samples'} for c in result['cases']]}, indent=2))
