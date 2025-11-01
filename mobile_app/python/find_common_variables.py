#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Find common sensor variables from EPIC variables.json"""

import json
import sys

if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

vars_file = r'c:\Users\user1\Downloads\epicefi_fw-master\epicefi_fw-master\epic_can_bus\variables.json'

with open(vars_file, 'r', encoding='utf-8') as f:
    variables = json.load(f)

# Common sensor names to search for
search_terms = [
    'CLT', 'CoolantTemp', 'coolantTemperature',
    'IAT', 'IntakeTemp', 'intakeAirTemperature',
    'MAP', 'MAPValue',
    'OilPress', 'oilPressure',
    'TPS', 'TPSValue',
    'RPM', 'RPMValue',
    'AFR', 'AFRValue',
    'Boost'
]

print("Common Sensor Variables:")
print("=" * 80)

matches = []
for var in variables:
    var_name = var.get('name', '').lower()
    if var.get('source') == 'output':
        for term in search_terms:
            if term.lower() in var_name:
                matches.append(var)
                break

# Remove duplicates and sort
unique_matches = {}
for var in matches:
    name = var.get('name')
    if name not in unique_matches:
        unique_matches[name] = var

sorted_matches = sorted(unique_matches.values(), key=lambda x: x.get('name', ''))

for var in sorted_matches:
    name = var.get('name')
    hash_id = var.get('hash')
    print(f"{name:40} Hash: {hash_id:>15}")

print(f"\nTotal found: {len(sorted_matches)}")

