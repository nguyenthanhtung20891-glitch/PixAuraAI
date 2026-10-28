#!/usr/bin/env python3
"""Offline final APK isolation. Standard library only; no unpacked executables."""
import sys, zipfile
from pathlib import PurePosixPath

MARKERS = (
    b'ReviewPreviewTestHooks', b'reviewPreviewTestControl', b'review-preview-test-pause', b'review-preview-test-failure',
    b'pixaura_test_', b'assistance_checkpoint', b'agent_checkpoint_state',
    b'AgentPlatformHooks', b'AssistTestHooks', b'AutoTestHooks',
    b'review-test-pause', b'review-test-failure', b'assist-test-pause',
    b'assist-test-failure', b'auto-test-pause', b'auto-test-failure',
    b'AdmissionTestExecution', b'admission_test_', b'PXSEG',
    b'synthetic-segmentation', b'fake-human-apply', b'auto-approval-hook',
    b'privileged-test-provider', b'hidden-plan-executor', b'direct-commit-hook',
    b'embeddinggemma', b'EmbeddingGemma',
    b'weights_mobilenet', b'FFCCModel', b'MobileIQA',
    b'OFFLINE_HEURISTIC_FEASIBILITY_NOT_PRODUCTION',
    b'OFFLINE_PAIRED_REFERENCE_NOT_ABSOLUTE_CORRECTION',
    b'Assistance admission v1', b'Assistance admission allocations',
    b'pixaura_model_test_boundary', b'pixaura_inference_test_boundary',
    b'pixaura_inference_android_test_boundary', b'pixaura_inference_apple_test_boundary',
    b'production_selection', b'certification_output_fault', b'cpu_execution_calls',
    b'PXSEC', b'pixaura_agent_schema_test_', b'agent_tools_golden', b'pixaura.test.only',
    b'reference_planner_fixtures', b'platform_checkpoint', b'assistance.allocation',
    b'assist_test_fault', b'auto_test_fault',
    bytes.fromhex('d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a'),
)
MODEL_SUFFIXES = ('.litertlm', '.tflite', '.onnx', '.safetensors', '.ckpt', '.pt', '.pth', '.hdf5', '.h5', '.npz', '.mat', '.pb')

def scan(path):
    natives = dex = 0
    total = 0
    with zipfile.ZipFile(path) as apk:
        names = set()
        for entry in apk.infolist():
            name = entry.filename
            if name in names or '\\' in name or name.startswith('/') or '..' in PurePosixPath(name).parts:
                raise ValueError('ambiguous APK path')
            names.add(name)
            if entry.flag_bits & 1 or entry.file_size > 128*1024*1024:
                raise ValueError('unsupported encrypted/oversized APK member')
            total += entry.file_size
            if total > 512*1024*1024:
                raise ValueError('APK scan bound exceeded')
            if name.lower().endswith(MODEL_SUFFIXES):
                raise ValueError('unadmitted model asset')
            if name.startswith('res/raw/'):
                raise ValueError('unreviewed raw resource')
            profiles = {'assets/dexopt/baseline.prof': b'pro\0', 'assets/dexopt/baseline.profm': b'prm\0'}
            if name.startswith('assets/') and not entry.is_dir() and name not in {'assets/ThirdPartyNotices.txt', *profiles}:
                raise ValueError('unreviewed release asset')
            if entry.is_dir():
                continue
            data = apk.read(entry)
            if name in profiles and not data.startswith(profiles[name]):
                raise ValueError('invalid Android baseline profile')
            for marker in MARKERS:
                if marker in data or (marker.isascii() and marker.decode('ascii').encode('utf-16le') in data):
                    raise ValueError('forbidden release marker in '+name)
            natives += name.startswith('lib/') and name.endswith('.so')
            dex += name.endswith('.dex')
    if not natives or not dex:
        raise ValueError('release APK missing native/DEX evidence')
    return {'native_libraries': natives, 'dex_files': dex, 'uncompressed_bytes': total}

if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('one final release APK required')
    print('Android final release isolation PASS', scan(sys.argv[1]))
