#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Project Health Check - Quick validation of project readiness
"""

import sys
from pathlib import Path

# Fix Windows console encoding
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

def check_project_health():
    """Quick health check of project"""
    project_root = Path(__file__).parent
    
    print("=" * 80)
    print("PROJECT HEALTH CHECK")
    print("=" * 80)
    print()
    
    checks = []
    
    # Check 1: Memory Bank
    mb_dir = project_root / '.project'
    required_mb_files = ['projectbrief.md', 'productContext.md', 'activeContext.md', 
                        'systemPatterns.md', 'techContext.md', 'progress.md']
    mb_status = all((mb_dir / f).exists() for f in required_mb_files)
    checks.append(("Memory Bank Files", mb_status, "6/6 files present"))
    
    # Check 2: Firmware Files
    firmware_dir = project_root / 'epic_can_logger'
    standard_fw = (firmware_dir / 'epic_can_logger.ino').exists()
    iso_fw = (firmware_dir / 'epic_can_logger_iso.ino').exists()
    checks.append(("Standard Firmware", standard_fw, "epic_can_logger.ino"))
    checks.append(("ISO Firmware", iso_fw, "epic_can_logger_iso.ino"))
    
    # Check 3: Core Modules
    core_modules = ['sd_logger', 'rusefi_dbc', 'config_manager']
    module_status = all(
        (firmware_dir / f"{m}.h").exists() and (firmware_dir / f"{m}.cpp").exists()
        for m in core_modules
    )
    checks.append(("Core Modules", module_status, f"{len(core_modules)} modules"))
    
    # Check 4: ISO Modules (if ISO version exists)
    if iso_fw:
        iso_modules = ['iso15765', 'uds']
        iso_module_status = all(
            (firmware_dir / f"{m}.h").exists() and (firmware_dir / f"{m}.cpp").exists()
            for m in iso_modules
        )
        checks.append(("ISO Modules", iso_module_status, f"{len(iso_modules)} modules"))
    
    # Check 5: Documentation
    docs = ['README.md', 'VERSIONS.md']
    doc_status = all((firmware_dir / d).exists() for d in docs)
    checks.append(("Key Documentation", doc_status, "README.md, VERSIONS.md"))
    
    # Check 6: MCP Configuration
    mcp_config = project_root / '.cursor' / 'mcp.json'
    mcp_status = mcp_config.exists()
    if mcp_status:
        try:
            import json
            with open(mcp_config) as f:
                mcp_data = json.load(f)
            mcp_uses_python = any(
                s.get('command') == 'python' 
                for s in mcp_data.get('mcpServers', {}).values()
            )
            mcp_status = mcp_uses_python
            mcp_detail = "Uses python command" if mcp_status else "Still uses uvx"
        except:
            mcp_detail = "File exists but invalid"
    else:
        mcp_detail = "Not found"
    checks.append(("MCP Configuration", mcp_status, mcp_detail))
    
    # Print results
    all_passed = all(status for _, status, _ in checks)
    
    for name, status, detail in checks:
        symbol = "✓" if status else "✗"
        print(f"{symbol} {name:25} - {detail}")
    
    print()
    print("=" * 80)
    if all_passed:
        print("✓ PROJECT STATUS: HEALTHY")
        print("  All critical components are present and configured correctly.")
    else:
        print("⚠ PROJECT STATUS: NEEDS ATTENTION")
        print("  Some components are missing or need configuration.")
    print("=" * 80)

if __name__ == '__main__':
    check_project_health()

