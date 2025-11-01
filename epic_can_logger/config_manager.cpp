// Configuration Manager Implementation
// Uses ESP32 Preferences (EEPROM) for persistent storage

#include "config_manager.h"
#include <Preferences.h>
#include <string.h>

// Debug support
#ifndef DEBUG_PRINT
  #define DEBUG_PRINT(...)
#endif

// Singleton configuration instance
static SystemConfig currentConfig;
static bool configLoaded = false;

// Preferences object for EEPROM access
static Preferences prefs;

// Calculate checksum for configuration validation
uint8_t configCalculateChecksum(const SystemConfig* config) {
    uint8_t sum = 0;
    const uint8_t* data = (const uint8_t*)config;
    size_t dataSize = sizeof(SystemConfig) - 1;  // Exclude checksum field itself
    
    for (size_t i = 0; i < dataSize; i++) {
        sum ^= data[i];
    }
    return sum;
}

// Initialize configuration system
bool configInit() {
    if (!prefs.begin(CONFIG_NAMESPACE, false)) {
        DEBUG_PRINT("ERROR: Failed to initialize configuration storage\n");
        return false;
    }
    return true;
}

// Load configuration from EEPROM
bool configLoad(SystemConfig* config) {
    if (!config) {
        return false;
    }
    
    // Load all configuration values
    config->ecu_id = prefs.getUChar("ecu_id", CONFIG_DEFAULT_ECU_ID);
    config->can_speed_kbps = prefs.getUShort("can_speed", CONFIG_DEFAULT_CAN_SPEED);
    config->var_request_interval_ms = prefs.getUShort("req_int", CONFIG_DEFAULT_REQUEST_INTERVAL);
    config->max_pending_requests = prefs.getUChar("max_pend", CONFIG_DEFAULT_MAX_PENDING);
    config->shift_light_rpm_threshold = prefs.getUChar("shift_rpm", CONFIG_DEFAULT_SHIFT_LIGHT_RPM);
    config->debug_enabled = prefs.getUChar("debug", 0);
    config->log_flush_interval_ms = prefs.getUShort("flush_int", 1000);
    
    // Load WiFi settings (with default)
    size_t ssid_len = prefs.getString("wifi_ssid", config->wifi_ssid, sizeof(config->wifi_ssid));
    if (ssid_len == 0) {
        strncpy(config->wifi_ssid, "EPIC_CAN_LOGGER", sizeof(config->wifi_ssid) - 1);
    }
    
    size_t pwd_len = prefs.getString("wifi_pwd", config->wifi_password, sizeof(config->wifi_password));
    if (pwd_len == 0) {
        strncpy(config->wifi_password, "password123", sizeof(config->wifi_password) - 1);
    }
    
    // Load and validate checksum
    uint8_t stored_checksum = prefs.getUChar("checksum", 0);
    uint8_t calculated_checksum = configCalculateChecksum(config);
    
    if (stored_checksum != 0 && stored_checksum != calculated_checksum) {
        DEBUG_PRINT("WARN: Configuration checksum mismatch - using defaults\n");
        return configResetToDefaults(config);
    }
    
    // Validate configuration values
    if (!configValidate(config)) {
        DEBUG_PRINT("WARN: Configuration validation failed - using defaults\n");
        return configResetToDefaults(config);
    }
    
    config->checksum = calculated_checksum;
    return true;
}

// Save configuration to EEPROM
bool configSave(const SystemConfig* config) {
    if (!config || !configValidate(config)) {
        return false;
    }
    
    // Calculate and store checksum
    uint8_t checksum = configCalculateChecksum(config);
    
    // Save all configuration values
    prefs.putUChar("ecu_id", config->ecu_id);
    prefs.putUShort("can_speed", config->can_speed_kbps);
    prefs.putUShort("req_int", config->var_request_interval_ms);
    prefs.putUChar("max_pend", config->max_pending_requests);
    prefs.putUChar("shift_rpm", config->shift_light_rpm_threshold);
    prefs.putUChar("debug", config->debug_enabled);
    prefs.putUShort("flush_int", config->log_flush_interval_ms);
    
    prefs.putString("wifi_ssid", config->wifi_ssid);
    prefs.putString("wifi_pwd", config->wifi_password);
    
    prefs.putUChar("checksum", checksum);
    
    DEBUG_PRINT("SUCCESS: Configuration saved\n");
    return true;
}

// Validate configuration values
bool configValidate(const SystemConfig* config) {
    if (!config) {
        return false;
    }
    
    // Validate ECU ID (0-15)
    if (config->ecu_id > 15) {
        DEBUG_PRINT("ERROR: Invalid ECU ID: %d (must be 0-15)\n", config->ecu_id);
        return false;
    }
    
    // Validate CAN speed (common values: 125, 250, 500, 1000 kbps)
    if (config->can_speed_kbps != 125 && 
        config->can_speed_kbps != 250 && 
        config->can_speed_kbps != 500 && 
        config->can_speed_kbps != 1000) {
        DEBUG_PRINT("WARN: Unusual CAN speed: %d kbps\n", config->can_speed_kbps);
        // Not a failure, just a warning
    }
    
    // Validate request interval (1-1000ms)
    if (config->var_request_interval_ms < 1 || config->var_request_interval_ms > 1000) {
        DEBUG_PRINT("ERROR: Invalid request interval: %d ms (must be 1-1000)\n", 
                   config->var_request_interval_ms);
        return false;
    }
    
    // Validate max pending requests (1-255)
    if (config->max_pending_requests < 1 || config->max_pending_requests > 255) {
        DEBUG_PRINT("ERROR: Invalid max pending: %d (must be 1-255)\n", 
                   config->max_pending_requests);
        return false;
    }
    
    // Validate shift light RPM (10-100 = 1000-10000 RPM)
    if (config->shift_light_rpm_threshold < 10 || config->shift_light_rpm_threshold > 100) {
        DEBUG_PRINT("ERROR: Invalid shift light RPM: %d (must be 10-100 = 1000-10000 RPM)\n", 
                   config->shift_light_rpm_threshold);
        return false;
    }
    
    // Validate WiFi SSID length (1-31 characters)
    size_t ssid_len = strlen(config->wifi_ssid);
    if (ssid_len == 0 || ssid_len > 31) {
        DEBUG_PRINT("ERROR: Invalid WiFi SSID length: %d (must be 1-31)\n", ssid_len);
        return false;
    }
    
    // Validate WiFi password length (8-63 characters)
    size_t pwd_len = strlen(config->wifi_password);
    if (pwd_len < 8 || pwd_len > 63) {
        DEBUG_PRINT("ERROR: Invalid WiFi password length: %d (must be 8-63)\n", pwd_len);
        return false;
    }
    
    return true;
}

// Reset configuration to defaults
bool configResetToDefaults(SystemConfig* config) {
    if (!config) {
        return false;
    }
    
    // Set default values
    config->ecu_id = CONFIG_DEFAULT_ECU_ID;
    config->can_speed_kbps = CONFIG_DEFAULT_CAN_SPEED;
    config->var_request_interval_ms = CONFIG_DEFAULT_REQUEST_INTERVAL;
    config->max_pending_requests = CONFIG_DEFAULT_MAX_PENDING;
    config->shift_light_rpm_threshold = CONFIG_DEFAULT_SHIFT_LIGHT_RPM;
    config->debug_enabled = 0;
    config->log_flush_interval_ms = 1000;
    strncpy(config->wifi_ssid, "EPIC_CAN_LOGGER", sizeof(config->wifi_ssid) - 1);
    strncpy(config->wifi_password, "password123", sizeof(config->wifi_password) - 1);
    config->wifi_ssid[sizeof(config->wifi_ssid) - 1] = '\0';
    config->wifi_password[sizeof(config->wifi_password) - 1] = '\0';
    
    // Calculate checksum
    config->checksum = configCalculateChecksum(config);
    
    // Save defaults
    return configSave(config);
}

// Get current configuration (singleton)
const SystemConfig* configGetCurrent() {
    if (!configLoaded) {
        // Load configuration on first access
        if (configInit()) {
            configLoad(&currentConfig);
            configLoaded = true;
        }
    }
    return &currentConfig;
}

