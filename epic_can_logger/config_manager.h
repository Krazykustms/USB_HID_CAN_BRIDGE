// Configuration Manager for EPIC CAN Logger
// Professional-grade: Runtime configuration storage in EEPROM/Flash

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Configuration structure - all settings that can be changed at runtime
typedef struct {
    // CAN settings
    uint8_t ecu_id;              // ECU ID (0-15)
    uint16_t can_speed_kbps;     // CAN bus speed (125, 250, 500, 1000)
    uint16_t var_request_interval_ms;  // Delay between variable requests
    uint8_t max_pending_requests;     // Maximum concurrent requests
    
    // WiFi settings
    char wifi_ssid[32];          // WiFi SSID
    char wifi_password[64];      // WiFi password
    
    // Logging settings
    uint8_t log_buffer_size_kb;   // Log buffer size (currently fixed, reserved)
    uint16_t log_flush_interval_ms;  // SD flush interval
    
    // System settings
    uint8_t debug_enabled;        // Debug output enabled (0/1)
    uint8_t shift_light_rpm_threshold;  // RPM for shift light (in hundreds: 40 = 4000 RPM)
    
    // Integrity
    uint8_t checksum;             // Configuration checksum for validation
} SystemConfig;

// Default configuration values
#define CONFIG_DEFAULT_ECU_ID                1
#define CONFIG_DEFAULT_CAN_SPEED             500
#define CONFIG_DEFAULT_REQUEST_INTERVAL      50
#define CONFIG_DEFAULT_MAX_PENDING           16
#define CONFIG_DEFAULT_SHIFT_LIGHT_RPM       40  // 4000 RPM

// Configuration namespace in Preferences
#define CONFIG_NAMESPACE  "epic_config"

// Function declarations
bool configInit();
bool configLoad(SystemConfig* config);
bool configSave(const SystemConfig* config);
bool configValidate(const SystemConfig* config);
bool configResetToDefaults(SystemConfig* config);
uint8_t configCalculateChecksum(const SystemConfig* config);

// Helper: Get current configuration (singleton pattern)
const SystemConfig* configGetCurrent();

#endif // CONFIG_MANAGER_H

