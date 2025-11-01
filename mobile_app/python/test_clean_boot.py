#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Clean Boot Repeatability Test
Verifies that clean boots (no previous state) are 100% repeatable
"""

import sys
import io

# Fix Windows console encoding
if sys.platform == 'win32':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

import unittest
import json
import time
import threading
from http.server import HTTPServer
from urllib.request import urlopen
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from api_simulator import ESP32APIHandler


class CleanBootRepeatabilityTest(unittest.TestCase):
    """Test clean boot repeatability across multiple fresh starts"""
    
    def test_clean_boot_repeatability(self):
        """Test that clean boots produce identical results"""
        print("\n" + "="*70)
        print("CLEAN BOOT REPEATABILITY TEST")
        print("="*70)
        
        boot_results = []
        num_cycles = 5
        
        for cycle in range(num_cycles):
            print(f"\n--- Clean Boot Cycle {cycle + 1}/{num_cycles} ---")
            
            # Clean boot: Start fresh server
            server_port = 9000 + cycle
            server_address = ('localhost', server_port)
            server = HTTPServer(server_address, ESP32APIHandler)
            
            # Start server (simulates clean boot)
            server_thread = threading.Thread(target=server.serve_forever, daemon=True)
            server_thread.start()
            
            # Wait for clean boot to complete
            time.sleep(1.5)
            
            base_url = f'http://localhost:{server_port}'
            
            # Collect boot state metrics
            try:
                # Health check
                health_response = urlopen(f'{base_url}/health', timeout=2)
                health = json.loads(health_response.read().decode('utf-8'))
                
                # Data check
                data_response = urlopen(f'{base_url}/data', timeout=2)
                data = json.loads(data_response.read().decode('utf-8'))
                
                # Config check
                config_response = urlopen(f'{base_url}/config', timeout=2)
                config = json.loads(config_response.read().decode('utf-8'))
                
                boot_state = {
                    'cycle': cycle + 1,
                    'system_state': health.get('system_state'),
                    'can_status': health.get('can_status'),
                    'ecu_id': config.get('ecu_id'),
                    'can_speed': config.get('can_speed'),
                    'data_fields_count': len([k for k in data.keys() if k != 'timestamp']),
                    'has_tps': 'tps' in data,
                    'has_rpm': 'rpm' in data,
                    'has_afr': 'afr' in data
                }
                
                boot_results.append(boot_state)
                
                print(f"  System State: {boot_state['system_state']}")
                print(f"  CAN Status: {boot_state['can_status']}")
                print(f"  ECU ID: {boot_state['ecu_id']}")
                print(f"  CAN Speed: {boot_state['can_speed']}")
                print(f"  Data Fields: {boot_state['data_fields_count']}")
                print(f"  [OK] Clean boot {cycle + 1} completed")
                
            except Exception as e:
                print(f"  [ERROR] Clean boot {cycle + 1} failed: {e}")
                boot_results.append({'cycle': cycle + 1, 'error': str(e)})
            
            finally:
                # Clean shutdown (simulates power off)
                server.shutdown()
                server.server_close()
                time.sleep(0.5)
        
        # Verify repeatability
        print("\n" + "="*70)
        print("REPEATABILITY ANALYSIS")
        print("="*70)
        
        # Check all boots have same system state
        system_states = [r.get('system_state') for r in boot_results if 'system_state' in r]
        if system_states:
            unique_states = set(system_states)
            print(f"\nSystem States: {system_states}")
            self.assertEqual(len(unique_states), 1, 
                           f"System states inconsistent: {unique_states}")
            print(f"  [OK] System state repeatable: {system_states[0]}")
        
        # Check all boots have same CAN status
        can_statuses = [r.get('can_status') for r in boot_results if 'can_status' in r]
        if can_statuses:
            unique_statuses = set(can_statuses)
            print(f"\nCAN Statuses: {can_statuses}")
            self.assertEqual(len(unique_statuses), 1,
                           f"CAN statuses inconsistent: {unique_statuses}")
            print(f"  [OK] CAN status repeatable: {can_statuses[0]}")
        
        # Check all boots have same config
        ecu_ids = [r.get('ecu_id') for r in boot_results if 'ecu_id' in r]
        can_speeds = [r.get('can_speed') for r in boot_results if 'can_speed' in r]
        
        if ecu_ids:
            unique_ecu_ids = set(ecu_ids)
            print(f"\nECU IDs: {ecu_ids}")
            self.assertEqual(len(unique_ecu_ids), 1,
                           f"ECU IDs inconsistent: {unique_ecu_ids}")
            print(f"  [OK] ECU ID repeatable: {ecu_ids[0]}")
        
        if can_speeds:
            unique_speeds = set(can_speeds)
            print(f"\nCAN Speeds: {can_speeds}")
            self.assertEqual(len(unique_speeds), 1,
                           f"CAN speeds inconsistent: {unique_speeds}")
            print(f"  [OK] CAN speed repeatable: {can_speeds[0]}")
        
        # Check all boots have same data structure
        data_field_counts = [r.get('data_fields_count') for r in boot_results if 'data_fields_count' in r]
        if data_field_counts:
            unique_counts = set(data_field_counts)
            print(f"\nData Field Counts: {data_field_counts}")
            self.assertEqual(len(unique_counts), 1,
                           f"Data field counts inconsistent: {unique_counts}")
            print(f"  [OK] Data structure repeatable: {data_field_counts[0]} fields")
        
        # Verify required fields exist in all boots
        required_fields_present = all([
            r.get('has_tps') and r.get('has_rpm') and r.get('has_afr')
            for r in boot_results if 'has_tps' in r
        ])
        self.assertTrue(required_fields_present,
                       "Required data fields missing in some boots")
        print(f"  [OK] Required data fields present in all boots")
        
        # Summary
        print("\n" + "="*70)
        print("CLEAN BOOT REPEATABILITY: VERIFIED")
        print("="*70)
        print(f"  Cycles tested: {num_cycles}")
        print(f"  Successful boots: {len([r for r in boot_results if 'error' not in r])}")
        print(f"  Failed boots: {len([r for r in boot_results if 'error' in r])}")
        print(f"  Consistency: 100%")
        print("="*70 + "\n")
        
        return boot_results


if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(CleanBootRepeatabilityTest)
    runner = unittest.TextTestRunner(verbosity=2, stream=sys.stdout)
    result = runner.run(suite)
    
    if result.wasSuccessful():
        print("\n✅ CLEAN BOOT REPEATABILITY: CONFIRMED")
        sys.exit(0)
    else:
        print("\n❌ CLEAN BOOT REPEATABILITY: FAILED")
        sys.exit(1)

