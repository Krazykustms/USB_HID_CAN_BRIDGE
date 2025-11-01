/**
 * Main Application Logic
 * Coordinates API communication and UI updates
 */

// Global state
let appState = {
    connected: false,
    data: null,
    health: null,
    config: null
};

/**
 * Initialize application
 */
async function initApp() {
    console.log('Initializing myepicEFI mobile app...');
    
    // Check if we're in simulator mode (localhost)
    const isSimulator = window.location.hostname === 'localhost' || 
                       window.location.hostname === '127.0.0.1';
    
    if (isSimulator) {
        api.baseUrl = `http://${window.location.hostname}:8080`;
        console.log('Running in simulator mode');
    }
    
    // Check connection
    try {
        appState.connected = await api.checkConnection();
        uiUpdater.updateConnectionStatus(appState.connected);
    } catch (error) {
        console.error('Initial connection check failed:', error);
        appState.connected = false;
        uiUpdater.updateConnectionStatus(false);
    }
    
    // Load initial data
    await loadHealth();
    await loadConfig();
    
    // Start real-time updates
    startDataUpdates();
    
    // Periodic health updates
    setInterval(loadHealth, 10000); // Every 10 seconds
    
    console.log('App initialized');
}

/**
 * Start real-time data updates
 */
function startDataUpdates() {
    api.startUpdates((data, error) => {
        if (error) {
            appState.connected = false;
            uiUpdater.updateConnectionStatus(false);
            uiUpdater.showError('Connection lost');
            return;
        }
        
        appState.connected = true;
        appState.data = data;
        uiUpdater.updateConnectionStatus(true);
        uiUpdater.updateGauges(data);
        uiUpdater.updateLastUpdateTime();
    });
}

/**
 * Load health metrics
 */
async function loadHealth() {
    try {
        appState.health = await api.getHealth();
        uiUpdater.updateSystemStatus(appState.health);
        uiUpdater.updateUptime(appState.health.uptime_formatted);
    } catch (error) {
        console.error('Failed to load health:', error);
    }
}

/**
 * Load configuration
 */
async function loadConfig() {
    try {
        appState.config = await api.getConfig();
        // Configuration can be displayed in modal when needed
    } catch (error) {
        console.error('Failed to load config:', error);
    }
}

/**
 * Open configuration modal
 */
async function openConfig() {
    const modal = document.getElementById('config-modal');
    const content = document.getElementById('config-content');
    
    if (!appState.config) {
        await loadConfig();
    }
    
    if (appState.config) {
        content.innerHTML = generateConfigForm(appState.config);
        modal.style.display = 'block';
    }
}

/**
 * Generate configuration form
 */
function generateConfigForm(config) {
    return `
        <form id="config-form" onsubmit="saveConfig(event)">
            <div class="form-group">
                <label for="ecu_id">ECU ID</label>
                <input type="number" id="ecu_id" name="ecu_id" value="${config.ecu_id}" min="0" max="255" required>
            </div>
            
            <div class="form-group">
                <label for="can_speed">CAN Speed (kbps)</label>
                <select id="can_speed" name="can_speed" required>
                    <option value="125" ${config.can_speed === 125 ? 'selected' : ''}>125</option>
                    <option value="250" ${config.can_speed === 250 ? 'selected' : ''}>250</option>
                    <option value="500" ${config.can_speed === 500 ? 'selected' : ''}>500</option>
                    <option value="1000" ${config.can_speed === 1000 ? 'selected' : ''}>1000</option>
                </select>
            </div>
            
            <div class="form-group">
                <label for="request_interval_ms">Request Interval (ms)</label>
                <input type="number" id="request_interval_ms" name="request_interval_ms" 
                       value="${config.request_interval_ms}" min="10" max="1000" required>
            </div>
            
            <div class="form-group">
                <label for="shift_light_rpm">Shift Light RPM</label>
                <input type="number" id="shift_light_rpm" name="shift_light_rpm" 
                       value="${config.shift_light_rpm}" min="1000" max="15000" required>
            </div>
            
            <div class="form-group">
                <label for="wifi_ssid">WiFi SSID</label>
                <input type="text" id="wifi_ssid" name="wifi_ssid" value="${config.wifi_ssid}" required>
            </div>
            
            <div class="form-actions">
                <button type="submit" class="btn-primary">Save</button>
                <button type="button" class="btn-secondary" onclick="closeModal('config-modal')">Cancel</button>
            </div>
        </form>
    `;
}

/**
 * Save configuration
 */
async function saveConfig(event) {
    event.preventDefault();
    
    const formData = new FormData(event.target);
    const config = {};
    
    for (let [key, value] of formData.entries()) {
        const numValue = parseInt(value);
        config[key] = isNaN(numValue) ? value : numValue;
    }
    
    try {
        const result = await api.saveConfig(config);
        uiUpdater.showSuccess('Configuration saved successfully');
        await loadConfig(); // Reload config
        closeModal('config-modal');
    } catch (error) {
        uiUpdater.showError('Failed to save configuration');
    }
}

/**
 * Open health modal
 */
async function openHealth() {
    await loadHealth();
    
    if (appState.health) {
        const healthHtml = `
            <div class="health-info">
                <div class="health-item">
                    <span class="health-label">Uptime:</span>
                    <span class="health-value">${appState.health.uptime_formatted}</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Memory Usage:</span>
                    <span class="health-value">${appState.health.memory_usage_percent.toFixed(1)}%</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Free Heap:</span>
                    <span class="health-value">${(appState.health.free_heap / 1024).toFixed(0)} KB</span>
                </div>
                <div class="health-item">
                    <span class="health-label">CAN Status:</span>
                    <span class="health-value">${appState.health.can_status}</span>
                </div>
                <div class="health-item">
                    <span class="health-label">SD Status:</span>
                    <span class="health-value">${appState.health.sd_status}</span>
                </div>
                <div class="health-item">
                    <span class="health-label">WiFi Status:</span>
                    <span class="health-value">${appState.health.wifi_status}</span>
                </div>
                <div class="health-item">
                    <span class="health-label">System State:</span>
                    <span class="health-value">${appState.health.system_state}</span>
                </div>
            </div>
        `;
        
        // Create modal dynamically or reuse config modal
        alert(healthHtml); // Simple implementation - can be enhanced with proper modal
    }
}

/**
 * Export logs
 */
async function exportLogs() {
    uiUpdater.showError('Log export feature coming soon');
    // TODO: Implement log export via SD card access or web API
}

/**
 * Close modal
 */
function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.display = 'none';
    }
}

/**
 * Close modal when clicking outside
 */
window.onclick = function(event) {
    const modals = document.getElementsByClassName('modal');
    for (let modal of modals) {
        if (event.target === modal) {
            modal.style.display = 'none';
        }
    }
}

// Initialize app when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initApp);
} else {
    initApp();
}

