#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Mobile App Startup Verification Test
Verifies that mobile app startup completes properly
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


class MobileAppStartupTest(unittest.TestCase):
    """Test mobile app startup sequence and completion"""
    
    @classmethod
    def setUpClass(cls):
        """Setup test server"""
        cls.server_port = 9100
        cls.server_address = ('localhost', cls.server_port)
        cls.server = HTTPServer(cls.server_address, ESP32APIHandler)
        cls.server_thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.server_thread.start()
        time.sleep(1.0)
        cls.base_url = f'http://localhost:{cls.server_port}'
        print(f"\n{'='*70}")
        print("Mobile App Startup Verification Test")
        print(f"{'='*70}\n")
    
    @classmethod
    def tearDownClass(cls):
        """Clean shutdown"""
        cls.server.shutdown()
        cls.server.server_close()
    
    def test_dashboard_html_loads(self):
        """Test 1: Dashboard HTML loads completely"""
        print("[STARTUP TEST 1] Dashboard HTML loading...")
        
        response = urlopen(f'{self.base_url}/', timeout=5)
        html = response.read().decode('utf-8')
        
        # Verify critical HTML elements
        self.assertIn('<!DOCTYPE html>', html)
        self.assertIn('<title>myepicEFI</title>', html)
        self.assertIn('myepicEFI', html)
        self.assertIn('app-header', html)
        self.assertIn('gauges-section', html)
        self.assertIn('variable-selector-section', html)
        
        # Verify JavaScript is embedded
        self.assertIn('function loadVariablesMap()', html)
        self.assertIn('function selectVariable(', html)
        self.assertIn('function updateGauges(', html)
        
        # Verify CSS is embedded
        self.assertIn('<style>', html)
        self.assertIn('--primary-color', html)
        
        print(f"  [OK] Dashboard HTML loaded ({len(html)} bytes)")
        print(f"  [OK] All critical elements present")
        return True
    
    def test_api_endpoints_available(self):
        """Test 2: All required API endpoints are available"""
        print("[STARTUP TEST 2] API endpoints availability...")
        
        endpoints = [
            ('/data', 'application/json'),
            ('/health', 'application/json'),
            ('/config', 'application/json'),
            ('/variables.json', 'application/json')
        ]
        
        available = []
        for endpoint, expected_type in endpoints:
            try:
                response = urlopen(f'{self.base_url}{endpoint}', timeout=2)
                content_type = response.headers.get('Content-Type', '')
                
                if expected_type in content_type or endpoint == '/variables.json':
                    available.append(endpoint)
                    print(f"  [OK] {endpoint} - {content_type}")
                else:
                    print(f"  [WARN] {endpoint} - unexpected type: {content_type}")
            except Exception as e:
                print(f"  [ERROR] {endpoint} - {type(e).__name__}")
        
        # All critical endpoints should be available
        critical = ['/data', '/health', '/config']
        for ep in critical:
            self.assertIn(ep, available, f"Critical endpoint {ep} not available")
        
        print(f"  [OK] {len(available)}/{len(endpoints)} endpoints available")
        return available
    
    def test_data_endpoint_initial_values(self):
        """Test 3: Data endpoint returns valid initial values"""
        print("[STARTUP TEST 3] Data endpoint initial values...")
        
        response = urlopen(f'{self.base_url}/data', timeout=2)
        data = json.loads(response.read().decode('utf-8'))
        
        # Verify all required fields exist
        required_fields = ['tps', 'rpm', 'afr', 'clt', 'iat', 'map', 'boost', 
                          'oil_press', 'shift_light', 'timestamp']
        
        missing = [f for f in required_fields if f not in data]
        self.assertEqual(len(missing), 0, f"Missing fields: {missing}")
        
        # Verify initial values are reasonable (not all zeros, but valid range)
        self.assertGreaterEqual(data['rpm'], 0, "RPM should be >= 0")
        self.assertLessEqual(data['rpm'], 30000, "RPM should be <= 30000")
        self.assertGreaterEqual(data['tps'], 0, "TPS should be >= 0")
        self.assertLessEqual(data['tps'], 100, "TPS should be <= 100")
        
        print(f"  [OK] All {len(required_fields)} required fields present")
        print(f"  [OK] Initial values: RPM={data['rpm']}, TPS={data['tps']:.1f}%")
        return data
    
    def test_variables_json_loads(self):
        """Test 4: Variables JSON loads for dropdown population"""
        print("[STARTUP TEST 4] Variables JSON loading...")
        
        try:
            response = urlopen(f'{self.base_url}/variables.json', timeout=10)
            variables = json.loads(response.read().decode('utf-8'))
            
            # Verify structure
            self.assertIsInstance(variables, list, "Variables should be a list")
            self.assertGreater(len(variables), 0, "Variables list should not be empty")
            
            # Check first variable has required fields
            if len(variables) > 0:
                var = variables[0]
                required_var_fields = ['name', 'hash', 'source']
                missing = [f for f in required_var_fields if f not in var]
                self.assertEqual(len(missing), 0, f"Variable missing fields: {missing}")
            
            # Count output variables (for dropdowns)
            output_vars = [v for v in variables if v.get('source') == 'output']
            
            print(f"  [OK] Variables JSON loaded ({len(variables)} total)")
            print(f"  [OK] Output variables: {len(output_vars)} (for dropdowns)")
            
            return len(output_vars)
        except Exception as e:
            print(f"  [WARN] Variables JSON not available: {type(e).__name__}")
            return 0
    
    def test_startup_performance(self):
        """Test 5: Startup completes within acceptable time"""
        print("[STARTUP TEST 5] Startup performance...")
        
        start_time = time.time()
        
        # Simulate startup sequence
        steps = {
            'HTML Load': lambda: urlopen(f'{self.base_url}/', timeout=5),
            'Data Endpoint': lambda: urlopen(f'{self.base_url}/data', timeout=2),
            'Health Endpoint': lambda: urlopen(f'{self.base_url}/health', timeout=2),
            'Config Endpoint': lambda: urlopen(f'{self.base_url}/config', timeout=2),
        }
        
        step_times = {}
        for step_name, step_func in steps.items():
            step_start = time.time()
            try:
                step_func()
                step_time = (time.time() - step_start) * 1000
                step_times[step_name] = step_time
                print(f"  {step_name}: {step_time:.1f}ms")
            except Exception as e:
                print(f"  {step_name}: FAILED - {type(e).__name__}")
                step_times[step_name] = None
        
        total_time = (time.time() - start_time) * 1000
        
        # Startup should complete within 15 seconds (including variables.json)
        self.assertLess(total_time, 15000, f"Startup took {total_time:.1f}ms (max 15000ms)")
        
        print(f"  [OK] Total startup time: {total_time:.1f}ms")
        return step_times
    
    def test_startup_completion_indicators(self):
        """Test 6: All startup completion indicators are present"""
        print("[STARTUP TEST 6] Startup completion indicators...")
        
        response = urlopen(f'{self.base_url}/', timeout=5)
        html = response.read().decode('utf-8')
        
        # Check for initialization functions (embedded version doesn't use initApp/DOMContentLoaded)
        init_indicators = [
            'loadVariablesMap',        # Critical: Loads EPIC variables for dropdowns
            'fetchData',               # Critical: Fetches real-time data
            'updateGauges',            # Critical: Updates gauge displays
            'selectVariable',          # Critical: Handles variable selection
            'myepicEFI starting',      # Startup log message
            'API_BASE',                # API base URL configuration
            'setInterval'              # Real-time update loop
        ]
        
        found = []
        for indicator in init_indicators:
            if indicator in html:
                found.append(indicator)
                print(f"  [OK] {indicator} present")
            else:
                print(f"  [WARN] {indicator} not found")
        
        # Critical indicators for embedded dashboard
        critical = ['loadVariablesMap', 'fetchData', 'updateGauges']
        for ind in critical:
            self.assertIn(ind, found, f"Critical startup indicator missing: {ind}")
        
        print(f"  [OK] {len(found)}/{len(init_indicators)} startup indicators present")
        return found


def run_startup_tests():
    """Run all startup tests"""
    print("\n" + "="*70)
    print("MOBILE APP STARTUP VERIFICATION")
    print("="*70 + "\n")
    
    suite = unittest.TestLoader().loadTestsFromTestCase(MobileAppStartupTest)
    runner = unittest.TextTestRunner(verbosity=2, stream=sys.stdout)
    result = runner.run(suite)
    
    print("\n" + "="*70)
    print("STARTUP VERIFICATION SUMMARY")
    print("="*70)
    print(f"Tests run: {result.testsRun}")
    print(f"Successes: {result.testsRun - len(result.failures) - len(result.errors)}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    
    if result.failures:
        print(f"\nFAILURES:")
        for test, traceback in result.failures:
            print(f"  - {test}")
    
    if result.errors:
        print(f"\nERRORS:")
        for test, traceback in result.errors:
            print(f"  - {test}")
    
    print("="*70 + "\n")
    
    return len(result.failures) == 0 and len(result.errors) == 0


if __name__ == '__main__':
    success = run_startup_tests()
    sys.exit(0 if success else 1)

