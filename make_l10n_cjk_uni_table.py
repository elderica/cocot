#!/usr/bin/env python3
#
# Generate l10n_cjk_uni_table.c from EastAsianWidth.txt.
#
# Code points whose East_Asian_Width is W (Wide), F (Fullwidth) or
# A (Ambiguous) occupy two columns on a CJK terminal. They are emitted
# as a sorted list of merged ranges; everything else is one column.
#
# Usage: make_l10n_cjk_uni_table.py [-u VERSION] [-i EastAsianWidth.txt]
#                                   [-o l10n_cjk_uni_table.c]
#
# EastAsianWidth.txt is downloaded from unicode.org when it is missing
# or does not match VERSION.

import argparse
import os
import re
import sys
import urllib.error
import urllib.request

UNICODE_VERSION = '18.0.0'
URL = 'https://www.unicode.org/Public/{}/ucd/EastAsianWidth.txt'

WIDE = {'W', 'F', 'A'}

LINE_RE = re.compile(r'^([0-9A-F]+)(?:\.\.([0-9A-F]+))?\s*;\s*(\w+)', re.I)
VERSION_RE = re.compile(r'^#\s*(EastAsianWidth-[\d.]+)\.txt')


def file_version(path):
    try:
        with open(path, encoding='utf-8') as f:
            m = VERSION_RE.match(f.readline())
    except FileNotFoundError:
        return None
    return m.group(1) if m else None


def download(version, path):
    url = URL.format(version)
    print('Downloading ' + url, file=sys.stderr)
    try:
        with urllib.request.urlopen(url) as res:
            data = res.read()
    except urllib.error.URLError as e:
        sys.exit('{}: {}'.format(url, e))
    with open(path, 'wb') as f:
        f.write(data)


def parse(path):
    version = None
    ranges = []
    with open(path, encoding='utf-8') as f:
        for lineno, line in enumerate(f, 1):
            if version is None:
                m = VERSION_RE.match(line)
                if m:
                    version = m.group(1)
            if re.match(r'^\s*(#|$)', line):
                continue
            m = LINE_RE.match(line)
            if not m:
                sys.exit('{}:{}: unexpected line: {}'.format(path, lineno, line.rstrip()))
            if m.group(3) in WIDE:
                first = int(m.group(1), 16)
                last = int(m.group(2) or m.group(1), 16)
                ranges.append((first, last))
    if version is None:
        sys.exit(path + ': no version header')
    return version, ranges


def merge(ranges):
    merged = []
    for first, last in sorted(ranges):
        if merged and first <= merged[-1][1] + 1:
            merged[-1][1] = max(merged[-1][1], last)
        else:
            merged.append([first, last])
    return merged


def render(version, ranges):
    out = [
        '/*',
        ' * Two-column code points (East_Asian_Width W, F and A)',
        ' *',
        ' * Generated from {}.txt by make_l10n_cjk_uni_table.py.'.format(version),
        ' * Do not edit.',
        ' */',
        '',
        '#include "l10n_cjk_uni_table.h"',
        '',
        'const struct cjk_width_range cjk_wide_ranges[] = {',
    ]
    out += ['    {{ 0x{:06x}, 0x{:06x} }},'.format(a, b) for a, b in ranges]
    out += [
        '};',
        '',
        'const size_t cjk_wide_ranges_count =',
        '    sizeof(cjk_wide_ranges) / sizeof(cjk_wide_ranges[0]);',
    ]
    return '\n'.join(out) + '\n'


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('-u', '--unicode-version', default=UNICODE_VERSION)
    ap.add_argument('-i', '--input', default=os.path.join(here, 'EastAsianWidth.txt'))
    ap.add_argument('-o', '--output', default=os.path.join(here, 'l10n_cjk_uni_table.c'))
    args = ap.parse_args()

    expected = 'EastAsianWidth-' + args.unicode_version
    if file_version(args.input) != expected:
        download(args.unicode_version, args.input)
    version, ranges = parse(args.input)
    if version != expected:
        sys.exit('{}: expected {}, got {}'.format(args.input, expected, version))

    table = render(version, merge(ranges))
    with open(args.output, 'w', encoding='utf-8', newline='\n') as f:
        f.write(table)
    print('Wrote {} ({} ranges)'.format(args.output, table.count('{ 0x')), file=sys.stderr)


if __name__ == '__main__':
    main()
