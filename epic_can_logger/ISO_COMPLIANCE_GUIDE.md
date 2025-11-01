# ISO 14229/15765 Compliance Implementation Guide

## Overview

### ISO 14229 (UDS - Unified Diagnostic Services)
**Purpose**: Standardized diagnostic services for vehicle ECUs  
**Scope**: Defines service IDs, request/response formats, error handling

### ISO 15765 (DoCAN - Diagnostics on CAN)
**Purpose**: Transport protocol for diagnostics over CAN bus  
**Scope**: Multi-frame message handling, flow control, timing

---

## Why ISO Compliance Matters

### Current System
- ✅ Custom EPIC protocol (works, but proprietary)
- ⚠️ Not compatible with standard diagnostic tools
- ⚠️ Cannot integrate with OEM diagnostic systems

### With ISO Compliance
- ✅ Works with standard diagnostic tools (e.g., CANoe, Vector tools)
- ✅ Compatible with OEM systems
- ✅ Industry-standard protocol
- ✅ Future-proof for integration

### When You Need It
- **Required**: Automotive OEM integration, commercial diagnostic tools
- **Optional**: Custom/internal applications, proprietary tools

---

## ISO 14229 (UDS) Overview

### Core Services Required

| Service ID | Name | Purpose | Required |
|------------|------|---------|----------|
| **0x10** | DiagnosticSessionControl | Switch diagnostic sessions | ✅ Yes |
| **0x11** | ECUReset | Reset ECU | ✅ Yes |
| **0x22** | ReadDataByIdentifier | Read variable/parameter | ✅ Yes |
| **0x2E** | WriteDataByIdentifier | Write parameter | ⚠️ Optional |
| **0x27** | SecurityAccess | Authentication | ⚠️ Optional |
| **0x3E** | TesterPresent | Keep session alive | ✅ Yes |
| **0x19** | ReadDTCInformation | Read diagnostic trouble codes | ⚠️ Optional |
| **0x14** | ClearDiagnosticInformation | Clear DTCs | ⚠️ Optional |

### Message Format

**Request Format**:
```
[Service ID] [Sub-function/Data Identifier] [Data...]
```

**Response Format**:
```
[Service ID + 0x40] [Sub-function] [Data...]  // Positive response
[0x7F] [Service ID] [Negative Response Code] // Negative response
```

**Example - ReadDataByIdentifier (0x22)**:
```
Request:  22 F1 90  (Read DID 0xF190)
Response: 62 F1 90 12 34 56 78  (Positive: value = 0x12345678)
Response: 7F 22 31  (Negative: RequestOutOfRange)
```

---

## ISO 15765 (DoCAN) Overview

### Single-Frame Messages
For messages ≤ 8 bytes (including protocol control info):

**Format**:
```
Byte 0: [PCI Type (0x00-0x07) | Length (0-7)]
Byte 1-7: [Data...]
```

**Example**: 8-byte data payload
```
[07] [data0] [data1] ... [data6]
 ^
Single frame, 7 bytes data
```

### Multi-Frame Messages
For messages > 8 bytes:

**First Frame (FF)**:
```
[1X YY YY YY] [data0] [data1] [data2] [data3] [data4] [data5]
 ^  ^  ^  ^
 |  |  |  Total length (12-4095 bytes)
 |  |  Length high byte
 |  Length mid byte
First frame, X = (length >> 8) & 0x0F
```

**Consecutive Frames (CF)**:
```
[2X] [data0] [data1] [data2] [data3] [data4] [data5] [data6]
 ^
Sequence number (0-15, wraps)
```

**Flow Control (FC)**:
```
[3X] [BS] [STmin]
 ^   ^    ^
 |   |    Separation time min (ms)
 |   Block size (0 = send all, FF = wait for FC)
Flow control type:
  0 = Continue to send
  1 = Wait
  2 = Overflow
  3 = ContinueToSend
```

### Timing Requirements

| Parameter | Value | Notes |
|-----------|-------|-------|
| **N_As** | 1000ms | Request timeout |
| **N_Ar** | 5000ms | Response timeout |
| **N_Bs** | 1000ms | Block size timeout |
| **N_Cr** | 1000ms | Consecutive frame timeout |
| **STmin** | 0-127ms | Minimum separation time |
| **BS** | 0-255 | Block size (0 = send all) |

---

## Implementation Architecture

### Required Components

```
┌─────────────────────────────────────────┐
│  Application Layer (UDS Services)      │
│  - Service handlers (0x10, 0x22, etc.) │
│  - Session management                    │
│  - Security access                       │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  Session Layer (ISO 15765-2)           │
│  - Multi-frame handling                  │
│  - Flow control                          │
│  - Sequence numbers                      │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  Transport Layer (ISO 15765-4)        │
│  - CAN message formatting                │
│  - Physical addressing                   │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│  CAN Bus (Physical Layer)               │
└─────────────────────────────────────────┘
```

---

## Implementation Plan

### Phase 1: Transport Layer (ISO 15765-4)
**Time**: 1-2 weeks  
**Priority**: Must have first

**Components**:
1. **Single-frame handler** (easy)
2. **Multi-frame handler** (complex)
3. **Flow control logic** (moderate)
4. **Timing management** (moderate)

### Phase 2: Session Layer (ISO 15765-2)
**Time**: 1 week  
**Priority**: Required for multi-frame

**Components**:
1. **Session state machine**
2. **Message reassembly**
3. **Sequence number tracking**
4. **Timeout management**

### Phase 3: UDS Services (ISO 14229)
**Time**: 2-3 weeks  
**Priority**: Core functionality

**Required Services**:
1. **DiagnosticSessionControl (0x10)**
2. **ECUReset (0x11)**
3. **ReadDataByIdentifier (0x22)** ⭐ Maps to EPIC variables
4. **TesterPresent (0x3E)**

### Phase 4: Advanced Services (Optional)
**Time**: 1-2 weeks  
**Priority**: Enhanced functionality

**Optional Services**:
1. **SecurityAccess (0x27)**
2. **WriteDataByIdentifier (0x2E)**
3. **ReadDTCInformation (0x19)**
4. **RoutineControl (0x31)**

---

## Code Structure Required

### 1. ISO 15765 Transport Layer

```cpp
// iso15765.h
#ifndef ISO15765_H
#define ISO15765_H

#include <stdint.h>
#include <stdbool.h>

// ISO 15765-4 addressing
#define ISO_15765_PHYSICAL_REQUEST  0x7DF   // Physical functional request
#define ISO_15765_PHYSICAL_RESPONSE 0x7E8   // Physical response base (ECU ID added)

// ISO 15765-2 PCI types
#define PCI_SINGLE_FRAME       0x00
#define PCI_FIRST_FRAME        0x10
#define PCI_CONSECUTIVE_FRAME  0x20
#define PCI_FLOW_CONTROL       0x30

// Timing (ms)
#define ISO_N_As     1000  // Request timeout
#define ISO_N_Ar     5000  // Response timeout
#define ISO_N_Bs     1000  // Block size timeout
#define ISO_N_Cr     1000  // Consecutive frame timeout

// Functions
bool iso15765_send_single(uint8_t* data, uint8_t length, uint32_t can_id);
bool iso15765_send_multi(uint8_t* data, uint16_t length, uint32_t can_id);
bool iso15765_receive_single(uint8_t* data, uint8_t* length, uint32_t* can_id);
bool iso15765_receive_multi(uint8_t* data, uint16_t* length, uint32_t* can_id);

#endif // ISO15765_H
```

### 2. UDS Service Layer

```cpp
// uds.h
#ifndef UDS_H
#define UDS_H

#include <stdint.h>
#include <stdbool.h>

// UDS Service IDs
#define UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL  0x10
#define UDS_SERVICE_ECU_RESET                   0x11
#define UDS_SERVICE_READ_DATA_BY_IDENTIFIER      0x22
#define UDS_SERVICE_WRITE_DATA_BY_IDENTIFIER     0x2E
#define UDS_SERVICE_SECURITY_ACCESS              0x27
#define UDS_SERVICE_TESTER_PRESENT               0x3E
#define UDS_SERVICE_READ_DTC_INFORMATION         0x19

// UDS Negative Response Codes (NRC)
#define NRC_POSITIVE_RESPONSE          0x00
#define NRC_GENERAL_REJECT             0x10
#define NRC_SERVICE_NOT_SUPPORTED      0x11
#define NRC_SUBFUNCTION_NOT_SUPPORTED 0x12
#define NRC_INCORRECT_MESSAGE_LENGTH  0x13
#define NRC_RESPONSE_TOO_LONG         0x14
#define NRC_BUSY_REPEAT_REQUEST       0x21
#define NRC_CONDITIONS_NOT_CORRECT   0x22
#define NRC_REQUEST_SEQUENCE_ERROR   0x24
#define NRC_NO_RESPONSE_FROM_SUBNET  0x25
#define NRC_FAILURE_PREVENT_EXECUTION 0x26
#define NRC_REQUEST_OUT_OF_RANGE     0x31
#define NRC_SECURITY_ACCESS_DENIED   0x33
#define NRC_INVALID_KEY             0x35
#define NRC_EXCEED_NUMBER_OF_ATTEMPTS 0x36
#define NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED 0x37

// Diagnostic sessions
#define UDS_SESSION_DEFAULT           0x01
#define UDS_SESSION_PROGRAMMING       0x02
#define UDS_SESSION_EXTENDED          0x03

// Data Identifiers (DIDs) - map to EPIC variables
#define DID_TPS_VALUE        0xF190
#define DID_RPM_VALUE        0xF191
#define DID_AFR_VALUE        0xF192
// Add more as needed

// Functions
bool uds_process_request(uint8_t* request, uint8_t request_len, 
                         uint8_t* response, uint8_t* response_len);
bool uds_handle_read_data_by_id(uint16_t did, uint8_t* data, uint8_t* length);
bool uds_handle_diagnostic_session_control(uint8_t session);
bool uds_handle_tester_present();

#endif // UDS_H
```

### 3. Integration with Current System

```cpp
// In epic_can_logger.ino

// Add ISO 15765 support
#include "iso15765.h"
#include "uds.h"

// CAN ID configuration for ISO compliance
#define ISO_PHYSICAL_REQUEST_ID  0x7DF   // Functional request (all ECUs)
#define ISO_PHYSICAL_RESPONSE_ID (0x7E8 + ECU_ID)  // Physical response (this ECU)

// Session management
static uint8_t currentDiagnosticSession = UDS_SESSION_DEFAULT;
static uint32_t lastTesterPresentTime = 0;

// In handleCanRx() - add ISO message handling
void handleCanRx() {
    CanFrame rx;
    
    while (ESP32Can.inRxQueue() > 0) {
        if (ESP32Can.readFrame(rx, 0)) {
            // Check for ISO 15765 messages
            if (rx.identifier == ISO_PHYSICAL_REQUEST_ID || 
                rx.identifier == (ISO_PHYSICAL_REQUEST_ID + ECU_ID)) {
                
                // Process ISO/UDS message
                uint8_t response[4095];  // Max UDS message size
                uint16_t response_len = 0;
                
                if (iso15765_receive_multi(rx.data, &response_len, &rx.identifier)) {
                    // Process UDS service
                    uint8_t uds_response[4095];
                    uint16_t uds_response_len = 0;
                    
                    if (uds_process_request(response, response_len, 
                                            uds_response, &uds_response_len)) {
                        // Send response via ISO 15765
                        iso15765_send_multi(uds_response, uds_response_len, 
                                          ISO_PHYSICAL_RESPONSE_ID);
                    }
                }
            }
            
            // Continue with existing EPIC/DBC message handling...
        }
    }
}
```

---

## Detailed Service Implementation

### 1. DiagnosticSessionControl (0x10)

**Purpose**: Switch between diagnostic sessions

**Request Format**:
```
[0x10] [Sub-function]
```

**Sub-functions**:
- 0x01: Default session
- 0x02: Programming session
- 0x03: Extended diagnostic session

**Response Format**:
```
[0x50] [Sub-function] [Session parameters...]
```

**Implementation**:
```cpp
bool uds_handle_diagnostic_session_control(uint8_t* request, uint8_t req_len,
                                          uint8_t* response, uint8_t* resp_len) {
    if (req_len < 2) {
        // Send negative response
        response[0] = 0x7F;
        response[1] = 0x10;  // Service ID
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t sub_function = request[1];
    
    // Validate sub-function
    if (sub_function > 0x03) {
        response[0] = 0x7F;
        response[1] = 0x10;
        response[2] = NRC_SUBFUNCTION_NOT_SUPPORTED;
        *resp_len = 3;
        return false;
    }
    
    // Switch session
    currentDiagnosticSession = sub_function;
    
    // Positive response
    response[0] = 0x50;  // 0x10 + 0x40
    response[1] = sub_function;
    // Add session parameters if needed
    *resp_len = 2;
    
    return true;
}
```

### 2. ReadDataByIdentifier (0x22) ⭐ KEY SERVICE

**Purpose**: Read variable value (maps to EPIC variable reading)

**Request Format**:
```
[0x22] [DID_High] [DID_Low]
```

**Response Format**:
```
[0x62] [DID_High] [DID_Low] [Data...]
```

**DID Mapping** (Map EPIC variables to DIDs):
```cpp
// Map EPIC variable IDs to UDS DIDs
#define DID_TPS_VALUE        0xF190
#define DID_RPM_VALUE        0xF191
#define DID_AFR_VALUE        0xF192

// Variable ID to DID mapping
static const struct {
    int32_t epic_var_id;
    uint16_t uds_did;
} VAR_DID_MAP[] = {
    {VAR_ID_TPS_VALUE, DID_TPS_VALUE},
    {VAR_ID_RPM_VALUE, DID_RPM_VALUE},
    {VAR_ID_AFR_VALUE, DID_AFR_VALUE},
    // Add more mappings...
};

bool uds_handle_read_data_by_id(uint8_t* request, uint8_t req_len,
                                uint8_t* response, uint8_t* resp_len) {
    if (req_len < 3) {
        response[0] = 0x7F;
        response[1] = 0x22;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    // Extract DID
    uint16_t did = ((uint16_t)request[1] << 8) | request[2];
    
    // Find corresponding EPIC variable
    int32_t epic_var_id = 0;
    for (uint8_t i = 0; i < sizeof(VAR_DID_MAP)/sizeof(VAR_DID_MAP[0]); i++) {
        if (VAR_DID_MAP[i].uds_did == did) {
            epic_var_id = VAR_DID_MAP[i].epic_var_id;
            break;
        }
    }
    
    if (epic_var_id == 0) {
        // DID not found
        response[0] = 0x7F;
        response[1] = 0x22;
        response[2] = NRC_REQUEST_OUT_OF_RANGE;
        *resp_len = 3;
        return false;
    }
    
    // Get current value (from varResponses array)
    float value = 0.0;
    bool found = false;
    for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
        if (EPIC_VARIABLES[i].var_id == epic_var_id && varResponses[i].valid) {
            value = varResponses[i].value;
            found = true;
            break;
        }
    }
    
    if (!found) {
        // Value not available yet
        response[0] = 0x7F;
        response[1] = 0x22;
        response[2] = NRC_CONDITIONS_NOT_CORRECT;
        *resp_len = 3;
        return false;
    }
    
    // Positive response with value
    response[0] = 0x62;  // 0x22 + 0x40
    response[1] = (did >> 8) & 0xFF;
    response[2] = did & 0xFF;
    
    // Encode float as 4 bytes (big-endian)
    uint8_t* value_bytes = (uint8_t*)&value;
    response[3] = value_bytes[3];
    response[4] = value_bytes[2];
    response[5] = value_bytes[1];
    response[6] = value_bytes[0];
    *resp_len = 7;
    
    return true;
}
```

### 3. TesterPresent (0x3E)

**Purpose**: Keep diagnostic session alive (must be sent every 5 seconds)

**Request Format**:
```
[0x3E] [Sub-function]  // Sub-function: 0x00 = suppress response, 0x01 = send response
```

**Response Format**:
```
[0x7E] [Sub-function]  // Only if sub-function = 0x01
```

**Implementation**:
```cpp
bool uds_handle_tester_present(uint8_t* request, uint8_t req_len,
                               uint8_t* response, uint8_t* resp_len) {
    if (req_len < 2) {
        response[0] = 0x7F;
        response[1] = 0x3E;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t sub_function = request[1];
    lastTesterPresentTime = millis();
    
    // If sub-function = 0x01, send response
    if (sub_function == 0x01) {
        response[0] = 0x7E;  // 0x3E + 0x40
        response[1] = 0x01;
        *resp_len = 2;
        return true;
    }
    
    // Sub-function 0x00 = suppress response
    *resp_len = 0;
    return true;
}

// Check for session timeout (in loop())
void checkTesterPresent() {
    if (currentDiagnosticSession != UDS_SESSION_DEFAULT) {
        if ((millis() - lastTesterPresentTime) > 5000) {
            // Session timeout, return to default
            currentDiagnosticSession = UDS_SESSION_DEFAULT;
        }
    }
}
```

### 4. ECUReset (0x11)

**Purpose**: Reset ECU (software reset)

**Request Format**:
```
[0x11] [Reset Type]
```

**Reset Types**:
- 0x01: Hard reset
- 0x03: Soft reset

**Response Format**:
```
[0x51] [Reset Type]
```

**Implementation**:
```cpp
bool uds_handle_ecu_reset(uint8_t* request, uint8_t req_len,
                         uint8_t* response, uint8_t* resp_len) {
    if (req_len < 2) {
        response[0] = 0x7F;
        response[1] = 0x11;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t reset_type = request[1];
    
    // Validate reset type
    if (reset_type != 0x01 && reset_type != 0x03) {
        response[0] = 0x7F;
        response[1] = 0x11;
        response[2] = NRC_SUBFUNCTION_NOT_SUPPORTED;
        *resp_len = 3;
        return false;
    }
    
    // Send positive response
    response[0] = 0x51;  // 0x11 + 0x40
    response[1] = reset_type;
    *resp_len = 2;
    
    // Note: Response must be sent before reset
    // In practice, queue response, then delay slightly, then reset
    
    delay(50);  // Allow response to be sent
    
    // Perform reset
    if (reset_type == 0x01 || reset_type == 0x03) {
        ESP.restart();
    }
    
    return true;
}
```

---

## Multi-Frame Handling (ISO 15765-2)

This is the most complex part. Implementation example:

```cpp
// iso15765.cpp - Multi-frame receive state machine

typedef enum {
    STATE_IDLE,
    STATE_WAITING_FF,
    STATE_RECEIVING_CF,
    STATE_WAITING_FC
} MultiFrameState;

static MultiFrameState rx_state = STATE_IDLE;
static uint8_t rx_buffer[4095];
static uint16_t rx_total_length = 0;
static uint16_t rx_received = 0;
static uint8_t rx_sequence = 0;

bool iso15765_receive_multi(CanFrame* frame, uint8_t* data, uint16_t* length) {
    uint8_t pci = frame->data[0];
    uint8_t pci_type = (pci & 0xF0) >> 4;
    
    switch (pci_type) {
        case 0:  // Single frame
            {
                uint8_t data_len = pci & 0x0F;
                memcpy(data, &frame->data[1], data_len);
                *length = data_len;
                return true;
            }
            
        case 1:  // First frame
            {
                // Extract total length
                rx_total_length = ((pci & 0x0F) << 8) | frame->data[1];
                rx_received = 6;  // First frame has 6 data bytes
                
                // Copy first frame data
                memcpy(rx_buffer, &frame->data[2], 6);
                
                // Send flow control: Continue to send
                CanFrame fc_frame = {0};
                fc_frame.identifier = frame->identifier + 8;  // Response ID
                fc_frame.extd = 0;
                fc_frame.data_length_code = 3;
                fc_frame.data[0] = 0x30;  // Flow control: Continue to send
                fc_frame.data[1] = 0x00;  // Block size: 0 (send all)
                fc_frame.data[2] = 0x00;   // STmin: 0ms
                ESP32Can.writeFrame(fc_frame, 0);
                
                rx_state = STATE_RECEIVING_CF;
                rx_sequence = 0;
                *length = 0;  // Not complete yet
                return false;
            }
            
        case 2:  // Consecutive frame
            {
                if (rx_state != STATE_RECEIVING_CF) {
                    return false;  // Out of sequence
                }
                
                uint8_t seq_num = pci & 0x0F;
                
                // Check sequence number
                if (seq_num != ((rx_sequence + 1) & 0x0F)) {
                    // Sequence error
                    rx_state = STATE_IDLE;
                    return false;
                }
                
                rx_sequence = seq_num;
                
                // Copy data (7 bytes per frame)
                uint16_t to_copy = min(7, rx_total_length - rx_received);
                memcpy(&rx_buffer[rx_received], &frame->data[1], to_copy);
                rx_received += to_copy;
                
                // Check if complete
                if (rx_received >= rx_total_length) {
                    memcpy(data, rx_buffer, rx_total_length);
                    *length = rx_total_length;
                    rx_state = STATE_IDLE;
                    return true;
                }
                
                return false;  // Still receiving
            }
            
        default:
            return false;
    }
}
```

---

## Testing & Validation

### Test Tools
1. **CANoe** (Vector) - Industry standard
2. **PCAN Explorer** - Affordable option
3. **OBDLink** - Simple testing
4. **Custom Python Script** - For development

### Test Cases
1. ✅ Single-frame ReadDataByIdentifier
2. ✅ Multi-frame ReadDataByIdentifier (>8 bytes)
3. ✅ DiagnosticSessionControl
4. ✅ TesterPresent
5. ✅ Negative responses (invalid DID, wrong length, etc.)
6. ✅ Session timeout
7. ✅ ECUReset

---

## Implementation Timeline

### Week 1-2: ISO 15765 Transport Layer
- Single-frame send/receive
- Multi-frame send/receive
- Flow control
- Timing management

### Week 3: UDS Core Services
- DiagnosticSessionControl (0x10)
- ReadDataByIdentifier (0x22) ⭐
- TesterPresent (0x3E)
- ECUReset (0x11)

### Week 4: Testing & Integration
- Integration testing
- Error handling
- Performance optimization

### Week 5-6: Advanced Services (Optional)
- SecurityAccess (0x27)
- WriteDataByIdentifier (0x2E)
- ReadDTCInformation (0x19)

---

## Integration Impact

### Current System Changes Needed

1. **CAN ID Configuration**
   - Add ISO 15765 CAN IDs
   - May need to change current EPIC protocol IDs (if conflicts)

2. **Message Handling**
   - Add ISO message detection in `handleCanRx()`
   - Route ISO messages to UDS handler
   - Keep existing EPIC protocol working in parallel

3. **Variable Access**
   - Map EPIC variables to UDS DIDs
   - Provide UDS access alongside EPIC protocol

4. **Session Management**
   - Track diagnostic sessions
   - Handle TesterPresent timeout

### Backward Compatibility

**Recommended**: Support both protocols simultaneously
- EPIC protocol: Continue working (custom tool)
- ISO/UDS protocol: New standard interface

**Implementation**: Check CAN ID to route to correct handler

---

## Estimated Effort

**Minimum Implementation** (Core services only):
- **Time**: 3-4 weeks
- **Complexity**: High
- **Required Services**: 0x10, 0x11, 0x22, 0x3E

**Full Implementation** (All services):
- **Time**: 6-8 weeks
- **Complexity**: Very High
- **All Services**: 0x10, 0x11, 0x22, 0x2E, 0x27, 0x3E, 0x19, 0x14

---

## Conclusion

**ISO 14229/15765 compliance requires**:
1. ✅ ISO 15765 transport layer (multi-frame handling)
2. ✅ UDS service layer (service handlers)
3. ✅ Session management
4. ✅ Error handling (negative responses)
5. ✅ Timing compliance

**Key Service**: `ReadDataByIdentifier (0x22)` maps your EPIC variables to standard UDS interface

**Recommendation**: Start with transport layer, then core services. Add advanced services as needed.

---

**See also**: `IMPLEMENTATION_ROADMAP.md` for detailed code structure and examples.

