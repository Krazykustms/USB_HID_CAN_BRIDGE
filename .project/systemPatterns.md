# System Patterns: USB_HID_CAN_BRIDGE / EPIC CAN Logger

## System Architecture

```
┌──────────────┐      USB OTG       ┌─────────────────────────────────┐      CAN Bus      ┌──────────────┐
│ USB Keyboard │ ──────────────────> │      ESP32-S3-USB-OTG           │ ─────────────────> │ EPIC ECU     │
└──────────────┘    HID Reports      │                                 │   0x700+ECU_ID    │ rusEFI ECU    │
                                      │  - EspUsbHost2                  │   0x711 (buttons) │ Diagnostic   │
┌──────────────┐    WiFi Connection  │  - TWAI/CAN                     │   512-522 (DBC)   │   Tools      │
│ Web Browser  │ ──────────────────> │  - WiFi AP                      │   0x7DF (ISO)     └──────────────┘
└──────────────┘                      │  - Web Server                   │
                                      │  - SD Logger                    │
                                      │  - Config Manager               │
                                      │  - ISO/UDS (ISO version)        │
                                      └─────────────────────────────────┘
                                             │
                                      ┌──────▼──────┐
                                      │  SD Card    │
                                      │  (Logs)     │
                                      └─────────────┘
```

## Key Technical Decisions

### Multi-Protocol CAN Support
- **Decision**: Support EPIC, rusEFI DBC, and ISO 15765 simultaneously
- **Rationale**: Maximum compatibility and flexibility
- **Pattern**: Message routing by CAN ID in `handleCanRx()`

### Request Pipelining
- **Decision**: Up to 16 concurrent EPIC variable requests
- **Rationale**: Maximizes throughput without overwhelming ECU
- **Pattern**: Track pending requests, process out-of-order responses

### Graceful Degradation
- **Decision**: System continues operation if SD card fails
- **Rationale**: Data logging is important but not critical for system function
- **Pattern**: State machine tracks system health, subsystems can fail independently

### Progressive Error Recovery
- **Decision**: Multi-stage CAN error recovery (reset → aggressive reset → restart)
- **Rationale**: Avoids unnecessary restarts while maintaining reliability
- **Pattern**: Error count thresholds trigger progressively more aggressive recovery

### Runtime Configuration
- **Decision**: ESP32 Preferences (EEPROM) for persistent storage
- **Rationale**: Built-in, reliable, no external components needed
- **Pattern**: Load on startup, validate, apply to runtime variables

### Dual Version Strategy
- **Decision**: Separate .ino files for standard and ISO versions
- **Rationale**: Users choose based on needs; ISO adds overhead
- **Pattern**: Shared modules (.h/.cpp), version-specific includes and routing

## Design Patterns in Use

### Observer Pattern
- `MyEspUsbHost` class observes USB events via callbacks
- `onReceive()`: Triggered when HID data arrives
- `onGone()`: Triggered when device disconnects
- ISO 15765 processes CAN frames and triggers UDS handlers

### State Machine Pattern
- **System States**: INIT → NORMAL → DEGRADED → CRITICAL → FAILURE
- **State Transitions**: Based on subsystem health (CAN, SD)
- **State Actions**: Different behavior based on current state

### Priority-Based Scheduling
- Tasks organized by priority in main loop
- Critical tasks (CAN) always run first
- Non-critical tasks (buttons) time-budgeted

### Ring Buffer Pattern
- SD logger uses ring buffer for writes
- Decouples write timing from CAN message timing
- Reduces SD card wear through batch writes

### Time Budgeting Pattern
```cpp
uint32_t startTime = millis();
const uint32_t MAX_TIME_MS = 10;
while (work_to_do && (millis() - startTime) < MAX_TIME_MS) {
    // do work
    if (time_limit_exceeded) break;
}
```
Used for CAN processing, SD flushing, button scanning.

### Progressive Recovery Pattern
```cpp
if (errors > THRESHOLD_3) {
    restart();
} else if (errors > THRESHOLD_2) {
    aggressive_reset();
} else if (errors > THRESHOLD_1) {
    soft_reset();
}
```

### Configuration Validation Pattern
```cpp
if (!configValidate(&config)) {
    return false; // reject invalid config
}
configSave(&config); // only save if valid
```

### ISO Message Routing Pattern
```cpp
if (can_id == ISO_REQUEST_ID) {
    iso15765_process_rx(&frame);
    if (message_complete) {
        uds_process_request(...);
        iso15765_send_response(...);
    }
}
```

## Component Relationships

### USB → Processing → CAN Flow
1. **USB Layer** (`EspUsbHost2`): Receives HID reports from keyboard
2. **Processing Layer** (`MyEspUsbHost::onReceive`): Parses report, extracts keys/modifiers
3. **Encoding Layer** (inline): Applies modifier encoding to keycodes
4. **Transmission Layer** (`sendCMD()`): Formats and sends CAN message
5. **CAN Layer** (`ESP32-TWAI-CAN`): Manages physical CAN bus transmission

### EPIC Variable Reading Flow
1. **Request Layer**: Sends variable request (0x700+ECU_ID) with pipelining
2. **Response Layer**: Receives variable response (0x720+ECU_ID), stores by variable ID
3. **Logging Layer**: Writes to SD card with sequence number and CRC16
4. **Display Layer**: Updates web dashboard variables (TPS, RPM, AFR)

### rusEFI DBC Decoding Flow
1. **Receive Layer**: Captures CAN broadcasts (IDs 512-522)
2. **Decode Layer**: Parses DBC format (bit extraction, sign extension, scaling)
3. **Storage Layer**: Stores decoded signals
4. **Logging Layer**: Writes to SD card
5. **Display Layer**: Updates web dashboard (RPM, TPS, AFR from DBC)

### ISO 15765/UDS Flow
1. **Transport Layer**: Receives ISO 15765 frames (0x7DF)
2. **Reassembly Layer**: Combines multi-frame messages
3. **Service Layer**: Routes to UDS service handlers
4. **Response Layer**: Formats UDS response
5. **Transport Layer**: Sends response via ISO 15765 (0x7E8+ECU_ID)

### Configuration Flow
1. **Load Layer**: Reads from ESP32 Preferences on startup
2. **Validation Layer**: Validates configuration values
3. **Application Layer**: Applies to runtime variables
4. **Web Layer**: Allows viewing/changing via web interface
5. **Save Layer**: Writes validated config back to Preferences

### Error Monitoring Flow
1. **Detection Layer**: Monitors CAN state, transmission failures, bus errors
2. **Counting Layer**: Tracks error counts per subsystem
3. **Assessment Layer**: Updates system state based on health
4. **Recovery Layer**: Progressive recovery actions based on error level
5. **Monitoring Layer**: Continuous health assessment in main loop

## Critical Implementation Paths

### Initialization Sequence (setup)
```
1. Serial.begin(115200)
2. Watchdog timer initialization
3. Load runtime configuration from EEPROM
4. Initialize CAN (with runtime config)
5. Initialize ISO 15765/UDS (ISO version only)
6. Initialize USB Host
7. Initialize GPIO buttons
8. Initialize shift light GPIO
9. Initialize WiFi AP and web server
10. Initialize SD card logger (graceful on failure)
11. Initialize variable response tracking
12. Update system state
```

### Main Loop Pattern
```
1. Reset watchdog
2. Update system state
3. PRIORITY 1: Process CAN RX (handleCanRx)
4. PRIORITY 2: Send EPIC variable requests
5. PRIORITY 3: ISO/UDS tasks (ISO version) or USB Host
6. PRIORITY 4: USB Host or Web server
7. PRIORITY 5: Web server (if not PRIORITY 4)
8. PRIORITY 6: SD logger (time-budgeted)
9. PRIORITY 7: Button processing (time-budgeted)
```

### CAN Message Routing (handleCanRx)
```
1. Check CAN RX queue
2. Apply time budget (10ms max)
3. For each message:
   a. Route ISO messages (0x7DF) → ISO 15765 handler
   b. Route rusEFI DBC (512-522) → DBC decoder
   c. Route EPIC responses (0x720+ECU_ID) → Variable handler
4. Process ISO/UDS if message complete
5. Decode DBC signals
6. Store EPIC variable values
7. Exit early if time budget exceeded
```

### EPIC Variable Request Cycle
```
1. Check pending request count (< MAX_PENDING)
2. Check time since last request (>= INTERVAL)
3. Get next variable from EPIC_VARIABLES array
4. Send CAN request (0x700+ECU_ID)
5. Increment pending request count
6. Advance to next variable (circular)
7. Wait for response (handled in handleCanRx)
8. On response: store value, decrement pending, log to SD
```

### SD Logging Flow
```
1. Variable response received
2. Format log entry: timestamp,sequence,var_id,value,checksum
3. Write to ring buffer
4. In main loop: Flush buffer to SD (time-budgeted, 5ms max)
5. Check file size, rotate if needed
6. Handle errors gracefully (degraded state)
```

### ISO 15765 Multi-Frame Reception
```
1. Receive First Frame (FF) with length
2. Send Flow Control (Continue to Send)
3. Receive Consecutive Frames (CF) with sequence numbers
4. Reassemble complete message
5. Validate sequence, check timeout
6. When complete: Process UDS request
7. Send UDS response (single or multi-frame)
```

### Web Configuration Flow
```
1. Client sends GET /config → Return current config (JSON)
2. Client sends POST /config/save → Validate parameters
3. Validate all configuration values
4. Save to ESP32 Preferences
5. Update runtime variables
6. Return success/error response
7. Some changes require restart (CAN speed, ECU ID)
```

## Code Organization Patterns

### Module Separation
- **Core Logic**: Main .ino file
- **SD Logging**: `sd_logger.h/.cpp`
- **DBC Parsing**: `rusefi_dbc.h/.cpp`
- **Configuration**: `config_manager.h/.cpp`
- **ISO Transport**: `iso15765.h/.cpp` (ISO version only)
- **UDS Services**: `uds.h/.cpp` (ISO version only)

### Version Strategy
- **Shared Code**: All .h/.cpp modules
- **Version-Specific**: Main .ino files differ in includes and routing
- **Conditional Compilation**: Debug macros, feature flags

### Data Structures
- **VarResponse**: Tracks EPIC variable values with timestamps
- **ButtonState**: Tracks button debounce and long-press state
- **SystemConfig**: Runtime configuration structure
- **rusefi_baseX_t**: DBC decoded signal structures

## Performance Optimization Patterns

### Inline Functions
- Small utility functions marked `inline`
- Reduces function call overhead
- Used for LED control, bit manipulation

### Direct Memory Access
- Prefer `memcpy()` over loops
- Use struct overlays for CAN data
- Avoid unnecessary copies

### Pre-computed Constants
- Signal IDs calculated at compile time
- No runtime string operations
- Lookup tables for HID codes

### Time Budgeting
- All non-critical tasks have execution time limits
- Prevents blocking critical CAN communication
- Maintains real-time responsiveness

