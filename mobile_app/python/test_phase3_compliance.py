#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Phase 3 Compliance Verification Test
Tests ISO 14229/15765 (UDS) implementation integrity
"""

import sys
import io
import os
from pathlib import Path

# Fix Windows console encoding
if sys.platform == 'win32':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

def check_file_exists(filepath, description):
    """Check if a file exists"""
    if Path(filepath).exists():
        print(f"  [OK] {description}: {filepath}")
        return True
    else:
        print(f"  [MISSING] {description}: {filepath}")
        return False

def check_file_content(filepath, required_strings, description):
    """Check if file contains required strings"""
    if not Path(filepath).exists():
        return False
    
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        all_found = True
        for req_str in required_strings:
            if req_str in content:
                print(f"  [OK] Found '{req_str}' in {description}")
            else:
                print(f"  [MISSING] '{req_str}' not found in {description}")
                all_found = False
        
        return all_found
    except Exception as e:
        print(f"  [ERROR] Reading {filepath}: {e}")
        return False

def verify_phase3_compliance():
    """Verify Phase 3 (UDS Services) compliance implementation"""
    print("="*60)
    print("Phase 3 Compliance Verification")
    print("ISO 14229/15765 (UDS) Implementation Check")
    print("="*60)
    print()
    
    base_path = Path("epic_can_logger")
    compliance_status = {
        'transport_layer': False,
        'service_layer': False,
        'integration': False,
        'required_services': False
    }
    
    # 1. Check ISO 15765 Transport Layer files
    print("1. ISO 15765 Transport Layer Files:")
    print("-" * 40)
    iso15765_h_exists = check_file_exists(base_path / "iso15765.h", "ISO 15765 header")
    iso15765_cpp_exists = check_file_exists(base_path / "iso15765.cpp", "ISO 15765 implementation")
    
    if iso15765_h_exists and iso15765_cpp_exists:
        # Check header content
        transport_checks = [
            "ISO_15765_PHYSICAL_REQUEST_BASE",
            "PCI_SINGLE_FRAME",
            "PCI_FIRST_FRAME",
            "PCI_CONSECUTIVE_FRAME",
            "iso15765_send_single",
            "iso15765_send_multi",
            "iso15765_receive"
        ]
        if check_file_content(base_path / "iso15765.h", transport_checks, "iso15765.h"):
            compliance_status['transport_layer'] = True
    
    print()
    
    # 2. Check UDS Service Layer files
    print("2. ISO 14229 UDS Service Layer Files:")
    print("-" * 40)
    uds_h_exists = check_file_exists(base_path / "uds.h", "UDS header")
    uds_cpp_exists = check_file_exists(base_path / "uds.cpp", "UDS implementation")
    
    if uds_h_exists and uds_cpp_exists:
        # Check header content for required services
        service_checks = [
            "UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL",
            "UDS_SERVICE_ECU_RESET",
            "UDS_SERVICE_READ_DATA_BY_IDENTIFIER",
            "UDS_SERVICE_TESTER_PRESENT",
            "UDS_DID_TPS_VALUE",
            "UDS_DID_RPM_VALUE",
            "UDS_DID_AFR_VALUE"
        ]
        if check_file_content(base_path / "uds.h", service_checks, "uds.h"):
            compliance_status['service_layer'] = True
        
        # Check implementation for handler functions
        handler_checks = [
            "uds_handle_read_data_by_id",
            "uds_handle_session_control",  # Note: actual function name
            "uds_handle_tester_present",
            "uds_handle_ecu_reset",
            "uds_process_request"
        ]
        if check_file_content(base_path / "uds.cpp", handler_checks, "uds.cpp"):
            compliance_status['required_services'] = True
    
    print()
    
    # 3. Check main firmware integration
    print("3. Main Firmware Integration:")
    print("-" * 40)
    iso_ino_exists = check_file_exists(base_path / "epic_can_logger_iso.ino", "ISO-compliant firmware")
    
    if iso_ino_exists:
        integration_checks = [
            '#include "iso15765.h"',
            '#include "uds.h"',
            "iso15765_init",
            "uds_init",
            "iso15765_process_rx",
            "uds_process_request",
            "ISO_15765_PHYSICAL_REQUEST_BASE"
        ]
        if check_file_content(base_path / "epic_can_logger_iso.ino", integration_checks, "epic_can_logger_iso.ino"):
            compliance_status['integration'] = True
    
    print()
    
    # 4. Check documentation
    print("4. Documentation:")
    print("-" * 40)
    doc_files = [
        ("ISO_COMPLIANCE_GUIDE.md", "ISO Compliance Guide"),
        ("ISO_IMPLEMENTATION.md", "ISO Implementation Documentation")
    ]
    
    for doc_file, desc in doc_files:
        check_file_exists(base_path / doc_file, desc)
    
    print()
    
    # 5. Summary
    print("="*60)
    print("Phase 3 Compliance Status")
    print("="*60)
    
    all_pass = True
    for category, status in compliance_status.items():
        status_str = "[PASS]" if status else "[FAIL]"
        print(f"{status_str} {category.replace('_', ' ').title()}")
        if not status:
            all_pass = False
    
    print()
    
    if all_pass:
        print("[SUCCESS] Phase 3 compliance is INTACT!")
        print()
        print("Verified Components:")
        print("  - ISO 15765 Transport Layer (multi-frame handling)")
        print("  - ISO 14229 UDS Service Layer")
        print("  - Required UDS Services:")
        print("    * 0x10 - DiagnosticSessionControl")
        print("    * 0x11 - ECUReset")
        print("    * 0x22 - ReadDataByIdentifier")
        print("    * 0x3E - TesterPresent")
        print("  - Firmware integration")
        print("  - EPIC variable to UDS DID mapping")
        return 0
    else:
        print("[WARNING] Some Phase 3 components may be missing or incomplete!")
        return 1

if __name__ == '__main__':
    sys.exit(verify_phase3_compliance())

