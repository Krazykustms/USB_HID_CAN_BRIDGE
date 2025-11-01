#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
myepicEFI API Simulator
Simulates the ESP32 web API for testing myepicEFI mobile app without hardware
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import time
from urllib.parse import urlparse, parse_qs

class ESP32APIHandler(BaseHTTPRequestHandler):
    """Simulates ESP32 web API endpoints"""
    
    # Simulated device state
    tps_value = 25.5
    rpm_value = 3500
    afr_value = 14.7
    clt_value = 20.0  # Coolant temperature
    iat_value = 25.0  # Intake air temperature
    map_value = 50.0  # Manifold absolute pressure
    boost_value = -51.3  # Boost (MAP - atmospheric)
    oil_press_value = 40.0  # Oil pressure
    shift_light_active = False
    start_time = time.time()
    request_count = 0
    
    def log_message(self, format, *args):
        """Override to reduce log noise"""
        pass
    
    def do_GET(self):
        """Handle GET requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == '/':
            self.send_mobile_dashboard()
        elif path == '/data':
            self.send_data()
        elif path == '/health':
            self.send_health()
        elif path == '/config':
            self.send_config()
        elif path == '/epic_variables_map.js':
            self.send_epic_variables_map()
        elif path == '/variables.json':
            self.send_variables_json()
        elif path == '/manifest.json':
            self.send_manifest()
        elif path.startswith('/images/icons/'):
            self.send_icon_file(path)
        elif path == '/favicon.png':
            self.send_icon_file('/images/icons/icon-32.png')
        elif path == '/favicon.ico':
            self.send_icon_file('/images/icons/icon-32.png')
        else:
            self.send_error(404, "Not Found")
    
    def do_POST(self):
        """Handle POST requests"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == '/config/save':
            self.handle_config_save()
        else:
            self.send_error(404, "Not Found")
    
    def send_mobile_dashboard(self):
        """Send mobile-optimized dashboard with embedded JavaScript"""
        html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=5.0, user-scalable=yes">
    <meta name="theme-color" content="#4CAF50">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <title>myepicEFI</title>
    
    <!-- PWA Manifest -->
    <link rel="manifest" href="/manifest.json">
    
    <!-- App Icons -->
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon.png">
    <link rel="apple-touch-icon" sizes="180x180" href="/images/icons/icon-180.png">
    <link rel="apple-touch-icon" sizes="192x192" href="/images/icons/icon-192.png">
    <link rel="apple-touch-icon" sizes="512x512" href="/images/icons/icon-512.png">
    <link rel="icon" type="image/png" sizes="96x96" href="/images/icons/icon-96.png">
    <link rel="icon" type="image/png" sizes="144x144" href="/images/icons/icon-144.png">
    <link rel="icon" type="image/png" sizes="192x192" href="/images/icons/icon-192.png">
    <link rel="icon" type="image/png" sizes="512x512" href="/images/icons/icon-512.png">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        :root {
            --primary-color: #4CAF50;
            --primary-dark: #1a1a1a;
            --text-primary: #FFFFFF;
            --text-secondary: #B0B0B0;
            --bg-primary: #1a1a1a;
            --bg-secondary: #2a2a2a;
            --bg-card: #2a2a2a;
            --border-color: #404040;
        }
        html {
            margin: 0;
            padding: 0;
            height: 100%;
            overflow-x: hidden;
            overflow-y: auto;
            -webkit-overflow-scrolling: touch;
        }
        body {
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background-color: var(--bg-primary);
            color: var(--text-primary);
            overflow-x: hidden;
            overflow-y: auto;
            -webkit-overflow-scrolling: touch;
            touch-action: pan-y pan-x;
            position: relative;
            min-height: 100%;
            height: auto;
        }
        .app-header {
            position: sticky;
            top: 0;
            background-color: var(--bg-secondary);
            padding: 8px 12px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            z-index: 100;
            border-bottom: 1px solid var(--border-color);
        }
        .app-title {
            font-size: 16px;
            font-weight: 600;
            color: var(--primary-color);
        }
        .status-indicator { display: flex; align-items: center; }
        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background-color: var(--primary-color);
            box-shadow: 0 0 8px var(--primary-color);
        }
        .app-main {
            padding: 8px 12px;
            max-width: 100%;
            margin: 0 auto;
            padding-bottom: 200px; /* Large padding to ensure all content is accessible above footer */
            min-height: calc(100vh - 60px); /* Minimum height minus header */
            width: 100%;
            box-sizing: border-box;
            position: relative;
            z-index: 1;
        }
        .gauges-section {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 8px;
            margin-bottom: 12px;
        }
        .dynamic-gauges-section {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 8px;
            margin-bottom: 12px;
        }
        .dynamic-gauge {
            transition: opacity 0.3s ease;
        }
        .dynamic-gauge.empty {
            opacity: 0.3;
        }
        .dynamic-gauge.populated {
            opacity: 1.0;
        }
        .gauge {
            width: 100%;
            min-width: 120px;
            aspect-ratio: 2 / 1;
            background: linear-gradient(135deg, var(--bg-card), var(--bg-secondary));
            border-radius: 12px;
            padding: 12px;
            text-align: center;
            border: 1px solid var(--border-color);
            cursor: pointer;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
            user-select: none;
            -webkit-user-select: none;
            -webkit-tap-highlight-color: transparent;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        .gauge:hover {
            transform: scale(1.02);
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
        }
        .gauge:active {
            transform: scale(0.98);
        }
        .original-gauge {
            cursor: pointer;
        }
        .dynamic-gauge {
            cursor: pointer;
        }
        .gauge-label {
            font-size: 10px;
            color: var(--text-secondary);
            margin-bottom: 4px;
            text-transform: uppercase;
            line-height: 1.2;
        }
        .gauge-value {
            font-size: 24px;
            font-weight: 700;
            color: var(--primary-color);
            line-height: 1.2;
            margin: 2px 0;
        }
        .gauge-unit {
            font-size: 10px;
            color: var(--text-secondary);
            margin-top: 2px;
            line-height: 1.2;
        }
        .shift-light-section {
            margin-bottom: 12px;
        }
        .shift-light {
            background: var(--bg-card);
            border-radius: 12px;
            padding: 10px;
            text-align: center;
            border: 1px solid var(--border-color);
        }
        .shift-light-label {
            font-size: 11px;
            color: var(--text-secondary);
            margin-bottom: 6px;
        }
        .shift-light-indicator {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            margin: 0 auto;
            transition: all 0.3s;
            background-color: #606060;
        }
        .shift-light-indicator.active {
            background-color: var(--primary-color);
            box-shadow: 0 0 20px var(--primary-color);
            animation: pulse 1s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.7; }
        }
        .stats-section {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 8px;
            margin-bottom: 12px;
        }
        .stat-card {
            background: var(--bg-card);
            border-radius: 10px;
            padding: 10px;
            text-align: center;
            border: 1px solid var(--border-color);
        }
        .stat-label {
            font-size: 10px;
            color: var(--text-secondary);
            margin-bottom: 4px;
        }
        .stat-value {
            font-size: 14px;
            font-weight: 600;
            color: var(--text-primary);
        }
        .actions-section {
            display: grid;
            gap: 8px;
            margin-bottom: 12px;
        }
        .action-button {
            background: var(--bg-card);
            border: 1px solid var(--border-color);
            border-radius: 10px;
            padding: 10px;
            color: var(--text-primary);
            font-size: 13px;
            cursor: pointer;
            min-height: 40px;
        }
        .app-footer {
            position: fixed;
            bottom: 0;
            left: 0;
            right: 0;
            background: var(--bg-secondary);
            padding: 6px 12px;
            text-align: center;
            border-top: 1px solid var(--border-color);
            z-index: 10;
            height: auto;
            min-height: 30px;
            pointer-events: none; /* Allow scrolling through footer area */
        }
        .app-footer .footer-info {
            pointer-events: auto; /* But allow clicks on footer content */
        }
        .footer-info {
            font-size: 10px;
            color: var(--text-secondary);
        }
        .variable-selector-section {
            margin-bottom: 12px;
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 8px;
            width: 100%;
        }
        .variable-selector-card {
            background: var(--bg-primary);
            border-radius: 12px;
            padding: 6px 12px;
            border: 1px solid var(--border-color);
            width: 100%;
            box-sizing: border-box;
        }
        .variable-select {
            width: 100%;
            padding: 4px 28px 4px 8px;
            background: var(--bg-primary);
            border: 1px solid var(--border-color);
            border-radius: 8px;
            color: var(--text-primary);
            font-size: 10px;
            margin-top: 4px;
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
            background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='10' height='10' viewBox='0 0 12 12'%3E%3Cpath fill='%23B0B0B0' d='M6 9L1 4h10z'/%3E%3C/svg%3E");
            background-repeat: no-repeat;
            background-position: right 8px center;
            background-size: 10px 10px;
            box-sizing: border-box;
            height: 22px;
            line-height: 22px;
            text-align: left;
            overflow: visible;
            white-space: nowrap;
        }
        .variable-select option {
            background: var(--bg-primary);
            color: var(--text-primary);
            font-size: 11px;
            text-align: left;
            padding: 4px;
        }
        .variable-selector-card .stat-label {
            font-size: 11px;
            margin-bottom: 2px;
        }
        .variable-search-info {
            font-size: 9px;
            margin-top: 2px;
        }
        .variable-select:focus {
            outline: none;
            border-color: var(--primary-color);
        }
        .variable-select option {
            background: var(--bg-primary);
            color: var(--text-primary);
        }
        .selected-variable-display {
            margin-top: 16px;
            padding: 12px;
            background: var(--bg-primary);
            border-radius: 8px;
            text-align: center;
        }
        .variable-name {
            font-size: 14px;
            color: var(--text-secondary);
            margin-bottom: 8px;
            text-transform: uppercase;
        }
        .variable-value {
            font-size: 32px;
            font-weight: 700;
            color: var(--primary-color);
            margin-bottom: 4px;
        }
        .variable-id {
            font-size: 12px;
            color: var(--text-secondary);
        }
        .variable-search-info {
            margin-top: 8px;
            font-size: 12px;
            color: var(--text-secondary);
            text-align: center;
        }
    </style>
</head>
<body>
    <header class="app-header">
        <h1 class="app-title">myepicEFI</h1>
        <div class="status-indicator" id="connection-status">
            <span class="status-dot"></span>
        </div>
    </header>

    <main class="app-main">
        <section class="variable-selector-section">
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 1</div>
                <select id="variable-selector-1" class="variable-select" onchange="selectVariable(1)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-1">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 2</div>
                <select id="variable-selector-2" class="variable-select" onchange="selectVariable(2)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-2">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 3</div>
                <select id="variable-selector-3" class="variable-select" onchange="selectVariable(3)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-3">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 4</div>
                <select id="variable-selector-4" class="variable-select" onchange="selectVariable(4)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-4">
                    <small>Loading variables...</small>
                </div>
            </div>
        </section>

        <section class="gauges-section">
            <div class="gauge-container">
                <div class="gauge original-gauge" id="rpm-gauge" data-gauge-id="rpm" data-variable-hash="1699696209" ondblclick="clearGauge('rpm')">
                    <div class="gauge-label">RPM</div>
                    <div class="gauge-value" id="rpm-value">0</div>
                    <div class="gauge-unit"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="tps-gauge" data-gauge-id="tps" data-variable-hash="1272048601" ondblclick="clearGauge('tps')">
                    <div class="gauge-label">Throttle</div>
                    <div class="gauge-value" id="tps-value">0</div>
                    <div class="gauge-unit">%</div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="afr-gauge" data-gauge-id="afr" data-variable-hash="-1093429509" ondblclick="clearGauge('afr')">
                    <div class="gauge-label">AFR</div>
                    <div class="gauge-value" id="afr-value">0.0</div>
                    <div class="gauge-unit"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="clt-gauge" data-gauge-id="clt" data-variable-hash="417946098" ondblclick="clearGauge('clt')">
                    <div class="gauge-label">Coolant</div>
                    <div class="gauge-value" id="clt-value">0</div>
                    <div class="gauge-unit">°C</div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="oil-press-gauge" data-gauge-id="oil-press" data-variable-hash="598268994" ondblclick="clearGauge('oil-press')">
                    <div class="gauge-label">Oil Press</div>
                    <div class="gauge-value" id="oil-press-value">0</div>
                    <div class="gauge-unit">kPa</div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="map-gauge" data-gauge-id="map" data-variable-hash="1281101952" ondblclick="clearGauge('map')">
                    <div class="gauge-label">MAP</div>
                    <div class="gauge-value" id="map-value">0</div>
                    <div class="gauge-unit">kPa</div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="boost-gauge" data-gauge-id="boost" data-variable-hash="1281101952" ondblclick="clearGauge('boost')">
                    <div class="gauge-label">Boost</div>
                    <div class="gauge-value" id="boost-value">0</div>
                    <div class="gauge-unit">kPa</div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge original-gauge" id="iat-gauge" data-gauge-id="iat" data-variable-hash="417952269" ondblclick="clearGauge('iat')">
                    <div class="gauge-label">Intake Temp</div>
                    <div class="gauge-value" id="iat-value">0</div>
                    <div class="gauge-unit">°C</div>
                </div>
            </div>
        </section>

        <section class="dynamic-gauges-section">
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-1" style="opacity: 0.3;" ondblclick="clearDynamicGauge(1)">
                    <div class="gauge-label" id="dynamic-label-1">Empty</div>
                    <div class="gauge-value" id="dynamic-value-1">--</div>
                    <div class="gauge-unit" id="dynamic-unit-1"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-2" style="opacity: 0.3;" ondblclick="clearDynamicGauge(2)">
                    <div class="gauge-label" id="dynamic-label-2">Empty</div>
                    <div class="gauge-value" id="dynamic-value-2">--</div>
                    <div class="gauge-unit" id="dynamic-unit-2"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-3" style="opacity: 0.3;" ondblclick="clearDynamicGauge(3)">
                    <div class="gauge-label" id="dynamic-label-3">Empty</div>
                    <div class="gauge-value" id="dynamic-value-3">--</div>
                    <div class="gauge-unit" id="dynamic-unit-3"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-4" style="opacity: 0.3;" ondblclick="clearDynamicGauge(4)">
                    <div class="gauge-label" id="dynamic-label-4">Empty</div>
                    <div class="gauge-value" id="dynamic-value-4">--</div>
                    <div class="gauge-unit" id="dynamic-unit-4"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-5" style="opacity: 0.3;" ondblclick="clearDynamicGauge(5)">
                    <div class="gauge-label" id="dynamic-label-5">Empty</div>
                    <div class="gauge-value" id="dynamic-value-5">--</div>
                    <div class="gauge-unit" id="dynamic-unit-5"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-6" style="opacity: 0.3;" ondblclick="clearDynamicGauge(6)">
                    <div class="gauge-label" id="dynamic-label-6">Empty</div>
                    <div class="gauge-value" id="dynamic-value-6">--</div>
                    <div class="gauge-unit" id="dynamic-unit-6"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-7" style="opacity: 0.3;" ondblclick="clearDynamicGauge(7)">
                    <div class="gauge-label" id="dynamic-label-7">Empty</div>
                    <div class="gauge-value" id="dynamic-value-7">--</div>
                    <div class="gauge-unit" id="dynamic-unit-7"></div>
                </div>
            </div>
            <div class="gauge-container">
                <div class="gauge dynamic-gauge" id="dynamic-gauge-8" style="opacity: 0.3;" ondblclick="clearDynamicGauge(8)">
                    <div class="gauge-label" id="dynamic-label-8">Empty</div>
                    <div class="gauge-value" id="dynamic-value-8">--</div>
                    <div class="gauge-unit" id="dynamic-unit-8"></div>
                </div>
            </div>
        </section>

        <section class="shift-light-section">
            <div class="shift-light">
                <div class="shift-light-label">Shift Light</div>
                <div class="shift-light-indicator" id="shift-light-indicator"></div>
            </div>
        </section>

        <section class="stats-section">
            <div class="stat-card">
                <div class="stat-label">System Status</div>
                <div class="stat-value" id="system-status">NORMAL</div>
            </div>
            <div class="stat-card">
                <div class="stat-label">Uptime</div>
                <div class="stat-value" id="uptime">--:--:--</div>
            </div>
        </section>

        <section class="variable-selector-section">
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 5</div>
                <select id="variable-selector-5" class="variable-select" onchange="selectVariable(5)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-5">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 6</div>
                <select id="variable-selector-6" class="variable-select" onchange="selectVariable(6)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-6">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 7</div>
                <select id="variable-selector-7" class="variable-select" onchange="selectVariable(7)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-7">
                    <small>Loading variables...</small>
                </div>
            </div>
            <div class="variable-selector-card">
                <div class="stat-label">Variable Selector 8</div>
                <select id="variable-selector-8" class="variable-select" onchange="selectVariable(8)">
                    <option value="">Pick your poison</option>
                </select>
                <div class="variable-search-info" id="info-8">
                    <small>Loading variables...</small>
                </div>
            </div>
        </section>

        <section class="actions-section">
            <button class="action-button" onclick="alert('Configuration coming soon')">⚙️ Configuration</button>
            <button class="action-button" onclick="loadHealth()">📊 System Health</button>
        </section>
    </main>

    <footer class="app-footer">
        <div class="footer-info">
            <span id="last-update">Last update: --</span>
        </div>
    </footer>

    <script>
        // API Communication - Auto-detect hostname/IP for mobile devices
        const API_BASE = window.location.protocol + '//' + window.location.hostname + ':8080';
        console.log('API Base:', API_BASE);
        
        let isFetching = false; // Prevent concurrent fetches for better performance
        async function fetchData() {
            if (isFetching) return; // Skip if already fetching
            isFetching = true;
            try {
                const response = await fetch(`${API_BASE}/data`);
                const data = await response.json();
                updateGauges(data);
                updateSelectedVariable(data);
                return data;
            } catch (error) {
                console.error('Error fetching data:', error);
                const statusDot = document.getElementById('connection-status')?.querySelector('.status-dot');
                if (statusDot) {
                    statusDot.style.backgroundColor = '#F44336';
                }
                return null;
            } finally {
                isFetching = false;
            }
        }
        
        async function loadHealth() {
            try {
                const response = await fetch(`${API_BASE}/health`);
                const health = await response.json();
                alert(`Uptime: ${health.uptime_formatted}\\nMemory: ${health.memory_usage_percent.toFixed(1)}%\\nCAN: ${health.can_status}`);
            } catch (error) {
                console.error('Error:', error);
            }
        }
        
        function updateGauges(data) {
            if (!data) return;
            
            // All gauge updates are now handled by updateSelectedVariable()
            // which tracks both original and dynamic gauges
            
            // Update shift light
            const shiftLight = document.getElementById('shift-light-indicator');
            if (shiftLight) {
                if (data.shift_light) {
                    shiftLight.classList.add('active');
                    shiftLight.classList.remove('inactive');
                } else {
                    shiftLight.classList.add('inactive');
                    shiftLight.classList.remove('active');
                }
            }
            
            // Update timestamp
            const now = new Date();
            document.getElementById('last-update').textContent = `Last update: ${now.toLocaleTimeString()}`;
            
            // Update connection status
            document.getElementById('connection-status').querySelector('.status-dot').style.backgroundColor = '#4CAF50';
            
            // Update selected variable display
            updateSelectedVariable(data);
        }
        
        function animateValue(element, start, end, duration, decimals) {
            const startTime = performance.now();
            const difference = end - start;
            
            function animate(currentTime) {
                const elapsed = currentTime - startTime;
                const progress = Math.min(elapsed / duration, 1);
                const easeOut = 1 - Math.pow(1 - progress, 3);
                const current = start + (difference * easeOut);
                
                if (decimals) {
                    element.textContent = current.toFixed(1);
                } else {
                    element.textContent = Math.round(current);
                }
                
                if (progress < 1) {
                    requestAnimationFrame(animate);
                } else {
                    element.textContent = decimals ? end.toFixed(1) : Math.round(end);
                }
            }
            
            requestAnimationFrame(animate);
        }
        
        // Start updates
        fetchData();
        // Optimize update interval for better performance
        // Update every 1 second instead of 500ms for mobile performance
        setInterval(fetchData, 1000);
        
        // Load initial health
        fetch(`${API_BASE}/health`)
            .then(r => r.json())
            .then(h => {
                const uptimeEl = document.getElementById('uptime');
                const statusEl = document.getElementById('system-status');
                if (uptimeEl) uptimeEl.textContent = h.uptime_formatted;
                if (statusEl) statusEl.textContent = h.system_state;
            })
            .catch(e => {
                console.error('Error loading health:', e);
            });
        
        // Constants
        const DROPDOWN_COUNT = 8;
        const DROPDOWN_NUMBERS = [1, 2, 3, 4, 5, 6, 7, 8];
        const CHUNK_SIZE = 100;
        const FALLBACK_VARIABLES = {
            '1272048601': { name: 'TPSValue', id: 1272048601, source: 'output' },
            '1699696209': { name: 'RPMValue', id: 1699696209, source: 'output' },
            '-1093429509': { name: 'AFRValue', id: -1093429509, source: 'output' }
        };
        
        // Variable selector functionality - 8 dropdowns with all 845 variables evenly split
        let EPIC_VARIABLES_MAP = {};
        let selectedVariableId = null;
        let selectedDropdown = null;
        
        // Original gauge tracking - maps gauge ID to variable hash (null if cleared)
        const originalGaugeConfig = {
            'rpm': { hash: '1699696209', label: 'RPM', valueEl: 'rpm-value', unit: '' },
            'tps': { hash: '1272048601', label: 'Throttle', valueEl: 'tps-value', unit: '%' },
            'afr': { hash: '-1093429509', label: 'AFR', valueEl: 'afr-value', unit: '' },
            'clt': { hash: '417946098', label: 'Coolant', valueEl: 'clt-value', unit: '°C' },
            'oil-press': { hash: '598268994', label: 'Oil Press', valueEl: 'oil-press-value', unit: 'kPa' },
            'map': { hash: '1281101952', label: 'MAP', valueEl: 'map-value', unit: 'kPa' },
            'boost': { hash: '1281101952', label: 'Boost', valueEl: 'boost-value', unit: 'kPa' },
            'iat': { hash: '417952269', label: 'Intake Temp', valueEl: 'iat-value', unit: '°C' }
        };
        let originalGauges = {
            'rpm': '1699696209',    // Start with default variables
            'tps': '1272048601',
            'afr': '-1093429509',
            'clt': '417946098',
            'oil-press': '598268994',
            'map': '1281101952',
            'boost': '1281101952',
            'iat': '417952269'
        };
        
        // Dynamic gauge tracking - maps gauge index to variable info
        let dynamicGauges = [null, null, null, null, null, null, null, null]; // 8 gauge slots
        const MAX_DYNAMIC_GAUGES = 8;
        
        // Load all variables from variables.json and split across 8 dropdowns
        async function loadVariablesMap() {
            try {
                // Reduced logging for performance - only log errors
                const response = await fetch(`${API_BASE}/variables.json`);
                
                if (response.ok) {
                    const varsData = await response.json();
                    
                    if (Array.isArray(varsData)) {
                        // Process in chunks to avoid blocking the UI
                        for (let i = 0; i < varsData.length; i += CHUNK_SIZE) {
                            const chunk = varsData.slice(i, i + CHUNK_SIZE);
                            chunk.forEach(v => {
                                if (v && v.source === 'output') {
                                    EPIC_VARIABLES_MAP[String(v.hash)] = {
                                        name: v.name,
                                        id: v.hash,
                                        source: v.source
                                    };
                                }
                            });
                            // Yield to browser between chunks
                            if (i + CHUNK_SIZE < varsData.length) {
                                await new Promise(resolve => setTimeout(resolve, 0));
                            }
                        }
                        console.log('Loaded', Object.keys(EPIC_VARIABLES_MAP).length, 'variables');
                    } else {
                        console.error('variables.json is not an array');
                        EPIC_VARIABLES_MAP = { ...FALLBACK_VARIABLES };
                    }
                } else {
                    console.error('Failed to fetch variables.json:', response.status);
                    EPIC_VARIABLES_MAP = { ...FALLBACK_VARIABLES };
                }
            } catch (e) {
                console.error('Error loading variables.json:', e);
                EPIC_VARIABLES_MAP = { ...FALLBACK_VARIABLES };
            }
            
            // Categorize variables by importance for tuning
            const criticalVars = [];  // Dropdown 1: Most important - Safety & Primary Tuning
            const importantVars = []; // Dropdown 2: Important - Performance
            const usefulVars = [];     // Dropdown 3: Useful - Secondary metrics
            const otherVars = [];      // Dropdown 4: Less critical
            
            const criticalKeywords = ['AFR', 'Lambda', 'Knock', 'Det', 'EGT', 'Oil', 'Coolant', 'CLT', 'Timing', 'Advance', 'MAP', 'Boost', 'Pressure'];
            const importantKeywords = ['RPM', 'TPS', 'Throttle', 'Fuel', 'Inject', 'IAT', 'Intake', 'VE', 'Volumetric', 'Load', 'Speed'];
            const usefulKeywords = ['Correction', 'Trim', 'EGO', 'Sensor', 'Temp', 'Voltage'];
            
            // Process variables and categorize efficiently
            const hashes = Object.keys(EPIC_VARIABLES_MAP);
            const criticalUpper = criticalKeywords.map(k => k.toUpperCase());
            const importantUpper = importantKeywords.map(k => k.toUpperCase());
            const usefulUpper = usefulKeywords.map(k => k.toUpperCase());
            
            for (const hash of hashes) {
                const varInfo = EPIC_VARIABLES_MAP[hash];
                if (!varInfo || !varInfo.name) continue;
                
                const nameUpper = varInfo.name.toUpperCase();
                
                if (criticalUpper.some(k => nameUpper.includes(k))) {
                    criticalVars.push(hash);
                } else if (importantUpper.some(k => nameUpper.includes(k))) {
                    importantVars.push(hash);
                } else if (usefulUpper.some(k => nameUpper.includes(k))) {
                    usefulVars.push(hash);
                } else {
                    otherVars.push(hash);
                }
            }
            
            // Sort each category alphabetically
            criticalVars.sort((a, b) => EPIC_VARIABLES_MAP[a].name.localeCompare(EPIC_VARIABLES_MAP[b].name));
            importantVars.sort((a, b) => EPIC_VARIABLES_MAP[a].name.localeCompare(EPIC_VARIABLES_MAP[b].name));
            usefulVars.sort((a, b) => EPIC_VARIABLES_MAP[a].name.localeCompare(EPIC_VARIABLES_MAP[b].name));
            otherVars.sort((a, b) => EPIC_VARIABLES_MAP[a].name.localeCompare(EPIC_VARIABLES_MAP[b].name));
            
            // Get all variables that weren't categorized - these go to dropdowns 5-8
            const categorizedHashes = new Set([...criticalVars, ...importantVars, ...usefulVars, ...otherVars]);
            const uncategorizedVars = Object.keys(EPIC_VARIABLES_MAP).filter(hash => !categorizedHashes.has(hash));
            uncategorizedVars.sort((a, b) => EPIC_VARIABLES_MAP[a].name.localeCompare(EPIC_VARIABLES_MAP[b].name));
            
            // Split critical variables between dropdowns 1 and 2
            const criticalHalf = Math.ceil(criticalVars.length / 2);
            const critical1 = criticalVars.slice(0, criticalHalf);
            const critical2 = criticalVars.slice(criticalHalf);
            
            // Split important variables between dropdowns 3 and 4
            const importantHalf = Math.ceil(importantVars.length / 2);
            const important1 = importantVars.slice(0, importantHalf);
            const important2 = importantVars.slice(importantHalf);
            
            // Combine all remaining variables (useful + other + uncategorized) and split across dropdowns 5-8
            const remainingVars = [...usefulVars, ...otherVars, ...uncategorizedVars];
            const varsPerRemainingDropdown = Math.ceil(remainingVars.length / 4); // For dropdowns 5-8
            
            // Populate dropdowns 1-4 with split critical and important variables
            populateDropdown(1, critical1);
            const info1 = document.getElementById('info-1');
            if (info1) info1.textContent = `${critical1.length} critical variables`;
            
            populateDropdown(2, critical2);
            const info2 = document.getElementById('info-2');
            if (info2) info2.textContent = `${critical2.length} critical variables`;
            
            populateDropdown(3, important1);
            const info3 = document.getElementById('info-3');
            if (info3) info3.textContent = `${important1.length} important variables`;
            
            populateDropdown(4, important2);
            const info4 = document.getElementById('info-4');
            if (info4) info4.textContent = `${important2.length} important variables`;
            
            // Distribute remaining variables evenly across dropdowns 5-8
            for (let i = 4; i < DROPDOWN_COUNT; i++) {
                const startIdx = (i - 4) * varsPerRemainingDropdown;
                const endIdx = Math.min(startIdx + varsPerRemainingDropdown, remainingVars.length);
                const varsForDropdown = remainingVars.slice(startIdx, endIdx);
                
                populateDropdown(i + 1, varsForDropdown);
                const infoEl = document.getElementById(`info-${i + 1}`);
                if (infoEl) infoEl.textContent = `${varsForDropdown.length} variables`;
            }
            
            console.log(`Dropdowns ready: ${criticalVars.length} critical, ${importantVars.length} important`);
        }
        
        function populateDropdown(dropdownNum, variableHashes) {
            // Input validation
            if (!dropdownNum || dropdownNum < 1 || dropdownNum > DROPDOWN_COUNT) {
                console.error(`Invalid dropdown number: ${dropdownNum}`);
                return;
            }
            
            if (!Array.isArray(variableHashes)) {
                console.error('variableHashes must be an array');
                return;
            }
            
            const selector = document.getElementById(`variable-selector-${dropdownNum}`);
            if (!selector) {
                console.error(`Dropdown selector ${dropdownNum} not found`);
                return;
            }
            
            variableHashes.forEach(hash => {
                const varInfo = EPIC_VARIABLES_MAP[hash];
                if (!varInfo) {
                    console.warn(`Variable info not found for hash: ${hash}`);
                    return;
                }
                
                const option = document.createElement('option');
                option.value = hash;
                option.textContent = `${varInfo.name} (ID: ${varInfo.id})`;
                selector.appendChild(option);
            });
        }
        
        function selectVariable(dropdownNum) {
            // Input validation
            if (!dropdownNum || dropdownNum < 1 || dropdownNum > DROPDOWN_COUNT) {
                console.error(`Invalid dropdown number: ${dropdownNum}`);
                return;
            }
            
            const selector = document.getElementById(`variable-selector-${dropdownNum}`);
            if (!selector) {
                console.error(`Dropdown selector ${dropdownNum} not found`);
                return;
            }
            
            selectedVariableId = selector.value;
            selectedDropdown = dropdownNum;
            
            // Clear other dropdowns if a variable is selected
            if (selectedVariableId) {
                DROPDOWN_NUMBERS.forEach(num => {
                    if (num !== dropdownNum) {
                        const otherSelector = document.getElementById(`variable-selector-${num}`);
                        if (otherSelector && otherSelector.value === selectedVariableId) {
                            otherSelector.value = '';
                            removeVariableFromAnyGauge(selectedVariableId);
                        }
                    }
                });
            }
            
            if (selectedVariableId && EPIC_VARIABLES_MAP[selectedVariableId]) {
                const varInfo = EPIC_VARIABLES_MAP[selectedVariableId];
                
                // Try to add to first available gauge (original first, then dynamic)
                addVariableToGauge(selectedVariableId, varInfo);
            } else {
                // If empty selection, variable was removed from dropdown
                // Gauge will remain populated until another variable is selected
            }
        }
        
        function addVariableToGauge(varHash, varInfo) {
            // Check if variable is already in any gauge (original or dynamic)
            let existingGaugeId = Object.keys(originalGauges).find(id => originalGauges[id] === varHash);
            if (existingGaugeId) {
                // Already in an original gauge, don't duplicate
                return;
            }
            
            let existingIndex = dynamicGauges.findIndex(g => g && g.hash === varHash);
            if (existingIndex !== -1) {
                // Already in a dynamic gauge, don't duplicate
                return;
            }
            
            // Try to find first empty original gauge (left to right)
            const originalGaugeOrder = ['rpm', 'tps', 'afr', 'clt', 'oil-press', 'map', 'boost', 'iat'];
            let emptyOriginalGaugeId = originalGaugeOrder.find(id => !originalGauges[id]);
            
            if (emptyOriginalGaugeId) {
                // Populate empty original gauge
                originalGauges[emptyOriginalGaugeId] = varHash;
                
                const config = originalGaugeConfig[emptyOriginalGaugeId];
                if (!config) {
                    console.error(`No config for gauge: ${emptyOriginalGaugeId}`);
                    return;
                }
                
                const gaugeElement = document.getElementById(`${emptyOriginalGaugeId}-gauge`);
                if (!gaugeElement) {
                    console.error(`Gauge element not found: ${emptyOriginalGaugeId}-gauge`);
                    return;
                }
                
                const labelElement = gaugeElement.querySelector('.gauge-label');
                const valueElement = document.getElementById(config.valueEl);
                const unitElement = gaugeElement.querySelector('.gauge-unit');
                
                if (!labelElement || !valueElement || !unitElement) {
                    console.error(`Gauge elements not found for: ${emptyOriginalGaugeId}`);
                    return;
                }
                
                // Update gauge display
                labelElement.textContent = varInfo.name;
                valueElement.textContent = '--';
                unitElement.textContent = config.unit || '';
                gaugeElement.setAttribute('data-variable-hash', varHash);
                return;
            }
            
            // No empty original gauge, try dynamic gauges
            let emptyIndex = dynamicGauges.findIndex(g => g === null);
            if (emptyIndex === -1) {
                // All gauges full, use first dynamic one (leftmost)
                emptyIndex = 0;
                // Remove existing variable from this slot
                if (dynamicGauges[0]) {
                    removeVariableFromDynamicGauge(0);
                }
            }
            
            // Populate the dynamic gauge
            const gaugeIndex = emptyIndex + 1;
            const gaugeElement = document.getElementById(`dynamic-gauge-${gaugeIndex}`);
            const labelElement = document.getElementById(`dynamic-label-${gaugeIndex}`);
            const valueElement = document.getElementById(`dynamic-value-${gaugeIndex}`);
            const unitElement = document.getElementById(`dynamic-unit-${gaugeIndex}`);
            
            if (!gaugeElement || !labelElement || !valueElement || !unitElement) {
                console.error(`Dynamic gauge elements not found for index: ${gaugeIndex}`);
                return;
            }
            
            // Store variable info
            dynamicGauges[emptyIndex] = {
                hash: varHash,
                name: varInfo.name,
                id: varInfo.id
            };
            
            // Update gauge display
            labelElement.textContent = varInfo.name;
            valueElement.textContent = '--';
            unitElement.textContent = '';
            gaugeElement.classList.remove('empty');
            gaugeElement.classList.add('populated');
            gaugeElement.style.opacity = '1.0';
        }
        
        function removeVariableFromAnyGauge(varHash) {
            // Check original gauges
            let gaugeId = Object.keys(originalGauges).find(id => originalGauges[id] === varHash);
            if (gaugeId) {
                clearGauge(gaugeId);
                return;
            }
            
            // Check dynamic gauges
            const index = dynamicGauges.findIndex(g => g && g.hash === varHash);
            if (index !== -1) {
                clearDynamicGauge(index + 1);
            }
        }
        
        function removeVariableFromDynamicGauge(index) {
            if (dynamicGauges[index]) {
                const varHash = dynamicGauges[index].hash;
                clearDynamicGauge(index + 1);
                
                // Clear from all dropdowns
                DROPDOWN_NUMBERS.forEach(num => {
                    const selector = document.getElementById(`variable-selector-${num}`);
                    if (selector && selector.value === varHash) {
                        selector.value = '';
                    }
                });
            }
        }
        
        function clearGauge(gaugeId) {
            if (!originalGaugeConfig[gaugeId]) {
                console.error(`No config for gauge: ${gaugeId}`);
                return;
            }
            
            const config = originalGaugeConfig[gaugeId];
            const gaugeElement = document.getElementById(`${gaugeId}-gauge`);
            if (!gaugeElement) {
                console.error(`Gauge element not found: ${gaugeId}-gauge`);
                return;
            }
            
            const labelElement = gaugeElement.querySelector('.gauge-label');
            const valueElement = document.getElementById(config.valueEl);
            const unitElement = gaugeElement.querySelector('.gauge-unit');
            
            if (!labelElement || !valueElement || !unitElement) {
                console.error(`Gauge elements not found for: ${gaugeId}`);
                return;
            }
            
            // Restore default label or show "Empty"
            labelElement.textContent = config.label;
            valueElement.textContent = '--';
            unitElement.textContent = config.unit || '';
            
            // Remove variable tracking
            const varHash = originalGauges[gaugeId];
            originalGauges[gaugeId] = null;
            
            // Clear from all dropdowns
            if (varHash) {
                DROPDOWN_NUMBERS.forEach(num => {
                    const selector = document.getElementById(`variable-selector-${num}`);
                    if (selector && selector.value === varHash) {
                        selector.value = '';
                    }
                });
            }
        }
        
        function clearDynamicGauge(gaugeIndex) {
            if (gaugeIndex < 1 || gaugeIndex > DROPDOWN_COUNT) {
                console.error(`Invalid gauge index: ${gaugeIndex}`);
                return;
            }
            
            const index = gaugeIndex - 1;
            if (!dynamicGauges[index]) return;
            
            const varHash = dynamicGauges[index].hash;
            const gaugeElement = document.getElementById(`dynamic-gauge-${gaugeIndex}`);
            const labelElement = document.getElementById(`dynamic-label-${gaugeIndex}`);
            const valueElement = document.getElementById(`dynamic-value-${gaugeIndex}`);
            const unitElement = document.getElementById(`dynamic-unit-${gaugeIndex}`);
            
            if (!gaugeElement || !labelElement || !valueElement || !unitElement) {
                console.error(`Dynamic gauge elements not found for index: ${gaugeIndex}`);
                return;
            }
            
            // Clear gauge
            labelElement.textContent = 'Empty';
            valueElement.textContent = '--';
            unitElement.textContent = '';
            gaugeElement.classList.remove('populated');
            gaugeElement.classList.add('empty');
            gaugeElement.style.opacity = '0.3';
            
            // Remove from tracking
            dynamicGauges[index] = null;
            
            // Clear from all dropdowns
            DROPDOWN_NUMBERS.forEach(num => {
                const selector = document.getElementById(`variable-selector-${num}`);
                if (selector && selector.value === varHash) {
                    selector.value = '';
                }
            });
        }
        
        function updateSelectedVariable(data) {
            // Update all populated original gauges
            Object.keys(originalGauges).forEach(gaugeId => {
                const varHash = originalGauges[gaugeId];
                if (varHash) {
                    const config = originalGaugeConfig[gaugeId];
                    const valueElement = document.getElementById(config.valueEl);
                    
                    if (valueElement) {
                        let value = getVariableValue(varHash, data);
                        
                        if (value !== null) {
                            const currentValue = parseFloat(valueElement.textContent) || 0;
                            const hasDecimals = Math.abs(value) < 1000 && value % 1 !== 0;
                            animateValue(valueElement, currentValue, value, 300, hasDecimals);
                        } else {
                            valueElement.textContent = 'N/A';
                        }
                    }
                }
            });
            
            // Update all populated dynamic gauges
            dynamicGauges.forEach((gaugeVar, index) => {
                if (gaugeVar) {
                    const gaugeIndex = index + 1;
                    const valueElement = document.getElementById(`dynamic-value-${gaugeIndex}`);
                    
                    if (valueElement) {
                        let value = getVariableValue(gaugeVar.hash, data);
                        
                        if (value !== null) {
                            const currentValue = parseFloat(valueElement.textContent) || 0;
                            const hasDecimals = Math.abs(value) < 1000 && value % 1 !== 0;
                            animateValue(valueElement, currentValue, value, 300, hasDecimals);
                        } else {
                            valueElement.textContent = 'N/A';
                        }
                    }
                }
            });
        }
        
        function getVariableValue(varHash, data) {
            const hashNum = parseInt(varHash);
            
            // Map known variable IDs to data fields
            if (hashNum === 1272048601) return data.tps;
            else if (hashNum === 1699696209) return data.rpm;
            else if (hashNum === -1093429509) return data.afr;
            else if (hashNum === 1281101952) return data.map;
            else if (hashNum === 598268994) return data.oil_press;
            else if (hashNum === 417946098) return data.clt;
            else if (hashNum === 417952269) return data.iat;
            
            // For other variables, try to find in data object by variable name
            const varInfo = EPIC_VARIABLES_MAP[varHash];
            if (!varInfo || !data) {
                return null;
            }
            
            // Try common variations
            const name = varInfo.name.toLowerCase();
            if (name.includes('tps') && name.includes('value')) return data.tps;
            if (name.includes('rpm') && name.includes('value')) return data.rpm;
            if (name.includes('afr') && name.includes('value')) return data.afr;
            if (name.includes('map') && name.includes('value')) return data.map;
            if (name.includes('oil') && name.includes('press')) return data.oil_press;
            if (name.includes('clt') || name.includes('coolant')) return data.clt;
            if (name.includes('iat') || name.includes('intake')) return data.iat;
            
            return null;
        }
        
        // Initialize app - load variables in background, don't block
        console.log('myepicEFI starting...');
        
        // Load variables asynchronously - app works even if this fails
        loadVariablesMap().catch(err => {
            console.error('Variables load failed:', err);
        });
        
        // Debug scroll height (non-blocking)
        if (window.innerWidth < 768) { // Only on mobile
            window.addEventListener('load', () => {
                setTimeout(() => {
                    const main = document.querySelector('.app-main');
                    const info = {
                        scrollHeight: document.body.scrollHeight,
                        innerHeight: window.innerHeight,
                        canScroll: document.body.scrollHeight > window.innerHeight
                    };
                    if (!info.canScroll) {
                        console.warn('Page might not scroll:', info);
                    }
                }, 2000);
            });
        }
    </script>
</body>
</html>"""
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(html.encode())
    
    def send_data(self):
        """Send real-time data as JSON"""
        # Simulate data changes with variable RPM for shift light testing
        self.request_count += 1
        current_time = time.time()
        
        # Create RPM pattern: slow ramp up to 7000, then drop back
        # This allows testing shift light activation (>6000 RPM)
        rpm_cycle = (current_time * 0.3) % 20  # 20 second cycle
        if rpm_cycle < 10:
            # Ramp up from 3000 to 7000 over 10 seconds
            self.rpm_value = 3000 + int(4000 * (rpm_cycle / 10))
        else:
            # Ramp down from 7000 to 3000 over 10 seconds
            self.rpm_value = 7000 - int(4000 * ((rpm_cycle - 10) / 10))
        
        # TPS follows RPM pattern
        self.tps_value = 20.0 + (self.rpm_value / 7000.0) * 60.0
        # AFR varies slightly
        self.afr_value = 14.0 + 0.5 + (time.time() % 1) * 0.5
        
        # New sensor values - realistic patterns
        # Coolant Temp (CLT): Warm up from 20°C to 95°C, then stabilize
        clt_cycle = (current_time * 0.05) % 200  # 200 second warm-up cycle
        self.clt_value = min(20.0 + (clt_cycle * 0.375), 95.0)
        
        # Intake Temp (IAT): Varies with RPM and ambient (20-50°C)
        self.iat_value = 25.0 + (self.rpm_value / 7000.0) * 25.0 + (time.time() % 2) * 2.0
        
        # MAP: Varies with throttle (40-150 kPa)
        self.map_value = 50.0 + (self.tps_value / 100.0) * 100.0 + (time.time() % 3) * 5.0
        
        # Boost: Calculated from MAP - atmospheric pressure (101.3 kPa)
        # Boost = MAP - 101.3 (positive = boost, negative = vacuum)
        self.boost_value = self.map_value - 101.3
        
        # Oil Pressure: Correlates with RPM (30-80 psi / 207-552 kPa)
        self.oil_press_value = 30.0 + (self.rpm_value / 7000.0) * 50.0
        
        # Shift light activates when RPM > 6000
        shift_light_active = self.rpm_value > 6000
        
        data = {
            "tps": round(self.tps_value, 2),
            "rpm": int(self.rpm_value),
            "afr": round(self.afr_value, 2),
            "clt": round(self.clt_value, 1),  # Coolant Temp in °C
            "iat": round(self.iat_value, 1),  # Intake Air Temp in °C
            "map": round(self.map_value, 1),  # MAP in kPa
            "boost": round(self.boost_value, 1),  # Boost in kPa (can be negative for vacuum)
            "oil_press": round(self.oil_press_value, 1),  # Oil Pressure in kPa
            "shift_light": shift_light_active,
            "timestamp": int(time.time() * 1000)
        }
        
        self.send_json_response(data)
    
    def send_health(self):
        """Send system health metrics"""
        uptime = int(time.time() - self.start_time)
        
        health = {
            "uptime_seconds": uptime,
            "uptime_formatted": f"{uptime // 3600}h {(uptime % 3600) // 60}m {uptime % 60}s",
            "free_heap": 150000,
            "total_heap": 520000,
            "memory_usage_percent": 71.2,
            "can_status": "ACTIVE",
            "sd_status": "ACTIVE",
            "wifi_status": "CONNECTED",
            "system_state": "NORMAL",
            "can_errors": 0,
            "sd_errors": 0
        }
        
        self.send_json_response(health)
    
    def send_config(self):
        """Send current configuration"""
        config = {
            "ecu_id": 1,
            "can_speed": 500,
            "request_interval_ms": 50,
            "max_pending_requests": 16,
            "shift_light_rpm": 6000,
            "wifi_ssid": "EPIC_CAN_LOGGER",
            "wifi_password": "password123",
            "wifi_channel": 1,
            "wifi_hidden": 0
        }
        
        self.send_json_response(config)
    
    def handle_config_save(self):
        """Handle configuration save"""
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        try:
            # Parse form data or JSON
            data = json.loads(post_data.decode('utf-8'))
            print(f"Received config: {data}")
            
            # In real implementation, validate and save
            response = {"status": "success", "message": "Configuration saved"}
            self.send_json_response(response)
        except Exception as e:
            response = {"status": "error", "message": str(e)}
            self.send_json_response(response, status=400)
    
    def send_json_response(self, data, status=200):
        """Send JSON response"""
        self.send_response(status)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())
    
    def send_epic_variables_map(self):
        """Serve the EPIC variables map JavaScript file"""
        try:
            from pathlib import Path
            map_path = Path('mobile_app/epic_variables_map.js')
            if map_path.exists():
                with open(map_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                self.send_response(200)
                self.send_header('Content-type', 'application/javascript')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(content.encode())
            else:
                self.send_error(404, "File not found")
        except Exception as e:
            self.send_error(500, f"Error: {str(e)}")
    
    def send_manifest(self):
        """Send PWA manifest.json"""
        from pathlib import Path
        script_dir = Path(__file__).parent
        manifest_path = script_dir.parent / "manifest.json"
        
        if manifest_path.exists():
            with open(manifest_path, 'r', encoding='utf-8') as f:
                manifest = f.read()
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(manifest.encode('utf-8'))
        else:
            self.send_error(404, "Manifest not found")
    
    def send_icon_file(self, icon_path):
        """Send icon file (PNG)"""
        from pathlib import Path
        import os
        
        script_dir = Path(__file__).parent
        # Remove leading slash and handle paths
        icon_path_clean = icon_path.lstrip('/')
        
        # Try images/icons directory first
        icon_file = script_dir.parent / icon_path_clean
        if not icon_file.exists():
            # Try alternative paths
            if 'icon-32' in icon_path_clean or 'favicon' in icon_path_clean:
                # For favicon, try icon-96 as fallback
                icon_file = script_dir.parent / "images" / "icons" / "icon-96.png"
        
        if icon_file.exists() and icon_file.suffix.lower() == '.png':
            try:
                with open(icon_file, 'rb') as f:
                    icon_data = f.read()
                self.send_response(200)
                self.send_header('Content-type', 'image/png')
                self.send_header('Cache-Control', 'public, max-age=86400')  # Cache for 1 day
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(icon_data)
            except Exception as e:
                self.send_error(500, f"Error reading icon: {str(e)}")
        else:
            self.send_error(404, "Icon not found")
    
    def send_variables_json(self):
        """Serve the EPIC variables.json file"""
        try:
            from pathlib import Path
            import os
            
            # Try multiple possible paths
            possible_paths = [
                # From project root when running from mobile_app/python
                Path(__file__).parent.parent.parent / 'keyboard_basic1' / 'variables.json',
                # From current working directory
                Path('keyboard_basic1') / 'variables.json',
                # Absolute path from Downloads
                Path(r'c:\Users\user1\Downloads\epicefi_fw-master\epicefi_fw-master\epic_can_bus\variables.json'),
                # From project root relative to script
                Path(__file__).parent.parent / 'keyboard_basic1' / 'variables.json',
            ]
            
            vars_path = None
            for path in possible_paths:
                if path.exists():
                    vars_path = path
                    break
            
            if vars_path and vars_path.exists():
                with open(vars_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(content.encode())
            else:
                # Return empty array instead of error so JavaScript doesn't break
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(b'[]')
                print(f"WARNING: variables.json not found. Tried paths: {[str(p) for p in possible_paths]}")
        except Exception as e:
            # Return empty array on error too
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(b'[]')
            print(f"ERROR serving variables.json: {str(e)}")


def run_server(port=8080):
    """Run the API simulator server"""
    server_address = ('', port)
    httpd = HTTPServer(server_address, ESP32APIHandler)
    
    # Get network IP for mobile access
    import socket
    try:
        # Get local IP address
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        network_ip = s.getsockname()[0]
        s.close()
    except:
        network_ip = "Check ipconfig for your IP"
    
    print(f"=" * 60)
    print(f"myepicEFI API Simulator")
    print(f"=" * 60)
    print(f"Server running on http://localhost:{port}")
    print(f"\n🌐 Mobile Access:")
    print(f"  Network IP: http://{network_ip}:{port}")
    print(f"\nAvailable endpoints:")
    print(f"  GET  http://localhost:{port}/          - Mobile dashboard")
    print(f"  GET  http://localhost:{port}/data      - Real-time data (JSON)")
    print(f"  GET  http://localhost:{port}/health    - Health metrics (JSON)")
    print(f"  GET  http://localhost:{port}/config    - Configuration (JSON)")
    print(f"  POST http://localhost:{port}/config/save - Save config")
    print(f"\n📱 To test on mobile:")
    print(f"  1. Connect mobile to same WiFi network")
    print(f"  2. Open browser and go to: http://{network_ip}:{port}")
    print(f"\nPress Ctrl+C to stop")
    print(f"=" * 60)
    
    # Give browser time to connect if opened automatically
    time.sleep(0.5)
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\nShutting down server...")
        httpd.server_close()
        print("Server stopped.")


if __name__ == '__main__':
    run_server()

