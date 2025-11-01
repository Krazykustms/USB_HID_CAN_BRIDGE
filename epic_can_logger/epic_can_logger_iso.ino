#include <Arduino.h>
#include <EspUsbHost.h>
#include <Adafruit_NeoPixel.h>
#include "esp_task_wdt.h"
#include <ESP32-TWAI-CAN.hpp>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
// ESPmDNS.h not used, removed to reduce dependencies
#include "epic_variables.h"
#include "sd_logger.h"
#include "rusefi_dbc.h"
#include "config_manager.h"
#include "iso15765.h"
#include "uds.h"

// ============================================
// ISO 14229/15765 COMPLIANT VERSION
// ============================================
// This version includes ISO 15765 (DoCAN) transport layer
// and ISO 14229 (UDS) service layer for compatibility
// with standard automotive diagnostic tools.
//
// For the original version without ISO support,
// use: epic_can_logger.ino
// ============================================

// ------------------------------
// Debug Configuration
// ------------------------------
// Set to 1 to enable debug output, 0 for production (optimized, no Serial blocking)
#define DEBUG_ENABLED           1  // Change to 0 to disable all debug output

// Debug categories (enable specific debug output)
#define DEBUG_CAN_RX            1  // CAN receive messages
#define DEBUG_CAN_TX            1  // CAN transmit (requests)
#define DEBUG_VARIABLES         1  // Variable responses
#define DEBUG_DBC               1  // DBC message decoding
#define DEBUG_BUTTONS           1  // Button presses
#define DEBUG_USB               1  // USB keyboard input
#define DEBUG_SD                1  // SD card operations
#define DEBUG_TIMING            0  // Performance timing (advanced)

// Debug macros (automatically disabled if DEBUG_ENABLED = 0)
#if DEBUG_ENABLED
  #if DEBUG_CAN_RX
    #define DEBUG_CAN_RX_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_CAN_RX_PRINT(...)
  #endif
  
  #if DEBUG_CAN_TX
    #define DEBUG_CAN_TX_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_CAN_TX_PRINT(...)
  #endif
  
  #if DEBUG_VARIABLES
    #define DEBUG_VAR_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_VAR_PRINT(...)
  #endif
  
  #if DEBUG_DBC
    #define DEBUG_DBC_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_DBC_PRINT(...)
  #endif
  
  #if DEBUG_BUTTONS
    #define DEBUG_BTN_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_BTN_PRINT(...)
  #endif
  
  #if DEBUG_USB
    #define DEBUG_USB_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_USB_PRINT(...)
  #endif
  
  #if DEBUG_SD
    #define DEBUG_SD_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_SD_PRINT(...)
  #endif
  
  #if DEBUG_TIMING
    #define DEBUG_TIMING_PRINT(...) Serial.printf(__VA_ARGS__)
  #else
    #define DEBUG_TIMING_PRINT(...)
  #endif
  
  #define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  // All debug macros are empty (compiler removes them completely)
  #define DEBUG_CAN_RX_PRINT(...)
  #define DEBUG_CAN_TX_PRINT(...)
  #define DEBUG_VAR_PRINT(...)
  #define DEBUG_DBC_PRINT(...)
  #define DEBUG_BTN_PRINT(...)
  #define DEBUG_USB_PRINT(...)
  #define DEBUG_SD_PRINT(...)
  #define DEBUG_TIMING_PRINT(...)
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// ------------------------------
// Configuration Constants
// ------------------------------

// GPIO pins - Optimized for ESP32-S3-USB-OTG reliability
#define CAN_TX_PIN                 5   // CAN TX (safe, no strapping)
#define CAN_RX_PIN                 4   // CAN RX (safe, no strapping)
#define LED_NEOPIXEL_PIN           48  // Onboard WS2812 LED (fixed)
#define SHIFT_LIGHT_GPIO           14  // Dedicated GPIO for shift light LED (safe)

// CAN protocol constants
#define CAN_ADDRESS_BUTTONBOX      0x711
#define CAN_MAGIC_BYTE             0x5A
#define CAN_BUTTON_BOX_ID          27
#define CAN_SPEED_KBPS             500

// EPIC CAN protocol constants for variable reading
#define CAN_ID_GET_VAR_REQ_BASE    0x700  // + ecuId
#define CAN_ID_GET_VAR_RES_BASE    0x720  // + ecuId
#define ECU_ID                     1           // Target ECU ID

// Fast logging configuration
#define VAR_REQUEST_INTERVAL_MS    10      // Delay between requests (minimize for speed)
#define MAX_PENDING_REQUESTS       16      // Max requests in flight (pipelining)
#define VAR_READ_CYCLE_MS           100     // How often to cycle through all variables

// Legacy variable IDs (kept for backward compatibility)
#define VAR_ID_TPS_VALUE           1272048601  // TPSValue variable ID
#define VAR_ID_RPM_VALUE           1699696209  // RPMValue variable ID
#define VAR_ID_AFR_VALUE           -1093429509 // AFRValue (O2 sensor) variable ID

// Shift light configuration
#define SHIFT_LIGHT_RPM_THRESHOLD  4000        // RPM threshold for shift light activation

// WiFi configuration
#define WIFI_AP_SSID               "CAN-Bridge-AP"
#define WIFI_AP_PASSWORD           "canbridge123"
#define WIFI_AP_CHANNEL            1
#define WIFI_MAX_CONNECTIONS       4

// Timing and retries
#define CAN_RETRY_DELAY_MS         13
#define CAN_MAX_RETRIES            5
#define CAN_INIT_MAX_RETRIES       10
#define ERROR_THRESHOLD            30

// Professional-grade: System state machine for graceful degradation
enum SystemState {
    SYSTEM_STATE_INIT,        // Initializing
    SYSTEM_STATE_NORMAL,      // All systems operational
    SYSTEM_STATE_DEGRADED,    // Some systems failed, continue with available
    SYSTEM_STATE_CRITICAL,    // Only critical systems operational
    SYSTEM_STATE_FAILURE      // System restart required
};

static SystemState systemState = SYSTEM_STATE_INIT;

// Progressive error recovery thresholds
#define CAN_ERROR_RESET_THRESHOLD  10   // Reset CAN controller at this count
#define CAN_ERROR_REDUCE_SPEED_THRESHOLD  20  // Try reducing speed at this count
#define CAN_ERROR_RESTART_THRESHOLD 30   // Full restart at this count

// Phase 2: Runtime configuration variables (loaded from EEPROM or use defaults)
static uint8_t runtimeECU_ID = ECU_ID;
static uint16_t runtimeCAN_SPEED = CAN_SPEED_KBPS;
static uint16_t runtimeVAR_REQUEST_INTERVAL = VAR_REQUEST_INTERVAL_MS;
static uint8_t runtimeMAX_PENDING = MAX_PENDING_REQUESTS;
static uint16_t runtimeSHIFT_LIGHT_RPM = SHIFT_LIGHT_RPM_THRESHOLD;

// HID constants
#define HID_KEYBOARD_REPORT_SIZE   8

// LED behavior
#define LED_NUM_PIXELS             1

// Watchdog
#define WDT_TIMEOUT_SECONDS        10

// GPIO Buttons
#define BUTTON_COUNT               8
// Choose safe, free GPIOs on ESP32-S3-USB-OTG. Wired: button -> GPIO, other side -> GND
// Using internal pullups (active-low)
// Optimized button pins: Avoided GPIO 9 (Flash CS strapping pin), 0, 46 (boot pins)
// GPIO 6-8: Safe general I/O
// GPIO 19-21: Safe, no strapping functions
// GPIO 36-39: Safe, good for buttons
static const int BUTTON_PINS[BUTTON_COUNT] = { 19, 20, 21, 36, 37, 38, 39, 40 };
// Map each button to a HID keycode (USB usage IDs)
// Examples: 0x04='a', 0x05='b', ... 0x1e='1', 0x1f='2', etc.
static const uint8_t BUTTON_HID_CODES[BUTTON_COUNT] = {
  0x1e, // BTN0 -> '1'
  0x1f, // BTN1 -> '2'
  0x20, // BTN2 -> '3'
  0x21, // BTN3 -> '4'
  0x04, // BTN4 -> 'A'
  0x23, // BTN5 -> '6'
  0x24, // BTN6 -> '7'
  0x25  // BTN7 -> '8'
};
// Debounce settings
#define DEBOUNCE_MS                25

// Long-press configuration
#define LONG_PRESS_BUTTON_INDEX     0   // Index in BUTTON_PINS / BUTTON_HID_CODES (GPIO 19)
#define LONG_PRESS_MS               3000   // 3 seconds for activation

// ------------------------------
// Globals
// ------------------------------

Adafruit_NeoPixel pixels(LED_NUM_PIXELS, LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// CAN driver objects
// Library exposes singleton ESP32Can and struct CanFrame
// Queue sizes are configured in the library; use defaults unless changed elsewhere

// USB host handling
static volatile int deviceGone = 1;  // 1 = no device connected

static volatile uint32_t canErrorCount = 0;

// Variable reading state - pipelining system
static uint32_t lastVarReadTime = 0;
static uint8_t currentVarIndex = 0;  // Current variable in EPIC_VARIABLES array
static uint8_t pendingRequestCount = 0;  // Number of requests in flight

// Response tracking - maps var_id to value (for out-of-order responses)
typedef struct {
    int32_t var_id;
    float value;
    uint32_t timestamp_ms;
    bool valid;
    uint32_t request_time_ms;  // ERROR #10 FIX: Track when request was sent
    bool request_pending;       // ERROR #10 FIX: Track if request is pending
} VarResponse;

static VarResponse varResponses[EPIC_VAR_COUNT];  // Store latest value for each variable

// ERROR #10 FIX: Variable response timeout checking
#define VAR_RESPONSE_TIMEOUT_MS  2000  // 2 second timeout for variable responses

// WiFi and web server
WebServer server(80);
static volatile float tpsValue = 0.0;
static volatile float rpmValue = 0.0;
static volatile float afrValue = 0.0;
static volatile bool shiftLightActive = false;

// rusEFI DBC decoded data storage
static rusefi_base0_t dbc_base0;
static rusefi_base1_t dbc_base1;
static rusefi_base2_t dbc_base2;
static rusefi_base3_t dbc_base3;
static rusefi_base4_t dbc_base4;
static rusefi_base5_t dbc_base5;
static rusefi_base6_t dbc_base6;
static rusefi_base7_t dbc_base7;
static rusefi_base8_t dbc_base8;
static rusefi_base9_t dbc_base9;
static rusefi_base10_t dbc_base10;
static uint32_t dbc_last_update[11] = {0};  // Track last update time for each message

typedef struct {
  uint8_t stableLevel;      // last stable level (HIGH with pullup = not pressed)
  uint8_t lastReadLevel;    // last raw read
  uint32_t lastChangeMs;    // last time raw changed
  uint8_t pressed;          // edge-detected pressed flag (cleared after handled)
  uint32_t holdStartMs;     // when LOW became stable (for long press)
  uint8_t longFired;        // prevent repeat firing on long press
  uint8_t active;           // latched active state (used for GPIO 19 long/short behavior)
} ButtonState;

static ButtonState buttonStates[BUTTON_COUNT];

// ------------------------------
// Utility: LED helpers
// ------------------------------

static inline void ledOff() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
}

static inline void ledBlue() {
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
}

static inline void ledGreen() {
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
}

static inline void ledRed() {
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();
}

static inline void ledYellow() {
  pixels.setPixelColor(0, pixels.Color(255, 200, 0));
  pixels.show();
}

// ------------------------------
// Shift light LED helpers
// ------------------------------

static inline void shiftLightOn() {
  digitalWrite(SHIFT_LIGHT_GPIO, HIGH);  // Turn on LED
}

static inline void shiftLightOff() {
  digitalWrite(SHIFT_LIGHT_GPIO, LOW);   // Turn off LED
}

// ------------------------------
// CAN helpers
// ------------------------------

static bool beginCanWithRetry() {
  // TwaiCAN::setPins expects (txPin, rxPin)
  ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
  // Library expects TwaiSpeed enum; convert numeric kbps to enum
  // Use runtime configuration if available
  ESP32Can.setSpeed(ESP32Can.convertSpeed(runtimeCAN_SPEED));

  int retries = CAN_INIT_MAX_RETRIES;
  while (!ESP32Can.begin() && retries > 0) {
    DEBUG_PRINT("CAN init failed, retrying... (%d remaining)\n", retries);
    ledRed();
    delay(500);
    retries--;
  }
  if (retries == 0) {
    DEBUG_PRINT("FATAL: CAN initialization failed\n");
    ledRed();
    return false;
  }
  return true;
}

// Helper: write big-endian 32-bit
static void be_u32(uint8_t *b, uint32_t v) {
    b[0] = (v >> 24) & 0xFF;
    b[1] = (v >> 16) & 0xFF;
    b[2] = (v >> 8) & 0xFF;
    b[3] = v & 0xFF;
}

// Helper: read big-endian int32
static int32_t be_i32(uint8_t *b) {
    return (int32_t)((uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | (uint32_t)b[3]);
}

// Helper: read big-endian float32
static float be_f32(uint8_t *b) {
    uint32_t bits = (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | (uint32_t)b[3];
    float val;
    memcpy(&val, &bits, sizeof(float));
    return val;
}

// Request variable from ECU
static bool requestVar(int32_t var_id) {
    uint8_t data[4];
    be_u32(data, (uint32_t)var_id);
    
    CanFrame frame = {0};
    frame.identifier = CAN_ID_GET_VAR_REQ_BASE + (ECU_ID & 0x0F);
    frame.extd = 0;  // Standard frame
    frame.data_length_code = 4;
    memcpy(frame.data, data, 4);
    
    return ESP32Can.writeFrame(frame, 5);
}

static bool sendCMD(uint8_t keyLSB, uint8_t keyMSB) {
  // Payload format (5 bytes):
  // [0] 0x5A, [1] 0x00, [2] 27, [3] keyMSB (modifier), [4] keyLSB (keycode)
  uint8_t payload[5];
  payload[0] = CAN_MAGIC_BYTE;
  payload[1] = 0x00;
  payload[2] = CAN_BUTTON_BOX_ID;
  payload[3] = keyMSB;
  payload[4] = keyLSB;

  CanFrame frame = {0};
  frame.identifier = CAN_ADDRESS_BUTTONBOX;
  frame.extd = 0;  // Standard frame
  frame.data_length_code = 5;
  memcpy(frame.data, payload, 5);

  int attempts = CAN_MAX_RETRIES;
  while (attempts-- > 0) {
    ledBlue();
    bool queued = ESP32Can.writeFrame(frame, 5 /* small timeout to allow queue */);
    if (queued) {
      // Reset error counter on successful queue
      canErrorCount = 0;
      ledOff();
      return true;
    }
    delay(CAN_RETRY_DELAY_MS);
    yield();  // Yield to allow CAN RX processing during retries
  }

  // Professional-grade: Progressive error recovery instead of immediate restart
  canErrorCount++;
  
  // Progressive recovery strategy
  if (canErrorCount > CAN_ERROR_RESTART_THRESHOLD) {
    // Last resort: Full system restart
    DEBUG_CAN_TX_PRINT("FATAL: Too many CAN errors (%d), restarting...\n", canErrorCount);
    delay(100);  // Allow debug message to be sent
    ESP.restart();
  } else if (canErrorCount > CAN_ERROR_REDUCE_SPEED_THRESHOLD) {
    // Second attempt: Reset CAN controller (soft reset)
    DEBUG_CAN_TX_PRINT("WARN: CAN errors high (%d), resetting CAN controller...\n", canErrorCount);
    ESP32Can.end();
    delay(100);
    if (ESP32Can.begin()) {
      canErrorCount = 0;  // Reset counter on successful recovery
      DEBUG_CAN_TX_PRINT("SUCCESS: CAN controller reset successful\n");
    } else {
      DEBUG_CAN_TX_PRINT("ERROR: CAN controller reset failed\n");
      // Will reach restart threshold on next error
    }
  } else if (canErrorCount > CAN_ERROR_RESET_THRESHOLD) {
    // First attempt: Try CAN controller reset
    DEBUG_CAN_TX_PRINT("WARN: CAN errors moderate (%d), attempting CAN reset...\n", canErrorCount);
    ESP32Can.end();
    delay(50);
    if (ESP32Can.begin()) {
      canErrorCount = 0;  // Reset counter on successful recovery
      DEBUG_CAN_TX_PRINT("SUCCESS: CAN reset successful\n");
    }
  } else {
    DEBUG_CAN_TX_PRINT("WARN: Button CAN send failed (ID=0x%03X, key=0x%02X), retries exhausted, errors: %d\n", 
                       frame.identifier, keyLSB, canErrorCount);
  }
  ledOff();
  return false;
}

// ------------------------------
// USB Host handling
// ------------------------------

class MyEspUsbHost : public EspUsbHost {
public:
  void onReceive(const usb_transfer_t *transfer) override {
    if (!transfer || !transfer->data_buffer) {
      DEBUG_USB_PRINT("ERROR: Invalid transfer buffer\n");
      return;
    }

    // Debug: Dump limited bytes for USB keyboard input
    #if DEBUG_USB
      size_t toPrint = min((int)transfer->data_buffer_size, 50);
      for (size_t i = 0; i < toPrint; i++) {
        DEBUG_USB_PRINT("%02x ", transfer->data_buffer[i]);
      }
      DEBUG_USB_PRINTLN();
    #endif

    // Ensure we have at least the standard 8-byte keyboard report
    if (transfer->num_bytes >= HID_KEYBOARD_REPORT_SIZE &&
        transfer->data_buffer_size >= HID_KEYBOARD_REPORT_SIZE) {
      uint8_t modifier = transfer->data_buffer[0];
      uint8_t firstKey = transfer->data_buffer[2];
      uint8_t secondKey = transfer->data_buffer[3];

      // Visual feedback for RX
      ledGreen();

      // Encode as 16-bit values: upper byte = modifier, lower byte = key
      uint16_t encodedFirst = 0;
      uint16_t encodedSecond = 0;
      if (firstKey > 0) {
        encodedFirst = (uint16_t)firstKey | ((uint16_t)modifier << 8);
      }
      if (secondKey > 0) {
        encodedSecond = (uint16_t)secondKey | ((uint16_t)modifier << 8);
      }

      // Send primary key if present
      if (encodedFirst != 0) {
        (void)sendCMD((uint8_t)(encodedFirst & 0xFF), (uint8_t)((encodedFirst >> 8) & 0xFF));
      }
      // Send secondary key if present
      if (encodedSecond != 0) {
        (void)sendCMD((uint8_t)(encodedSecond & 0xFF), (uint8_t)((encodedSecond >> 8) & 0xFF));
      }

      ledOff();
    }
  }

  void onGone(const usb_host_client_event_msg_t *eventMsg) override {
    deviceGone = 1;
    DEBUG_USB_PRINT("USB device disconnected\n");
  }

  void onConnected() {
    deviceGone = 0;
    DEBUG_USB_PRINT("USB device connected (keyboard ready)\n");
  }
};

MyEspUsbHost usbHost;

// ------------------------------
// Web server handlers
// ------------------------------

// Forward declarations for Phase 2 endpoints
void handleHealth();
void handleConfig();
void handleConfigSave();

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CAN Bridge Monitor</title>
    <meta http-equiv="refresh" content="1">
    <style>
        body { font-family: Arial; background: #1a1a1a; color: #fff; padding: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .card { background: #2d2d2d; border-radius: 10px; padding: 20px; margin: 10px 0; }
        .value { font-size: 48px; font-weight: bold; color: #4CAF50; }
        .label { font-size: 18px; color: #888; margin-bottom: 10px; }
        .shift-light { background: #ff4444; }
    </style>
</head>
<body>
    <div class="container">
        <h1>CAN Bridge Monitor</h1>
        
        <div class="card">
            <div class="label">Throttle Position (TPS)</div>
            <div class="value">)" + String(tpsValue, 1) + R"(</div>
        </div>
        
        <div class="card">
            <div class="label">Engine RPM</div>
            <div class="value">)" + String(rpmValue, 0) + R"( rpm</div>
        </div>
        
        <div class="card">
            <div class="label">AFR (O2 Sensor)</div>
            <div class="value">)" + String(afrValue, 2) + R"(</div>
        </div>
        
        <div class="card" + (shiftLightActive ? String(" shift-light") : String("")) + R"(")>
            <div class="label">Shift Light</div>
            <div class="value">)" + (shiftLightActive ? String("ON") : String("OFF")) + R"(</div>
        </div>
    </div>
</body>
</html>)";
  
  server.send(200, "text/html", html);
}

void handleData() {
  // JSON API for AJAX requests
  String json = "{";
  json += "\"tps\":" + String(tpsValue, 2) + ",";
  json += "\"rpm\":" + String(rpmValue, 0) + ",";
  json += "\"afr\":" + String(afrValue, 2) + ",";
  json += "\"shiftLight\":" + String(shiftLightActive ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// Phase 2: Health metrics endpoint
void handleHealth() {
  // Collect system health data
  uint32_t uptime_seconds = millis() / 1000;
  uint32_t free_heap = ESP.getFreeHeap();
  uint32_t total_heap = ESP.getHeapSize();
  float memory_usage = ((float)(total_heap - free_heap) / total_heap) * 100.0;
  
  // CAN statistics
  uint8_t can_status = (canErrorCount < CAN_ERROR_RESET_THRESHOLD) ? 1 : 0;
  
  // SD card status
  uint8_t sd_status = sdLoggerGetStatus();
  uint32_t sd_sequence = 0;
  #if LOG_ENABLE_SEQUENCE_NUMBERS
    sd_sequence = sdLoggerGetSequenceNumber();
  #endif
  
  // System state
  const char* state_str = "UNKNOWN";
  switch (systemState) {
    case SYSTEM_STATE_INIT: state_str = "INIT"; break;
    case SYSTEM_STATE_NORMAL: state_str = "NORMAL"; break;
    case SYSTEM_STATE_DEGRADED: state_str = "DEGRADED"; break;
    case SYSTEM_STATE_CRITICAL: state_str = "CRITICAL"; break;
    case SYSTEM_STATE_FAILURE: state_str = "FAILURE"; break;
  }
  
  // Build JSON response
  String json = "{";
  json += "\"uptime_seconds\":" + String(uptime_seconds) + ",";
  json += "\"system_state\":\"" + String(state_str) + "\",";
  json += "\"memory_free_bytes\":" + String(free_heap) + ",";
  json += "\"memory_total_bytes\":" + String(total_heap) + ",";
  json += "\"memory_usage_percent\":" + String(memory_usage, 2) + ",";
  json += "\"can_status\":" + String(can_status) + ",";
  json += "\"can_errors\":" + String(canErrorCount) + ",";
  json += "\"sd_status\":" + String(sd_status) + ",";
  json += "\"sd_sequence\":" + String(sd_sequence) + ",";
  json += "\"pending_requests\":" + String(pendingRequestCount) + ",";
  json += "\"variables\":{";
  json += "\"tps\":" + String(tpsValue, 6) + ",";
  json += "\"rpm\":" + String(rpmValue, 1) + ",";
  json += "\"afr\":" + String(afrValue, 2);
  json += "}";
  json += "}";
  
  server.send(200, "application/json", json);
}

// Phase 2: Configuration endpoint (GET - view config)
void handleConfig() {
  // Return current configuration as JSON
  const SystemConfig* config = configGetCurrent();
  if (!config) {
    server.send(500, "text/plain", "Configuration not available");
    return;
  }
  
  String json = "{";
  json += "\"ecu_id\":" + String(config->ecu_id) + ",";
  json += "\"can_speed_kbps\":" + String(config->can_speed_kbps) + ",";
  json += "\"var_request_interval_ms\":" + String(config->var_request_interval_ms) + ",";
  json += "\"max_pending_requests\":" + String(config->max_pending_requests) + ",";
  json += "\"shift_light_rpm_threshold\":" + String(config->shift_light_rpm_threshold * 100) + ",";
  json += "\"wifi_ssid\":\"" + String(config->wifi_ssid) + "\",";
  json += "\"wifi_password\":\"***\",";  // Don't expose password
  json += "\"debug_enabled\":" + String(config->debug_enabled);
  json += "}";
  
  server.send(200, "application/json", json);
}

// Phase 2: Save configuration
void handleConfigSave() {
  SystemConfig config;
  const SystemConfig* current = configGetCurrent();
  
  // Start with current config
  if (current) {
    memcpy(&config, current, sizeof(SystemConfig));
  } else {
    // Load defaults if no current config
    configResetToDefaults(&config);
  }
  
  // Parse POST parameters
  if (server.hasArg("ecu_id")) {
    config.ecu_id = server.arg("ecu_id").toInt();
  }
  if (server.hasArg("can_speed_kbps")) {
    config.can_speed_kbps = server.arg("can_speed_kbps").toInt();
  }
  if (server.hasArg("var_request_interval_ms")) {
    config.var_request_interval_ms = server.arg("var_request_interval_ms").toInt();
  }
  if (server.hasArg("max_pending_requests")) {
    config.max_pending_requests = server.arg("max_pending_requests").toInt();
  }
  if (server.hasArg("shift_light_rpm")) {
    uint16_t rpm = server.arg("shift_light_rpm").toInt();
    config.shift_light_rpm_threshold = (rpm + 50) / 100;  // Round to nearest hundred
  }
  if (server.hasArg("wifi_ssid")) {
    String ssid = server.arg("wifi_ssid");
    strncpy(config.wifi_ssid, ssid.c_str(), sizeof(config.wifi_ssid) - 1);
    config.wifi_ssid[sizeof(config.wifi_ssid) - 1] = '\0';
  }
  if (server.hasArg("wifi_password")) {
    String pwd = server.arg("wifi_password");
    strncpy(config.wifi_password, pwd.c_str(), sizeof(config.wifi_password) - 1);
    config.wifi_password[sizeof(config.wifi_password) - 1] = '\0';
  }
  if (server.hasArg("debug_enabled")) {
    config.debug_enabled = server.arg("debug_enabled").toInt() ? 1 : 0;
  }
  
  // Validate configuration
  if (!configValidate(&config)) {
    server.send(400, "application/json", "{\"error\":\"Invalid configuration values\"}");
    return;
  }
  
  // Save configuration
  if (configSave(&config)) {
    // Update runtime variables (will apply on next restart or can be applied immediately)
    runtimeECU_ID = config.ecu_id;
    runtimeCAN_SPEED = config.can_speed_kbps;
    runtimeVAR_REQUEST_INTERVAL = config.var_request_interval_ms;
    runtimeMAX_PENDING = config.max_pending_requests;
    runtimeSHIFT_LIGHT_RPM = config.shift_light_rpm_threshold * 100;
    
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved. Some changes require restart.\"}");
    DEBUG_PRINT("Configuration saved via web interface\n");
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to save configuration\"}");
  }
}

void initWiFi() {
  // Load WiFi configuration from runtime config
  const SystemConfig* config = configGetCurrent();
  const char* wifi_ssid = config ? config->wifi_ssid : WIFI_AP_SSID;
  const char* wifi_password = config ? config->wifi_password : WIFI_AP_PASSWORD;
  
  // Start WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(wifi_ssid, wifi_password, WIFI_AP_CHANNEL, false, WIFI_MAX_CONNECTIONS);
  
  IPAddress IP = WiFi.softAPIP();
  DEBUG_PRINT("WiFi AP started\n");
  DEBUG_PRINT("AP SSID: %s\n", wifi_ssid);
  DEBUG_PRINT("AP IP address: %s\n", IP.toString().c_str());
  
  // Start web server with all endpoints
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/health", handleHealth);  // Phase 2: Health metrics endpoint
  server.on("/config", handleConfig);  // Phase 2: Configuration endpoint
  server.on("/config/save", HTTP_POST, handleConfigSave);  // Phase 2: Save config
  server.begin();
  DEBUG_PRINT("Web server started at http://%s\n", IP.toString().c_str());
}

// ------------------------------
// Setup and Loop
// ------------------------------

void setup() {
  Serial.begin(115200);
  delay(100);

  pixels.begin();
  ledOff();

  // Watchdog
  const esp_task_wdt_config_t wdt_cfg = {
    .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_err_t wdt_status = esp_task_wdt_status(NULL);
  if (wdt_status == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_init(&wdt_cfg);
  } else {
    esp_task_wdt_reconfigure(&wdt_cfg);
  }
  esp_task_wdt_add(NULL);

  // Phase 2: Load runtime configuration from EEPROM
  SystemConfig config;
  if (configInit()) {
    if (configLoad(&config)) {
      DEBUG_PRINT("Configuration loaded from EEPROM\n");
      // Apply runtime configuration
      runtimeECU_ID = config.ecu_id;
      runtimeCAN_SPEED = config.can_speed_kbps;
      runtimeVAR_REQUEST_INTERVAL = config.var_request_interval_ms;
      runtimeMAX_PENDING = config.max_pending_requests;
      runtimeSHIFT_LIGHT_RPM = config.shift_light_rpm_threshold * 100;  // Convert from hundreds to RPM
      DEBUG_PRINT("Runtime config: ECU=%d, CAN=%d kbps, Interval=%d ms, MaxPending=%d, ShiftLight=%d RPM\n",
                 runtimeECU_ID, runtimeCAN_SPEED, runtimeVAR_REQUEST_INTERVAL, 
                 runtimeMAX_PENDING, runtimeSHIFT_LIGHT_RPM);
    } else {
      DEBUG_PRINT("Using default configuration (no config in EEPROM)\n");
      // Use default values (already set in static variables)
    }
  } else {
    DEBUG_PRINT("WARN: Failed to init configuration storage, using defaults\n");
  }

  // Initialize CAN (uses runtime configuration)
  if (!beginCanWithRetry()) {
    // Fatal: leave LED red and halt
    while (true) {
      delay(1000);
    }
  }

  // Initialize ISO 15765 transport layer and UDS services
  if (iso15765_init(runtimeECU_ID)) {
    DEBUG_PRINT("ISO 15765 transport layer initialized (ECU ID: %d)\n", runtimeECU_ID);
  }
  if (uds_init()) {
    DEBUG_PRINT("UDS service layer initialized\n");
  }

  // Initialize USB Host
  usbHost.begin();
  deviceGone = 1;  // Until we know a device is present

  // Initialize buttons
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    buttonStates[i].stableLevel = HIGH;
    buttonStates[i].lastReadLevel = HIGH;
    buttonStates[i].lastChangeMs = millis();
    buttonStates[i].pressed = 0;
    buttonStates[i].holdStartMs = 0;
    buttonStates[i].longFired = 0;
    buttonStates[i].active = 0;
  }

  // Initialize shift light GPIO (output, initially off)
  pinMode(SHIFT_LIGHT_GPIO, OUTPUT);
  shiftLightOff();

  // Initialize WiFi access point and web server
  initWiFi();

  // Professional-grade: Graceful degradation - Continue even if SD fails
  bool sdOk = sdLoggerInit();
  if (sdOk) {
    sdLoggerStart();
    
    // Write CSV header with variable names
    const char* var_names[EPIC_VAR_COUNT];
    for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
      var_names[i] = EPIC_VARIABLES[i].name;
    }
    sdLoggerWriteHeader(var_names, EPIC_VAR_COUNT);
    DEBUG_SD_PRINT("SD logging enabled with %d variables\n", EPIC_VAR_COUNT);
  } else {
    DEBUG_SD_PRINT("WARNING: SD logging disabled - SD card init failed (continuing without SD)\n");
    // System continues to operate without SD logging
    // This is graceful degradation - don't restart
  }
  
  // Update system state based on initialization
  if (sdOk) {
    systemState = SYSTEM_STATE_NORMAL;
  } else {
    systemState = SYSTEM_STATE_DEGRADED;  // SD failed, but CAN/USB still work
  }

  // Initialize variable response tracking
  for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
    varResponses[i].var_id = EPIC_VARIABLES[i].var_id;
    varResponses[i].value = 0.0;
    varResponses[i].timestamp_ms = 0;
    varResponses[i].valid = false;
  }

  // Print variable reading info
  DEBUG_PRINT("EPIC CAN Logger initialized\n");
  DEBUG_PRINT("Logging %d variables from ECU %d:\n", EPIC_VAR_COUNT, ECU_ID);
  for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
    DEBUG_PRINT("  - %s (ID %d)\n", EPIC_VARIABLES[i].name, EPIC_VARIABLES[i].var_id);
  }
  DEBUG_PRINT("Request pipelining: max %d pending requests\n", MAX_PENDING_REQUESTS);
  DEBUG_PRINT("Request interval: %d ms\n", VAR_REQUEST_INTERVAL_MS);
}

// Pre-computed signal IDs for fast logging (avoid strlen in hot path)
#define DBC_SIG_ID(msg_id, sig_offset) ((msg_id) * 10000 + (sig_offset))

// Signal offsets for each message (pre-computed, no string operations)
// BASE0 signals
#define SIG_WARNINGCOUNTER    1
#define SIG_DISTANCETRAVELED  2

// BASE1 signals  
#define SIG_RPM               1
#define SIG_IGNITIONTIMING    2
#define SIG_VEHICLESPEED      3

// BASE2 signals
#define SIG_TPS1              1
#define SIG_TPS2              2
#define SIG_PPS               3

// BASE3 signals
#define SIG_MAP               1
#define SIG_COOLANTTEMP       2
#define SIG_INTAKETEMP        3

// BASE4 signals
#define SIG_OILPRESS          1
#define SIG_BATTVOLT          2

// BASE5 signals
#define SIG_CYLAM             1
#define SIG_ESTMAF            2
#define SIG_INJPW             3

// BASE6 signals
#define SIG_FUELFLOW          1

// BASE7 signals
#define SIG_LAM1              1
#define SIG_LAM2              2

// BASE9 signals
#define SIG_EGT1              1

// Helper: Log DBC signal to SD card - OPTIMIZED (no string operations)
static inline void logDbcSignal(uint32_t timestamp, uint32_t msg_id, uint16_t signal_offset, float value) {
    // Direct ID calculation - no string operations
    sdLoggerWriteEntry(timestamp, DBC_SIG_ID(msg_id, signal_offset), value);
}

static void handleCanRx() {
  CanFrame rx;
  uint32_t nowMs = millis();
  
  // Process CAN messages with time budget to prevent blocking other operations
  // Maximum processing time: 10ms (allows processing ~100 messages at 0.1ms each)
  const uint32_t MAX_CAN_PROCESS_TIME_MS = 10;
  uint32_t processStartTime = nowMs;
  uint16_t messagesProcessed = 0;
  
  // ERROR #9 FIX: Adaptive processing limit based on queue depth
  uint16_t queueDepth = ESP32Can.inRxQueue();
  uint16_t MAX_MESSAGES_PER_CYCLE;
  
  if (queueDepth > 100) {
    // High load - process more messages to catch up
    MAX_MESSAGES_PER_CYCLE = 100;
  } else if (queueDepth > 50) {
    // Medium load - standard processing
    MAX_MESSAGES_PER_CYCLE = 75;
  } else {
    // Normal load - standard limit
    MAX_MESSAGES_PER_CYCLE = 50;
  }
  
  while (ESP32Can.inRxQueue() > 0 && messagesProcessed < MAX_MESSAGES_PER_CYCLE) {
    // Time budget check - exit if taking too long
    if ((millis() - processStartTime) > MAX_CAN_PROCESS_TIME_MS) {
      // Exit early to maintain system responsiveness
      // Remaining messages will be processed next loop cycle
      break;
    }
    
    if (ESP32Can.readFrame(rx, 0)) {
      messagesProcessed++;
      
      // ISO 15765/UDS message routing (dual protocol support)
      // Check for ISO 15765 physical request (functional addressing)
      if (rx.identifier == ISO_15765_PHYSICAL_REQUEST_BASE || 
          rx.identifier == (ISO_15765_PHYSICAL_REQUEST_BASE + runtimeECU_ID)) {
        // Process ISO 15765 message
        iso15765_process_rx(&rx);
        
        // Check if ISO message is complete
        uint8_t iso_data[4095];
        uint16_t iso_length = 0;
        if (iso15765_receive_complete(iso_data, &iso_length)) {
          // Process UDS service request
          uint8_t uds_response[4095];
          uint16_t uds_response_len = 0;
          
          if (uds_process_request(iso_data, iso_length, uds_response, &uds_response_len)) {
            // Send UDS response via ISO 15765
            uint32_t response_id = ISO_15765_PHYSICAL_RESPONSE_BASE + iso15765_get_ecu_id();
            if (uds_response_len <= 7) {
              iso15765_send_single(uds_response, uds_response_len, response_id);
            } else {
              iso15765_send_multi(uds_response, uds_response_len, response_id);
            }
          }
        }
        // Continue to also check for EPIC/DBC messages below
      }
      
      // Check if this is a rusEFI broadcast message (DBC format)
      // Verify data length is valid (8 bytes for CAN standard frame)
      if (rx.identifier >= RUSEFI_MSG_BASE0 && rx.identifier <= RUSEFI_MSG_BASE10 && rx.data_length_code >= 8) {
        // Decode rusEFI broadcast message
        bool decoded = false;
        uint8_t msg_index = rx.identifier - RUSEFI_MSG_BASE0;
        // Bounds check (msg_index should be 0-10, array has 11 elements)
        if (msg_index > 10) {
          DEBUG_CAN_RX_PRINT("ERROR: Invalid DBC message index: %d (ID: %d)\n", msg_index, rx.identifier);
          continue;
        }
        
        switch (rx.identifier) {
          case RUSEFI_MSG_BASE0:
            if (dbc_decode_base0(rx.data, &dbc_base0) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_WARNINGCOUNTER, dbc_base0.WarningCounter);
              logDbcSignal(nowMs, rx.identifier, SIG_DISTANCETRAVELED, dbc_base0.DistanceTraveled);
            }
            break;
            
          case RUSEFI_MSG_BASE1:
            if (dbc_decode_base1(rx.data, &dbc_base1) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              // Update RPM from DBC (preferred over EPIC if available)
              // Valid RPM range: 0-20000 (0 is valid - engine off)
              if (dbc_base1.RPM <= 20000) {
                rpmValue = dbc_base1.RPM;
                // Shift light logic (use runtime configuration)
                if (dbc_base1.RPM >= runtimeSHIFT_LIGHT_RPM) {
                  shiftLightOn();
                  shiftLightActive = true;
                } else {
                  shiftLightOff();
                  shiftLightActive = false;
                }
              }
              logDbcSignal(nowMs, rx.identifier, SIG_RPM, dbc_base1.RPM);
              logDbcSignal(nowMs, rx.identifier, SIG_IGNITIONTIMING, dbc_base1.IgnitionTiming);
              logDbcSignal(nowMs, rx.identifier, SIG_VEHICLESPEED, dbc_base1.VehicleSpeed);
              DEBUG_DBC_PRINT("DBC[513] CAN ID=0x%03X: RPM=%.0f, Timing=%.2f deg, Speed=%d kph\n", 
                              rx.identifier, dbc_base1.RPM, dbc_base1.IgnitionTiming, dbc_base1.VehicleSpeed);
            }
            break;
            
          case RUSEFI_MSG_BASE2:
            if (dbc_decode_base2(rx.data, &dbc_base2) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              // Update TPS from DBC (valid range: 0-100%)
              if (dbc_base2.TPS1 >= 0.0f && dbc_base2.TPS1 <= 100.0f) {
                tpsValue = dbc_base2.TPS1;
              }
              logDbcSignal(nowMs, rx.identifier, SIG_TPS1, dbc_base2.TPS1);
              logDbcSignal(nowMs, rx.identifier, SIG_TPS2, dbc_base2.TPS2);
              logDbcSignal(nowMs, rx.identifier, SIG_PPS, dbc_base2.PPS);
              DEBUG_DBC_PRINT("DBC[514] CAN ID=0x%03X: TPS1=%.2f%%, TPS2=%.2f%%, PPS=%.2f%%\n",
                              rx.identifier, dbc_base2.TPS1, dbc_base2.TPS2, dbc_base2.PPS);
            }
            break;
            
          case RUSEFI_MSG_BASE3:
            if (dbc_decode_base3(rx.data, &dbc_base3) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_MAP, dbc_base3.MAP);
              logDbcSignal(nowMs, rx.identifier, SIG_COOLANTTEMP, dbc_base3.CoolantTemp);
              logDbcSignal(nowMs, rx.identifier, SIG_INTAKETEMP, dbc_base3.IntakeTemp);
              DEBUG_DBC_PRINT("DBC[515] CAN ID=0x%03X: MAP=%.1f kPa, Coolant=%.1f°C, IAT=%.1f°C\n",
                              rx.identifier, dbc_base3.MAP, dbc_base3.CoolantTemp, dbc_base3.IntakeTemp);
            }
            break;
            
          case RUSEFI_MSG_BASE4:
            if (dbc_decode_base4(rx.data, &dbc_base4) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_OILPRESS, dbc_base4.OilPress);
              logDbcSignal(nowMs, rx.identifier, SIG_BATTVOLT, dbc_base4.BattVolt * 0.001f);  // Convert mV to V inline
            }
            break;
            
          case RUSEFI_MSG_BASE5:
            if (dbc_decode_base5(rx.data, &dbc_base5) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_CYLAM, dbc_base5.CylAM);
              logDbcSignal(nowMs, rx.identifier, SIG_ESTMAF, dbc_base5.EstMAF);
              logDbcSignal(nowMs, rx.identifier, SIG_INJPW, dbc_base5.InjPW);
            }
            break;
            
          case RUSEFI_MSG_BASE6:
            if (dbc_decode_base6(rx.data, &dbc_base6) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_FUELFLOW, dbc_base6.FuelFlow);
            }
            break;
            
          case RUSEFI_MSG_BASE7:
            if (dbc_decode_base7(rx.data, &dbc_base7) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              // Update AFR from lambda (valid range: 0.5-2.0 lambda, ~7.35-29.4 AFR)
              if (dbc_base7.Lam1 >= 0.5f && dbc_base7.Lam1 <= 2.0f) {
                afrValue = dbc_base7.Lam1 * 14.7f;  // Convert lambda to AFR
              }
              logDbcSignal(nowMs, rx.identifier, SIG_LAM1, dbc_base7.Lam1);
              logDbcSignal(nowMs, rx.identifier, SIG_LAM2, dbc_base7.Lam2);
              DEBUG_DBC_PRINT("DBC[519] CAN ID=0x%03X: Lambda1=%.4f (AFR=%.2f), Lambda2=%.4f\n",
                              rx.identifier, dbc_base7.Lam1, dbc_base7.Lam1 * 14.7f, dbc_base7.Lam2);
            }
            break;
            
          case RUSEFI_MSG_BASE8:
            if (dbc_decode_base8(rx.data, &dbc_base8) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              // Cam positions logged if needed
            }
            break;
            
          case RUSEFI_MSG_BASE9:
            if (dbc_decode_base9(rx.data, &dbc_base9) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              logDbcSignal(nowMs, rx.identifier, SIG_EGT1, dbc_base9.Egt1);
            }
            break;
            
          case RUSEFI_MSG_BASE10:
            if (dbc_decode_base10(rx.data, &dbc_base10) && msg_index < 11) {
              dbc_last_update[msg_index] = nowMs;
              decoded = true;
              // Knock sensors logged if needed
            }
            break;
        }
        
        // Continue to also check for EPIC variable responses below
      }
      
      // Check if this is a variable response from our target ECU (EPIC protocol)
      // Validate data length before accessing data array
      if ((rx.identifier & 0x7F0) == CAN_ID_GET_VAR_RES_BASE && rx.data_length_code >= 8) {
        if ((rx.identifier & 0x0F) == (runtimeECU_ID & 0x0F)) {
          // Safe to access rx.data[0-7] since data_length_code >= 8
          int32_t received_var_id = be_i32(rx.data);
          float value = be_f32(&rx.data[4]);
          
          // Find which variable this is and store it
          bool found = false;
          for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
            if (EPIC_VARIABLES[i].var_id == received_var_id) {
              varResponses[i].var_id = received_var_id;
              varResponses[i].value = value;
              varResponses[i].timestamp_ms = nowMs;
              varResponses[i].valid = true;
              // ERROR #10 FIX: Clear pending request flag on successful response
              varResponses[i].request_pending = false;
              varResponses[i].request_time_ms = 0;
              found = true;
              
              // Log to SD card with timestamp and variable value
              // Safe cast: int32_t var_id to uint32_t (values are positive hash IDs)
              // If negative, cast still works but log ID will be large positive number
              sdLoggerWriteEntry(nowMs, (uint32_t)received_var_id, value);
              
              // Update web display variables (for backward compatibility)
              if (received_var_id == VAR_ID_TPS_VALUE) {
                tpsValue = value;
                DEBUG_VAR_PRINT("TPSValue: %.6f\n", value);
              } else if (received_var_id == VAR_ID_RPM_VALUE) {
                rpmValue = value;
                DEBUG_VAR_PRINT("RPMValue: %.1f rpm\n", value);
                
                // Shift light logic (use runtime configuration)
                if (value >= runtimeSHIFT_LIGHT_RPM) {
                  shiftLightOn();
                  shiftLightActive = true;
                } else {
                  shiftLightOff();
                  shiftLightActive = false;
                }
              } else if (received_var_id == VAR_ID_AFR_VALUE) {
                afrValue = value;
                DEBUG_VAR_PRINT("AFRValue: %.2f\n", value);
              } else {
                // Log other variables (less verbosely)
                DEBUG_VAR_PRINT("Var %d: %.6f\n", received_var_id, value);
              }
              
              // Decrease pending count (saturate at 0, prevent underflow)
              if (pendingRequestCount > 0) {
                pendingRequestCount--;
              } else {
                // Mismatch detected - response received but no pending request
                DEBUG_CAN_RX_PRINT("WARN: Response without pending request (var_id: %d, value: %.6f)\n", 
                                   received_var_id, value);
              }
              break;
            }
          }
          
          if (!found) {
            DEBUG_CAN_RX_PRINT("Unknown variable ID: %d, value: %.6f (logged to SD)\n", received_var_id, value);
          }
        }
      }
    }
  }
}

// Professional-grade: Update system state based on subsystem health
static void updateSystemState() {
    bool canOk = (canErrorCount < CAN_ERROR_RESET_THRESHOLD);
    bool sdOk = (sdLoggerGetStatus() == SD_LOG_STATUS_ACTIVE);
    
    // Determine system state
    if (canOk && sdOk) {
        systemState = SYSTEM_STATE_NORMAL;
    } else if (canOk || sdOk) {
        systemState = SYSTEM_STATE_DEGRADED;
        // Log degradation if state changed
        static SystemState lastState = SYSTEM_STATE_INIT;
        if (lastState == SYSTEM_STATE_NORMAL) {
            DEBUG_PRINT("WARN: System degraded - %s failed\n", 
                       canOk ? "SD card" : "CAN bus");
        }
        lastState = systemState;
    } else if (canErrorCount >= CAN_ERROR_RESTART_THRESHOLD) {
        systemState = SYSTEM_STATE_FAILURE;
    } else {
        systemState = SYSTEM_STATE_CRITICAL;
    }
}

void loop() {
  esp_task_wdt_reset();

  // Professional-grade: Update system state
  updateSystemState();

  // PRIORITY 1: CAN communication (highest priority - no delays)
  handleCanRx();  // Process all queued CAN messages first
  
  // PRIORITY 1.5: ERROR #10 FIX - Cleanup timed-out variable requests
  uint32_t nowMs = millis();
  for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
    // Check for pending requests that have timed out
    if (varResponses[i].request_pending && varResponses[i].request_time_ms > 0) {
      uint32_t age = nowMs - varResponses[i].request_time_ms;
      if (age > VAR_RESPONSE_TIMEOUT_MS) {
        // Request timed out - clear pending flag and decrement pending count
        DEBUG_CAN_RX_PRINT("WARN: Variable request timeout for var_id %d (age: %d ms)\n", 
                          varResponses[i].var_id, age);
        varResponses[i].request_pending = false;
        varResponses[i].request_time_ms = 0;
        
        // Decrement pending count (saturate at 0)
        if (pendingRequestCount > 0) {
          pendingRequestCount--;
        }
      }
    }
  }
  
  // PRIORITY 2: CAN variable requests (time-critical)
  
  // Check if we should send another request
  // ERROR #8 FIX: Prevent overflow - explicit wraparound protection
  // Use runtime configuration
  if (pendingRequestCount < runtimeMAX_PENDING && pendingRequestCount < 255) {
    // Time to send next request?
    // Use runtime configuration
    if ((nowMs - lastVarReadTime) >= runtimeVAR_REQUEST_INTERVAL) {
      // Send request for current variable
      int32_t var_id = EPIC_VARIABLES[currentVarIndex].var_id;
      if (requestVar(var_id)) {
        // ERROR #8 FIX: Safe increment with explicit wraparound protection
        if (pendingRequestCount < 255) {
          pendingRequestCount++;
        } else {
          // Counter at maximum, wait for responses
          DEBUG_CAN_TX_PRINT("WARN: Pending request counter at maximum (255), waiting for responses\n");
        }
        
        // ERROR #10 FIX: Track request time and pending status
        varResponses[currentVarIndex].var_id = var_id;
        varResponses[currentVarIndex].request_time_ms = nowMs;
        varResponses[currentVarIndex].request_pending = true;
        
        DEBUG_CAN_TX_PRINT("Request var[%d]: %s (ID: %d), pending: %d\n", 
                           currentVarIndex, EPIC_VARIABLES[currentVarIndex].name, var_id, pendingRequestCount);
        currentVarIndex = (currentVarIndex + 1) % EPIC_VAR_COUNT;
        lastVarReadTime = nowMs;
      } else {
        DEBUG_CAN_TX_PRINT("WARN: Failed to queue request for var[%d]: %s\n", 
                           currentVarIndex, EPIC_VARIABLES[currentVarIndex].name);
      }
    }
  }
  
  // PRIORITY 3: ISO 15765/UDS tasks (non-blocking, quick)
  iso15765_task();  // Process ISO transport layer
  uds_task();       // Process UDS session management

  // PRIORITY 4: USB Host (non-blocking, quick)
  usbHost.task();
  
  // PRIORITY 5: Web server (non-blocking, quick)
  server.handleClient();  // Handle web server requests (non-blocking)

  // PRIORITY 6: SD logger (non-blocking, time-limited flush)
  // Professional-grade: Only attempt if SD is operational (graceful degradation)
  if (systemState != SYSTEM_STATE_FAILURE && sdLoggerGetStatus() == SD_LOG_STATUS_ACTIVE) {
    sdLoggerTask();
  } else if (systemState == SYSTEM_STATE_DEGRADED && sdLoggerGetStatus() != SD_LOG_STATUS_ACTIVE) {
    // Try to recover SD card periodically (every 10 seconds)
    static uint32_t lastSdRetryTime = 0;
    if ((millis() - lastSdRetryTime) > 10000) {
      lastSdRetryTime = millis();
      if (sdLoggerInit()) {
        sdLoggerStart();
        DEBUG_SD_PRINT("SUCCESS: SD card recovered\n");
        updateSystemState();  // Update state after recovery
      }
    }
  }
  
  // PRIORITY 7: Button processing (lowest priority, time-budgeted to not block CAN)
  // Process buttons with time budget to prevent blocking CAN communication
  uint32_t buttonProcessStart = nowMs;
  const uint32_t MAX_BUTTON_PROCESS_TIME_MS = 2;  // Max 2ms for button processing
  
  // Scan buttons with debounce (active-low)
  for (int i = 0; i < BUTTON_COUNT; i++) {
    // Time budget check - exit if taking too long
    if ((millis() - buttonProcessStart) > MAX_BUTTON_PROCESS_TIME_MS) {
      // Skip remaining buttons to maintain CAN responsiveness
      break;
    }
    uint8_t raw = (uint8_t)digitalRead(BUTTON_PINS[i]);
    if (raw != buttonStates[i].lastReadLevel) {
      buttonStates[i].lastReadLevel = raw;
      buttonStates[i].lastChangeMs = nowMs;
    }
    if ((nowMs - buttonStates[i].lastChangeMs) >= DEBOUNCE_MS) {
      if (raw != buttonStates[i].stableLevel) {
        // Stable state changed after debounce
        uint8_t prev = buttonStates[i].stableLevel;
        buttonStates[i].stableLevel = raw;
        // Detect press edge: HIGH->LOW (with pullup)
        if (prev == HIGH && raw == LOW) {
          if (i == LONG_PRESS_BUTTON_INDEX) {
            // For GPIO 19 (button index 0): arm long-press timer for activation; do not send yet
            buttonStates[i].holdStartMs = nowMs;
            buttonStates[i].longFired = 0;
          } else {
            // Other buttons: short press sends immediately
            buttonStates[i].pressed = 1;
          }
        }
        // Detect release edge: LOW->HIGH
        if (prev == LOW && raw == HIGH) {
          if (i == LONG_PRESS_BUTTON_INDEX) {
            // If released before long press fired, treat as short press -> de-activate if active
            if (!buttonStates[i].longFired && buttonStates[i].active) {
              uint8_t key = BUTTON_HID_CODES[i];
              uint16_t encoded = (uint16_t)key; // modifier = 0
              DEBUG_BTN_PRINT("GPIO %d short release -> Deactivate (HID 0x%02X)\n", BUTTON_PINS[i], key);
              ledGreen();
              (void)sendCMD((uint8_t)(encoded & 0xFF), (uint8_t)((encoded >> 8) & 0xFF));
              ledOff();
              buttonStates[i].active = 0;
            }
            buttonStates[i].longFired = 0;
          } else {
          buttonStates[i].longFired = 0;
          }
        }
      }
    }
    // While held LOW, check for long-press timeout on the designated button
    if (i == LONG_PRESS_BUTTON_INDEX && buttonStates[i].stableLevel == LOW) {
      if (!buttonStates[i].longFired && (nowMs - buttonStates[i].holdStartMs) >= LONG_PRESS_MS) {
        // Long press -> Activate (send once if not already active)
        if (!buttonStates[i].active) {
          uint8_t key = BUTTON_HID_CODES[i];
          uint16_t encoded = (uint16_t)key; // modifier = 0
          DEBUG_BTN_PRINT("GPIO %d long press -> Activate (HID 0x%02X)\n", BUTTON_PINS[i], key);
          ledGreen();
          (void)sendCMD((uint8_t)(encoded & 0xFF), (uint8_t)((encoded >> 8) & 0xFF));
          ledOff();
          buttonStates[i].active = 1;
        }
        buttonStates[i].longFired = 1;
      }
    }
    // Handle immediate-send buttons (non-GPIO19 buttons, index 0 is long-press)
    if (buttonStates[i].pressed) {
      buttonStates[i].pressed = 0;
      // Send CAN message representing this button as a key press (no modifiers)
      uint8_t key = BUTTON_HID_CODES[i];
      uint16_t encoded = (uint16_t)key; // modifier = 0, so MSB=0
      DEBUG_BTN_PRINT("Button GPIO %d pressed -> HID 0x%02X\n", BUTTON_PINS[i], key);
      ledGreen();
      bool sent = sendCMD((uint8_t)(encoded & 0xFF), (uint8_t)((encoded >> 8) & 0xFF));
      if (!sent) {
        DEBUG_BTN_PRINT("WARN: CAN send failed for button GPIO %d\n", BUTTON_PINS[i]);
      }
      ledOff();
    }
  }
}

