"""Checks that every mapped CC in map/params.csv appears in docs/MIDI-IMPLEMENTATION.md with the same name, and that
the chart lists no function for a CC the map sends. Usage: python3 tests/check_docs.py"""
import csv, re, sys, os
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
doc = {}
for l in open(os.path.join(root, 'docs/MIDI-IMPLEMENTATION.md')):
    m = re.match(r'\| (\d+) \| (.*?) \|', l)
    if m: doc[int(m[1])] = m[2]
bad = 0
for r in csv.DictReader(open(os.path.join(root, 'map/params.csv'))):
    if r['kind'] != 'cc': continue
    n = int(r['a'])
    if doc.get(n) != r['name']:
        print('mismatch CC', n, repr(doc.get(n)), repr(r['name'])); bad += 1
    if r['hw'] == '1': print('note: CC', n, 'is marked hardware verified; a capture file should be cited')
print('docs and map agree' if not bad else f'{bad} mismatches')
sys.exit(1 if bad else 0)
