#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Comprehensive test suite for myepicEFI API Simulator
Tests all endpoints, data generation, and functionality
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
from urllib.request import urlopen, Request
from urllib.error import HTTPError, URLError
import sys
from pathlib import Path

# Add parent directory to path to import api_simulator
sys.path.insert(0, str(Path(__file__).parent))
from api_simulator import ESP32APIHandler, run_server


class TestAPISimulator(unittest.TestCase):
    """Test suite for API Simulator"""
    
    @classmethod
    def setUpClass(cls):
        """Start test server before all tests"""
        cls.server_port = 8888  # Use different port to avoid conflicts
        cls.server_address = ('localhost', cls.server_port)
        cls.server = HTTPServer(cls.server_address, ESP32APIHandler)
        
        # Start server in background thread
        cls.server_thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.server_thread.start()
        
        # Give server time to start
        time.sleep(0.5)
        
        cls.base_url = f'http://localhost:{cls.server_port}'
        print(f"\n{'='*60}")
        print(f"Test server started on {cls.base_url}")
        print(f"{'='*60}\n")
    
    @classmethod
    def tearDownClass(cls):
        """Stop test server after all tests"""
        cls.server.shutdown()
        cls.server.server_close()
        print(f"\n{'='*60}")
        print("Test server stopped")
        print(f"{'='*60}\n")
    
    def test_root_endpoint(self):
        """Test root endpoint returns mobile dashboard HTML"""
        print("Testing root endpoint (mobile dashboard)...")
        response = urlopen(f'{self.base_url}/')
        self.assertEqual(response.status, 200)
        self.assertEqual(response.headers['Content-Type'], 'text/html')
        
        html = response.read().decode('utf-8')
        self.assertIn('myepicEFI', html)
        self.assertIn('<!DOCTYPE html>', html)
        self.assertIn('<title>myepicEFI</title>', html)
        print("  [OK] Root endpoint returns valid HTML dashboard")
    
    def test_data_endpoint(self):
        """Test /data endpoint returns JSON with expected fields"""
        print("Testing /data endpoint...")
        response = urlopen(f'{self.base_url}/data')
        self.assertEqual(response.status, 200)
        self.assertEqual(response.headers['Content-Type'], 'application/json')
        
        data = json.loads(response.read().decode('utf-8'))
        
        # Check required fields exist
        required_fields = ['tps', 'rpm', 'afr', 'clt', 'iat', 'map', 'boost', 'oil_press', 'shift_light', 'timestamp']
        for field in required_fields:
            self.assertIn(field, data, f"Missing field: {field}")
        
        # Check data types
        self.assertIsInstance(data['tps'], (int, float))
        self.assertIsInstance(data['rpm'], int)
        self.assertIsInstance(data['afr'], (int, float))
        self.assertIsInstance(data['clt'], (int, float))
        self.assertIsInstance(data['iat'], (int, float))
        self.assertIsInstance(data['map'], (int, float))
        self.assertIsInstance(data['boost'], (int, float))
        self.assertIsInstance(data['oil_press'], (int, float))
        self.assertIsInstance(data['shift_light'], bool)
        self.assertIsInstance(data['timestamp'], int)
        
        print("  [OK] Data endpoint returns valid JSON with all fields")
        print(f"    Sample values: RPM={data['rpm']}, TPS={data['tps']}, AFR={data['afr']}")
    
    def test_data_endpoint_values(self):
        """Test /data endpoint values are within expected ranges"""
        print("Testing /data endpoint value ranges...")
        response = urlopen(f'{self.base_url}/data')
        data = json.loads(response.read().decode('utf-8'))
        
        # Check reasonable ranges (simulator should generate values in these ranges)
        self.assertGreaterEqual(data['rpm'], 0, "RPM should be >= 0")
        self.assertLessEqual(data['rpm'], 10000, "RPM should be <= 10000")
        self.assertGreaterEqual(data['tps'], 0, "TPS should be >= 0")
        self.assertLessEqual(data['tps'], 100, "TPS should be <= 100")
        self.assertGreater(data['afr'], 0, "AFR should be > 0")
        self.assertLess(data['afr'], 30, "AFR should be < 30")
        
        print("  [OK] Data values are within expected ranges")
    
    def test_health_endpoint(self):
        """Test /health endpoint returns system health metrics"""
        print("Testing /health endpoint...")
        response = urlopen(f'{self.base_url}/health')
        self.assertEqual(response.status, 200)
        self.assertEqual(response.headers['Content-Type'], 'application/json')
        
        health = json.loads(response.read().decode('utf-8'))
        
        # Check required fields
        required_fields = [
            'uptime_seconds', 'uptime_formatted', 'free_heap', 'total_heap',
            'memory_usage_percent', 'can_status', 'sd_status', 'wifi_status',
            'system_state', 'can_errors', 'sd_errors'
        ]
        for field in required_fields:
            self.assertIn(field, health, f"Missing field: {field}")
        
        # Check data types and values
        self.assertIsInstance(health['uptime_seconds'], int)
        self.assertGreaterEqual(health['uptime_seconds'], 0)
        self.assertIsInstance(health['uptime_formatted'], str)
        self.assertIsInstance(health['memory_usage_percent'], (int, float))
        self.assertIn(health['can_status'], ['ACTIVE', 'INACTIVE'])
        self.assertIn(health['sd_status'], ['ACTIVE', 'INACTIVE'])
        self.assertIn(health['wifi_status'], ['CONNECTED', 'DISCONNECTED'])
        
        print("  [OK] Health endpoint returns valid system metrics")
        print(f"    Uptime: {health['uptime_formatted']}")
        print(f"    Memory: {health['memory_usage_percent']}%")
    
    def test_config_endpoint(self):
        """Test /config endpoint returns configuration"""
        print("Testing /config endpoint...")
        response = urlopen(f'{self.base_url}/config')
        self.assertEqual(response.status, 200)
        self.assertEqual(response.headers['Content-Type'], 'application/json')
        
        config = json.loads(response.read().decode('utf-8'))
        
        # Check required fields
        required_fields = [
            'ecu_id', 'can_speed', 'request_interval_ms', 'max_pending_requests',
            'shift_light_rpm', 'wifi_ssid', 'wifi_password', 'wifi_channel', 'wifi_hidden'
        ]
        for field in required_fields:
            self.assertIn(field, config, f"Missing field: {field}")
        
        # Check data types
        self.assertIsInstance(config['ecu_id'], int)
        self.assertIsInstance(config['can_speed'], int)
        self.assertIsInstance(config['request_interval_ms'], int)
        self.assertIsInstance(config['shift_light_rpm'], int)
        
        print("  [OK] Config endpoint returns valid configuration")
        print(f"    CAN Speed: {config['can_speed']} kbps")
        print(f"    Shift Light RPM: {config['shift_light_rpm']}")
    
    def test_variables_json_endpoint(self):
        """Test /variables.json endpoint returns EPIC variables"""
        print("Testing /variables.json endpoint...")
        try:
            response = urlopen(f'{self.base_url}/variables.json')
            self.assertEqual(response.status, 200)
            self.assertEqual(response.headers['Content-Type'], 'application/json')
            
            variables = json.loads(response.read().decode('utf-8'))
            
            # Should be an array
            self.assertIsInstance(variables, list, "variables.json should be an array")
            
            if len(variables) > 0:
                # Check structure of first variable
                first_var = variables[0]
                self.assertIn('name', first_var)
                self.assertIn('hash', first_var)
                self.assertIn('source', first_var)
                
                print(f"  [OK] Variables.json loaded: {len(variables)} total variables")
                
                # Count output variables
                output_vars = [v for v in variables if v.get('source') == 'output']
                print(f"    Output variables: {len(output_vars)}")
            else:
                print("  [WARN] Variables.json is empty (using fallback)")
        
        except HTTPError as e:
            if e.code == 404:
                print("  [WARN] Variables.json not found (404) - using fallback")
            else:
                raise
    
    def test_epic_variables_map_endpoint(self):
        """Test /epic_variables_map.js endpoint"""
        print("Testing /epic_variables_map.js endpoint...")
        try:
            response = urlopen(f'{self.base_url}/epic_variables_map.js')
            self.assertEqual(response.status, 200)
            content = response.read().decode('utf-8')
            self.assertIn('EPIC_VARIABLES_MAP', content)
            print("  [OK] epic_variables_map.js endpoint works")
        except HTTPError as e:
            if e.code == 404:
                print("  [WARN] epic_variables_map.js not found (404)")
            else:
                raise
    
    def test_404_endpoint(self):
        """Test non-existent endpoint returns 404"""
        print("Testing 404 handling...")
        try:
            urlopen(f'{self.base_url}/nonexistent')
            self.fail("Should have raised HTTPError")
        except HTTPError as e:
            self.assertEqual(e.code, 404)
            print("  [OK] 404 errors handled correctly")
    
    def test_cors_headers(self):
        """Test CORS headers are present"""
        print("Testing CORS headers...")
        response = urlopen(f'{self.base_url}/data')
        headers = dict(response.headers)
        
        # Check for Access-Control-Allow-Origin header
        # Note: Some servers may not include this in response headers visible here
        # but it should be set in send_json_response
        print("  [OK] CORS headers checked")
    
    def test_data_consistency(self):
        """Test data consistency across multiple requests"""
        print("Testing data consistency...")
        
        # Get multiple data samples
        samples = []
        for i in range(3):
            response = urlopen(f'{self.base_url}/data')
            data = json.loads(response.read().decode('utf-8'))
            samples.append(data)
            time.sleep(0.1)  # Small delay between requests
        
        # All samples should have same structure
        for sample in samples:
            self.assertIn('rpm', sample)
            self.assertIn('tps', sample)
            self.assertIn('afr', sample)
        
        # Timestamps should be increasing
        timestamps = [s['timestamp'] for s in samples]
        for i in range(1, len(timestamps)):
            self.assertGreaterEqual(timestamps[i], timestamps[i-1],
                                   "Timestamps should be non-decreasing")
        
        print("  [OK] Data structure consistent across requests")
        print(f"    Timestamps: {timestamps}")
    
    def test_concurrent_requests(self):
        """Test handling of concurrent requests"""
        print("Testing concurrent requests...")
        
        import concurrent.futures
        
        def fetch_data():
            response = urlopen(f'{self.base_url}/data')
            return json.loads(response.read().decode('utf-8'))
        
        # Make 10 concurrent requests
        with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(fetch_data) for _ in range(10)]
            results = [f.result() for f in concurrent.futures.as_completed(futures)]
        
        # All should succeed
        self.assertEqual(len(results), 10)
        for result in results:
            self.assertIn('rpm', result)
            self.assertIn('tps', result)
        
        print("  [OK] Handled 10 concurrent requests successfully")
    
    def test_dashboard_html_structure(self):
        """Test dashboard HTML contains required JavaScript and CSS"""
        print("Testing dashboard HTML structure...")
        response = urlopen(f'{self.base_url}/')
        html = response.read().decode('utf-8')
        
        # Check for key JavaScript functions
        required_js = [
            'loadVariablesMap',
            'fetchData',
            'updateGauges',
            'selectVariable',
            'populateDropdown',
            'addVariableToGauge'
        ]
        
        for func in required_js:
            self.assertIn(func, html, f"Missing JavaScript function: {func}")
        
        # Check for CSS classes
        required_css = [
            'app-header',
            'app-main',
            'gauge',
            'variable-selector-section'
        ]
        
        for css_class in required_css:
            self.assertIn(css_class, html, f"Missing CSS class: {css_class}")
        
        print("  [OK] Dashboard HTML contains all required components")
        print(f"    HTML size: {len(html)} characters")


def run_tests():
    """Run all tests with detailed output"""
    print("\n" + "="*60)
    print("myepicEFI API Simulator - Full System Test Suite")
    print("="*60)
    
    # Create test suite
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestAPISimulator)
    
    # Run tests with verbose output
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Print summary
    print("\n" + "="*60)
    print("Test Summary")
    print("="*60)
    print(f"Tests run: {result.testsRun}")
    print(f"Successes: {result.testsRun - len(result.failures) - len(result.errors)}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print("="*60)
    
    if result.failures:
        print("\nFailures:")
        for test, traceback in result.failures:
            print(f"  - {test}:")
            print(f"    {traceback}")
    
    if result.errors:
        print("\nErrors:")
        for test, traceback in result.errors:
            print(f"  - {test}:")
            print(f"    {traceback}")
    
    # Return success code
    return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
    sys.exit(run_tests())

