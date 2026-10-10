#!/usr/bin/env python3
"""Independent Decimal(70) oracle; development-only, no production dependency.

Generate/check immutable pixel fixtures and exact-rational matrix inverses.
Does not call/read the C++ evaluator. Parameters are integers; fixture pixels
are binary32 bit patterns. Per-operation oracle rounds to binary32 once.
"""
from decimal import Decimal as D, getcontext
from pathlib import Path
import argparse
import struct

getcontext().prec = 70
ROOT = Path(__file__).resolve().parent.parent


def inverse(m):
    rows = [list(row) + [D(i == j) for j in range(3)] for i, row in enumerate(m)]
    for i in range(3):
        pivot = rows[i][i]
        rows[i] = [v / pivot for v in rows[i]]
        for j in range(3):
            if j != i:
                factor = rows[j][i]
                rows[j] = [a - factor*b for a, b in zip(rows[j], rows[i])]
    return [row[3:] for row in rows]


def dot(m, v):
    return [sum(a*b for a, b in zip(row, v)) for row in m]


M = [[D(506752)/1228815, D(87881)/245763, D(12673)/70218],
     [D(87098)/409605, D(175762)/245763, D(12673)/175545],
     [D(7918)/409605, D(87881)/737289, D(1001167)/1053270]]
B = [[D(x) for x in row] for row in
     [('0.8951', '0.2664', '-0.1614'),
      ('-0.7502', '1.7135', '0.0367'), ('0.0389', '-0.0685', '1.0296')]]
MI, BI = inverse(M), inverse(B)


def white(t):
    t = D(t)
    if t <= 7000:
        x = D('0.244063') + D('99.11')/t + D(2967800)/t**2 - D(4607000000)/t**3
    else:
        x = D('0.237040') + D('247.48')/t + D(1901800)/t**2 - D(2006400000)/t**3
    y = -3*x*x + D('2.87')*x - D('0.275')
    return [x/y, D(1), (1-x-y)/y]


def bits(x):
    return struct.unpack('<I', struct.pack('<f', float(x)))[0]


def value(b):
    return D.from_float(struct.unpack('<f', struct.pack('<I', b))[0])


def evaluate(tool, parameter, color):
    r, g, b, a = color
    c = [r, g, b]
    neutral = 1000 if tool == 'saturation' else 6504 if tool == 'temperature' else 0
    if parameter == neutral or a == 0:
        return color
    p = D(parameter)
    gain = (D(2).ln() * p / 1000).exp()
    y = r if r == g == b else D('0.2126')*r+D('0.7152')*g+D('0.0722')*b
    if tool == 'exposure':
        c = [v*gain for v in c]
    elif tool == 'brightness':
        c = [v+a*p/1000 for v in c]
    elif tool == 'contrast':
        pivot = D('0.18')*a
        c = [pivot+gain*(v-pivot) for v in c]
    elif tool in ('highlights', 'shadows'):
        l = y/a
        t = (l-D('0.18'))/D('0.82') if tool == 'highlights' else l/D('0.18')
        t = max(D(0), min(D(1), t))
        weight = t*t*(3-2*t)
        if tool == 'shadows':
            weight = 1-weight
        c = [v*(1+weight*(gain-1)) for v in c]
    elif tool == 'saturation':
        c = [y+p/1000*(v-y) for v in c]
    elif tool == 'temperature':
        source, target = dot(B, white(parameter)), dot(B, white(6504))
        lms = dot(B, dot(M, c))
        c = dot(MI, dot(BI, [v*t/s for v, t, s in zip(lms, target, source)]))
    return c+[a]


def generated():
    limits = [('brightness', -1000, 1000, 0), ('contrast', -2000, 2000, 0),
              ('exposure', -5000, 5000, 0), ('highlights', -2000, 2000, 0),
              ('saturation', 0, 2000, 1000), ('shadows', -2000, 2000, 0),
              ('temperature', 4000, 25000, 6504)]
    colors = [(0, 0, 0, 0), (0, 0, 0, 1), ('-0','0','-0','-0'),
              ('1.40129846e-45','-1.40129846e-45',0,'1.40129846e-45')]
    colors += [(v, v, v, 1) for v in ('0.01', '0.18', '0.5', '1', '-0.25', '2')]
    colors += [(r, g, b, 1) for r, g, b in [(1,0,0),(0,1,0),(0,0,1),(1,1,0),(1,0,1),(0,1,1)]]
    colors += [('0.025', '0.125', '0.225', '0.25'), ('0.9', '0.2', '0.05', 1),
               ('1e-30', '-1e-30', 0, '1e-30'), ('0.5', '0.25', '0.75', '0.5')]
    rows = []
    for tool, low, high, neutral in limits:
        parameters = sorted({low, low+1, neutral, high-1, high} |
                            ({7000, 7001} if tool == 'temperature' else {low//2, high//2}))
        for parameter in parameters:
            for color in colors:
                inputs = [bits(D(v)) for v in color]
                outputs = [bits(v) for v in evaluate(tool, parameter, [value(b) for b in inputs])]
                rows.append('    {"%s", %d, {%s}, {%s}},' %
                            (tool, parameter, ','.join('0x%08xu'%b for b in inputs),
                             ','.join('0x%08xu'%b for b in outputs)))
    fixtures = ('// Independent Decimal(70) oracle, scripts/tone-reference.py.\n'
                'struct ToneFixture { const char* tool; int parameter; unsigned input[4], expected[4]; };\n'
                'constexpr ToneFixture tone_fixtures[] = {\n'+'\n'.join(rows)+'\n};\n')
    constants = '#ifndef PIXAURA_TONE_CONSTANTS_HPP\n#define PIXAURA_TONE_CONSTANTS_HPP\nnamespace pixaura::tone {\n'
    for name, matrix in [('rgb_xyz', M), ('xyz_rgb', MI), ('bradford', B), ('bradford_inverse', BI)]:
        constants += 'inline constexpr double %s[3][3] = {\n'%name
        constants += ''.join('    {'+', '.join(float(v).hex() for v in row)+'},\n' for row in matrix)
        constants += '};\n'
    constants += '}\n#endif\n'
    return {ROOT/'packages/core/tests/tone_fixtures.hpp': fixtures,
            ROOT/'packages/core/src/tone_constants.hpp': constants}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    for path, text in generated().items():
        if args.check:
            assert path.read_text(encoding='utf-8') == text, path
        else:
            path.write_text(text, encoding='utf-8', newline='\n')
    print('Independent tone fixture and matrix coefficient contract PASS')
