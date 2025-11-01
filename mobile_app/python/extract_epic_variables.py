#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Extract EPIC ECU Variables from variables.json
Processes the EPIC firmware variables.json file and generates
JavaScript/JSON data for the mobile app dropdown
"""

import json
import sys
from pathlib import Path

# Fix Windows console encoding
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

def extract_variables(variables_json_path):
    """Extract all variables from EPIC variables.json"""
    with open(variables_json_path, 'r', encoding='utf-8') as f:
        variables = json.load(f)
    
    # Sort by name for easier browsing
    variables_sorted = sorted(variables, key=lambda x: x.get('name', '').lower())
    
    print(f"Total variables found: {len(variables)}")
    print(f"Output variables: {sum(1 for v in variables if v.get('source') == 'output')}")
    print(f"Config variables: {sum(1 for v in variables if v.get('source') == 'config')}")
    
    return variables_sorted

def generate_javascript_map(variables, output_vars_only=True):
    """Generate JavaScript variable map for mobile app"""
    if output_vars_only:
        # Filter to only output variables (readable variables)
        filtered = [v for v in variables if v.get('source') == 'output']
    else:
        filtered = variables
    
    js_map = {}
    for var in filtered:
        var_hash = var.get('hash')
        var_name = var.get('name', 'Unknown')
        
        # Convert hash to string key (handles negative hashes)
        hash_key = str(var_hash)
        js_map[hash_key] = {
            'name': var_name,
            'id': var_hash,
            'source': var.get('source', 'unknown')
        }
    
    return js_map

def generate_html_options(variables, limit=None, output_vars_only=True):
    """Generate HTML option elements for dropdown"""
    if output_vars_only:
        filtered = [v for v in variables if v.get('source') == 'output']
    else:
        filtered = variables
    
    if limit:
        filtered = filtered[:limit]
    
    options = []
    for var in filtered:
        var_hash = var.get('hash')
        var_name = var.get('name', 'Unknown')
        hash_key = str(var_hash)
        options.append(f'                    <option value="{hash_key}">{var_name} (ID: {var_hash})</option>')
    
    return '\n'.join(options)

def find_keyboard_basic1_variables(variables):
    """Find the 3 variables used in keyboard_basic1"""
    keyboard_vars = {
        1272048601: 'TPSValue',
        1699696209: 'RPMValue',
        -1093429509: 'AFRValue'
    }
    
    found = []
    for var in variables:
        var_hash = var.get('hash')
        if var_hash in keyboard_vars:
            found.append({
                'hash': var_hash,
                'name': var.get('name'),
                'expected': keyboard_vars[var_hash],
                'source': var.get('source')
            })
    
    return found

def main():
    # Try project-local file first, then fall back to EPIC firmware location
    epic_vars_path = Path('keyboard_basic1/variables.json')
    if not epic_vars_path.exists():
        epic_vars_path = Path(r'c:\Users\user1\Downloads\epicefi_fw-master\epicefi_fw-master\epic_can_bus\variables.json')
    
    if not epic_vars_path.exists():
        print(f"Error: Variables file not found at {epic_vars_path}")
        sys.exit(1)
    
    print("=" * 80)
    print("EPIC ECU Variables Extractor")
    print("=" * 80)
    print()
    
    # Extract variables
    variables = extract_variables(epic_vars_path)
    
    # Find keyboard_basic1 variables
    print("\nVariables used in keyboard_basic1:")
    kb_vars = find_keyboard_basic1_variables(variables)
    for var in kb_vars:
        match = "[OK]" if var['name'] == var['expected'] else "[DIFF]"
        print(f"  {match} {var['name']} (Hash: {var['hash']}, Source: {var['source']})")
    
    # Generate JavaScript map (all output variables)
    print("\nGenerating JavaScript variable map...")
    js_map = generate_javascript_map(variables, output_vars_only=True)
    
    # Save JavaScript map
    output_file = Path('mobile_app/epic_variables_map.js')
    output_file.parent.mkdir(parents=True, exist_ok=True)
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('// EPIC ECU Variables Map\n')
        f.write('// Auto-generated from EPIC firmware variables.json\n')
        f.write('// Total variables: ' + str(len(variables)) + '\n')
        f.write('// Output variables: ' + str(len(js_map)) + '\n\n')
        f.write('const EPIC_VARIABLES_MAP = ')
        json.dump(js_map, f, indent=2)
        f.write(';\n')
    
    print(f"✓ Saved JavaScript map to {output_file}")
    print(f"  Contains {len(js_map)} output variables")
    
    # Generate HTML options (top 100 most common)
    print("\nGenerating HTML dropdown options...")
    html_options = generate_html_options(variables, limit=100, output_vars_only=True)
    
    # Save HTML options
    html_file = Path('mobile_app/epic_variables_options.html')
    with open(html_file, 'w', encoding='utf-8') as f:
        f.write('<!-- EPIC ECU Variables Dropdown Options -->\n')
        f.write('<!-- Top 100 output variables (most common) -->\n')
        f.write('<!-- Full list available in epic_variables_map.js -->\n\n')
        f.write(html_options)
    
    print(f"✓ Saved HTML options to {html_file}")
    print(f"  Contains {html_options.count('<option')} options")
    
    # Create summary document
    summary_file = Path('EPIC_VARIABLES_SUMMARY.md')
    with open(summary_file, 'w', encoding='utf-8') as f:
        f.write('# EPIC ECU Variables Summary\n\n')
        f.write(f'**Total Variables**: {len(variables)}\n\n')
        f.write(f'**Output Variables** (readable): {sum(1 for v in variables if v.get("source") == "output")}\n\n')
        f.write(f'**Config Variables** (writable): {sum(1 for v in variables if v.get("source") == "config")}\n\n')
        f.write('## Variables Used in keyboard_basic1\n\n')
        f.write('| Variable Name | Hash ID | Source |\n')
        f.write('|---------------|---------|--------|\n')
        for var in kb_vars:
            f.write(f'| {var["name"]} | {var["hash"]} | {var["source"]} |\n')
        f.write('\n## Common Output Variables\n\n')
        f.write('The first 50 output variables (alphabetically sorted):\n\n')
        f.write('| Variable Name | Hash ID |\n')
        f.write('|---------------|---------|\n')
        output_vars = sorted([v for v in variables if v.get('source') == 'output'], 
                            key=lambda x: x.get('name', '').lower())[:50]
        for var in output_vars:
            f.write(f'| {var["name"]} | {var["hash"]} |\n')
    
    print(f"✓ Saved summary to {summary_file}")
    
    print("\n" + "=" * 80)
    print("✓ Extraction complete!")
    print("=" * 80)
    print(f"\nFiles created:")
    print(f"  1. {output_file} - Full JavaScript map ({len(js_map)} variables)")
    print(f"  2. {html_file} - HTML dropdown options (top 100)")
    print(f"  3. {summary_file} - Summary document")

if __name__ == '__main__':
    main()

