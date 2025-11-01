// ISO 14229 UDS Service Layer Implementation

#include "uds.h"
#include "epic_variables.h"
#include "iso15765.h"
#include <string.h>

// Debug support
#ifndef DEBUG_PRINT
  #define DEBUG_PRINT(...)
#endif

// Session management
static uint8_t current_session = UDS_SESSION_DEFAULT;
static uint32_t last_tester_present_time = 0;
static bool uds_initialized = false;

// DID to EPIC variable mapping
typedef struct {
    uint16_t uds_did;
    int32_t epic_var_id;
    const char* name;
} DidMapping;

// Map EPIC variables to UDS DIDs
static const DidMapping did_map[] = {
    {UDS_DID_TPS_VALUE, VAR_ID_TPS_VALUE, "TPSValue"},
    {UDS_DID_RPM_VALUE, VAR_ID_RPM_VALUE, "RPMValue"},
    {UDS_DID_AFR_VALUE, VAR_ID_AFR_VALUE, "AFRValue"},
    // Add more mappings as needed
    {0, 0, NULL}  // Terminator
};

// Initialize UDS service layer
bool uds_init(void) {
    current_session = UDS_SESSION_DEFAULT;
    last_tester_present_time = 0;
    uds_initialized = true;
    return true;
}

uint8_t uds_get_current_session(void) {
    return current_session;
}

// Find EPIC variable ID from UDS DID
static int32_t find_epic_var_id(uint16_t did) {
    for (uint8_t i = 0; did_map[i].uds_did != 0; i++) {
        if (did_map[i].uds_did == did) {
            return did_map[i].epic_var_id;
        }
    }
    return 0;  // Not found
}

// Get variable value from EPIC response array
static bool get_epic_variable_value(int32_t var_id, float* value) {
    extern VarResponse varResponses[];
    extern const uint8_t EPIC_VAR_COUNT;
    
    for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
        if (varResponses[i].var_id == var_id && varResponses[i].valid) {
            *value = varResponses[i].value;
            return true;
        }
    }
    return false;
}

// Handle DiagnosticSessionControl (0x10)
static bool uds_handle_session_control(uint8_t* request, uint8_t req_len,
                                       uint8_t* response, uint16_t* resp_len) {
    if (req_len < 2) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t sub_function = request[1];
    
    // Validate sub-function
    if (sub_function == 0x00 || sub_function > UDS_SESSION_SAFETY_SYSTEM) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL;
        response[2] = NRC_SUBFUNCTION_NOT_SUPPORTED;
        *resp_len = 3;
        return false;
    }
    
    // Switch session
    current_session = sub_function;
    last_tester_present_time = millis();  // Reset tester present timer
    
    // Positive response
    response[0] = 0x50;  // 0x10 + 0x40
    response[1] = sub_function;
    // Add session parameters if needed (for now, just sub-function echo)
    *resp_len = 2;
    
    DEBUG_PRINT("UDS: Session changed to %d\n", sub_function);
    return true;
}

// Handle ECUReset (0x11)
static bool uds_handle_ecu_reset(uint8_t* request, uint8_t req_len,
                                 uint8_t* response, uint16_t* resp_len) {
    if (req_len < 2) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_ECU_RESET;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t reset_type = request[1];
    
    // Validate reset type (only support hard reset 0x01 and soft reset 0x03)
    if (reset_type != 0x01 && reset_type != 0x03) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_ECU_RESET;
        response[2] = NRC_SUBFUNCTION_NOT_SUPPORTED;
        *resp_len = 3;
        return false;
    }
    
    // Send positive response BEFORE reset
    response[0] = 0x51;  // 0x11 + 0x40
    response[1] = reset_type;
    *resp_len = 2;
    
    // Note: Actual reset will happen after response is sent
    // This is handled in main code after sending response
    
    DEBUG_PRINT("UDS: ECU Reset requested (type=%d)\n", reset_type);
    return true;
}

// Handle ReadDataByIdentifier (0x22) - Maps to EPIC variables
static bool uds_handle_read_data_by_id(uint8_t* request, uint8_t req_len,
                                       uint8_t* response, uint16_t* resp_len) {
    if (req_len < 3) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_READ_DATA_BY_IDENTIFIER;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    // Extract DID (big-endian)
    uint16_t did = ((uint16_t)request[1] << 8) | request[2];
    
    // Find corresponding EPIC variable
    int32_t epic_var_id = find_epic_var_id(did);
    
    if (epic_var_id == 0) {
        // DID not found
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_READ_DATA_BY_IDENTIFIER;
        response[2] = NRC_REQUEST_OUT_OF_RANGE;
        *resp_len = 3;
        return false;
    }
    
    // Get current value from EPIC variable responses
    float value = 0.0;
    if (!get_epic_variable_value(epic_var_id, &value)) {
        // Value not available yet
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_READ_DATA_BY_IDENTIFIER;
        response[2] = NRC_CONDITIONS_NOT_CORRECT;
        *resp_len = 3;
        return false;
    }
    
    // Positive response with value
    response[0] = 0x62;  // 0x22 + 0x40
    response[1] = (did >> 8) & 0xFF;
    response[2] = did & 0xFF;
    
    // Encode float as 4 bytes (big-endian IEEE 754)
    uint8_t* value_bytes = (uint8_t*)&value;
    response[3] = value_bytes[3];
    response[4] = value_bytes[2];
    response[5] = value_bytes[1];
    response[6] = value_bytes[0];
    *resp_len = 7;
    
    DEBUG_PRINT("UDS: ReadDataByIdentifier DID=0x%04X, value=%.6f\n", did, value);
    return true;
}

// Handle TesterPresent (0x3E)
static bool uds_handle_tester_present(uint8_t* request, uint8_t req_len,
                                      uint8_t* response, uint16_t* resp_len) {
    if (req_len < 2) {
        response[0] = 0x7F;
        response[1] = UDS_SERVICE_TESTER_PRESENT;
        response[2] = NRC_INCORRECT_MESSAGE_LENGTH;
        *resp_len = 3;
        return false;
    }
    
    uint8_t sub_function = request[1];
    last_tester_present_time = millis();
    
    // If sub-function = 0x01, send response; if 0x00, suppress response
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

// Main UDS request processor
bool uds_process_request(uint8_t* request, uint16_t request_len, 
                        uint8_t* response, uint16_t* response_len) {
    if (!request || !response || request_len == 0) {
        return false;
    }
    
    uint8_t service_id = request[0];
    
    switch (service_id) {
        case UDS_SERVICE_DIAGNOSTIC_SESSION_CONTROL:
            return uds_handle_session_control(request, request_len, response, response_len);
            
        case UDS_SERVICE_ECU_RESET:
            return uds_handle_ecu_reset(request, request_len, response, response_len);
            
        case UDS_SERVICE_READ_DATA_BY_IDENTIFIER:
            return uds_handle_read_data_by_id(request, request_len, response, response_len);
            
        case UDS_SERVICE_TESTER_PRESENT:
            return uds_handle_tester_present(request, request_len, response, response_len);
            
        default:
            // Service not supported
            response[0] = 0x7F;
            response[1] = service_id;
            response[2] = NRC_SERVICE_NOT_SUPPORTED;
            *response_len = 3;
            return false;
    }
}

// Check tester present timeout
void uds_check_tester_present(void) {
    if (current_session != UDS_SESSION_DEFAULT) {
        uint32_t now = millis();
        if ((now - last_tester_present_time) > 5000) {  // 5 second timeout
            // Session timeout, return to default
            current_session = UDS_SESSION_DEFAULT;
            DEBUG_PRINT("UDS: Session timeout, returned to default session\n");
        }
    }
}

// Periodic task
void uds_task(void) {
    uds_check_tester_present();
}

