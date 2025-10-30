#include <Arduino.h>
#include <EspUsbHost.h>
#include <Adafruit_NeoPixel.h>
#include "esp_task_wdt.h"
#include <ESP32-TWAI-CAN.hpp>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ------------------------------
// Configuration Constants
// ------------------------------

// GPIO pins
#define CAN_TX_PIN                 5
#define CAN_RX_PIN                 4
#define LED_NEOPIXEL_PIN           48
#define SHIFT_LIGHT_GPIO           14  // Dedicated GPIO for shift light LED

// CAN protocol constants
#define CAN_ADDRESS_BUTTONBOX      0x711
#define CAN_MAGIC_BYTE             0x5A
#define CAN_BUTTON_BOX_ID          27
#define CAN_SPEED_KBPS             500

// EPIC CAN protocol constants for variable reading
#define CAN_ID_GET_VAR_REQ_BASE    0x700  // + ecuId
#define CAN_ID_GET_VAR_RES_BASE    0x720  // + ecuId
#define VAR_ID_TPS_VALUE           1272048601  // TPSValue variable ID
#define VAR_ID_RPM_VALUE           1699696209  // RPMValue variable ID
#define VAR_ID_AFR_VALUE           -1093429509 // AFRValue (O2 sensor) variable ID
#define ECU_ID                     1           // Target ECU ID
#define VAR_READ_INTERVAL_MS       1000        // Read every second

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
static const int BUTTON_PINS[BUTTON_COUNT] = { 6, 7, 8, 9, 10, 11, 12, 13 };
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
#define LONG_PRESS_BUTTON_INDEX     4   // Index in BUTTON_PINS / BUTTON_HID_CODES (GPIO 10)
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

// Variable reading state
static uint32_t lastVarReadTime = 0;
static uint8_t requestCounter = 0;  // Counter to cycle through TPS, RPM, and AFR

// WiFi and web server
WebServer server(80);
static volatile float tpsValue = 0.0;
static volatile float rpmValue = 0.0;
static volatile float afrValue = 0.0;
static volatile bool shiftLightActive = false;

typedef struct {
  uint8_t stableLevel;      // last stable level (HIGH with pullup = not pressed)
  uint8_t lastReadLevel;    // last raw read
  uint32_t lastChangeMs;    // last time raw changed
  uint8_t pressed;          // edge-detected pressed flag (cleared after handled)
  uint32_t holdStartMs;     // when LOW became stable (for long press)
  uint8_t longFired;        // prevent repeat firing on long press
  uint8_t active;           // latched active state (used for GPIO 10 long/short behavior)
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
  ESP32Can.setSpeed(ESP32Can.convertSpeed(CAN_SPEED_KBPS));

  int retries = CAN_INIT_MAX_RETRIES;
  while (!ESP32Can.begin() && retries > 0) {
    Serial.println("CAN init failed, retrying...");
    ledRed();
    delay(500);
    retries--;
  }
  if (retries == 0) {
    Serial.println("FATAL: CAN initialization failed");
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
  }

  // Track error
  canErrorCount++;
  if (canErrorCount > ERROR_THRESHOLD) {
    Serial.println("Too many CAN errors, restarting...");
    ESP.restart();
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
      Serial.println("Invalid transfer buffer");
      return;
    }

    // Dump limited bytes for debug
    size_t toPrint = min((int)transfer->data_buffer_size, 50);
    for (size_t i = 0; i < toPrint; i++) {
      Serial.printf("%02x ", transfer->data_buffer[i]);
    }
    Serial.println();

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
    Serial.println("USB device gone");
  }

  void onConnected() {
    deviceGone = 0;
    Serial.println("USB device connected");
  }
};

MyEspUsbHost usbHost;

// ------------------------------
// Web server handlers
// ------------------------------

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

void initWiFi() {
  // Start WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, false, WIFI_MAX_CONNECTIONS);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("WiFi AP started");
  Serial.print("AP SSID: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Start web server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started at http://" + IP.toString());
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

  // Initialize CAN
  if (!beginCanWithRetry()) {
    // Fatal: leave LED red and halt
    while (true) {
      delay(1000);
    }
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

  // Print variable reading info
  Serial.println("keyboard_basic1 initialized");
  Serial.printf("Reading variables from ECU %d every second:\n", ECU_ID);
  Serial.printf("  - TPSValue (ID %d)\n", VAR_ID_TPS_VALUE);
  Serial.printf("  - RPMValue (ID %d) - Shift light at %d rpm\n", VAR_ID_RPM_VALUE, SHIFT_LIGHT_RPM_THRESHOLD);
  Serial.printf("  - AFRValue (ID %d)\n", VAR_ID_AFR_VALUE);
}

static void handleCanRx() {
  CanFrame rx;
  while (ESP32Can.inRxQueue() > 0) {
    if (ESP32Can.readFrame(rx, 0)) {
      // Check if this is a variable response from our target ECU
      if ((rx.identifier & 0x7F0) == CAN_ID_GET_VAR_RES_BASE && rx.data_length_code == 8) {
        if ((rx.identifier & 0x0F) == (ECU_ID & 0x0F)) {
          int32_t received_var_id = be_i32(rx.data);
          
          if (received_var_id == VAR_ID_TPS_VALUE) {
            float value = be_f32(&rx.data[4]);
            tpsValue = value;  // Store for web display
            Serial.printf("Variable ID %d (TPSValue): %.6f\n", received_var_id, value);
          }
          else if (received_var_id == VAR_ID_RPM_VALUE) {
            float rpm = be_f32(&rx.data[4]);
            rpmValue = rpm;  // Store for web display
            Serial.printf("Variable ID %d (RPMValue): %.1f rpm\n", received_var_id, rpm);
            
            // Shift light logic: activate if RPM >= 4000
            if (rpm >= SHIFT_LIGHT_RPM_THRESHOLD) {
              shiftLightOn();   // Activate shift light on dedicated GPIO
              shiftLightActive = true;  // Set flag for web display
              Serial.println("SHIFT LIGHT: ON");
            } else {
              shiftLightOff();  // Turn off shift light
              shiftLightActive = false;  // Clear flag
            }
          }
          else if (received_var_id == VAR_ID_AFR_VALUE) {
            float afr = be_f32(&rx.data[4]);
            afrValue = afr;  // Store for web display
            Serial.printf("Variable ID %d (AFRValue): %.2f\n", received_var_id, afr);
          }
        }
      }
    }
  }
}

void loop() {
  esp_task_wdt_reset();

  usbHost.task();
  handleCanRx();
  server.handleClient();  // Handle web server requests

  // Periodic variable reading every second - cycle through TPS, RPM, and AFR
  uint32_t nowMs = millis();
  if (nowMs - lastVarReadTime >= VAR_READ_INTERVAL_MS) {
    bool success = false;
    if (requestCounter == 0) {
      success = requestVar(VAR_ID_TPS_VALUE);
    } else if (requestCounter == 1) {
      success = requestVar(VAR_ID_RPM_VALUE);
    } else {
      success = requestVar(VAR_ID_AFR_VALUE);
    }

    // Always advance the timer to avoid spamming the bus on failures
    lastVarReadTime = nowMs;
    if (success) {
      requestCounter = (requestCounter + 1) % 3;  // Cycle through 0, 1, 2 (TPS, RPM, AFR)
    }
  }

  // Scan buttons with debounce (active-low)
  for (int i = 0; i < BUTTON_COUNT; i++) {
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
            // For GPIO 10: arm long-press timer for activation; do not send yet
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
              Serial.printf("GPIO %d short release -> Deactivate (HID 0x%02X)\n", BUTTON_PINS[i], key);
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
          Serial.printf("GPIO %d long press -> Activate (HID 0x%02X)\n", BUTTON_PINS[i], key);
          ledGreen();
          (void)sendCMD((uint8_t)(encoded & 0xFF), (uint8_t)((encoded >> 8) & 0xFF));
          ledOff();
          buttonStates[i].active = 1;
        }
        buttonStates[i].longFired = 1;
      }
    }
    // Handle immediate-send buttons (non-GPIO10 buttons)
    if (buttonStates[i].pressed) {
      buttonStates[i].pressed = 0;
      // Send CAN message representing this button as a key press (no modifiers)
      uint8_t key = BUTTON_HID_CODES[i];
      uint16_t encoded = (uint16_t)key; // modifier = 0, so MSB=0
      Serial.printf("Button GPIO %d pressed -> HID 0x%02X\n", BUTTON_PINS[i], key);
      ledGreen();
      bool sent = sendCMD((uint8_t)(encoded & 0xFF), (uint8_t)((encoded >> 8) & 0xFF));
      if (!sent) {
        Serial.println("WARN: CAN send not queued");
      }
      ledOff();
    }
  }
}


