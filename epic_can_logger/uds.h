// ISO 14229 Unified Diagnostic Services (UDS)
// Service layer for automotive diagnostics

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
#define UDS_SERVICE_CLEAR_DIAGNOSTIC_INFORMATION 0x14

// UDS Negative Response Code (NRC)
#define NRC_POSITIVE_RESPONSE          0x00
#define NRC_GENERAL_REJECT             0x10
#define NRC_SERVICE_NOT_SUPPORTED      0x11
#define NRC_SUBFUNCTION_NOT_SUPPORTED 0x12
#define NRC_INCORRECT_MESSAGE_LENGTH   0x13
#define NRC_RESPONSE_TOO_LONG          0x14
#define NRC_BUSY_REPEAT_REQUEST        0x21
#define NRC_CONDITIONS_NOT_CORRECT    0x22
#define NRC_REQUEST_SEQUENCE_ERROR    0x24
#define NRC_NO_RESPONSE_FROM_SUBNET   0x25
#define NRC_FAILURE_PREVENT_EXECUTION  0x26
#define NRC_REQUEST_OUT_OF_RANGE      0x31
#define NRC_SECURITY_ACCESS_DENIED    0x33
#define NRC_INVALID_KEY               0x35
#define NRC_EXCEED_NUMBER_OF_ATTEMPTS 0x36
#define NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED 0x37

// Diagnostic Sessions
#define UDS_SESSION_DEFAULT           0x01
#define UDS_SESSION_PROGRAMMING       0x02
#define UDS_SESSION_EXTENDED          0x03
#define UDS_SESSION_SAFETY_SYSTEM     0x04

// Data Identifiers (DIDs) - Map EPIC variables to UDS DIDs
#define UDS_DID_TPS_VALUE             0xF190
#define UDS_DID_RPM_VALUE             0xF191
#define UDS_DID_AFR_VALUE             0xF192
// Add more DIDs as needed (0xF193, 0xF194, etc.)

// Function declarations

// Initialize UDS service layer
bool uds_init(void);

// Process UDS request and generate response
// Returns true if response generated, false on error
bool uds_process_request(uint8_t* request, uint16_t request_len, 
                        uint8_t* response, uint16_t* response_len);

// Get current diagnostic session
uint8_t uds_get_current_session(void);

// Check tester present timeout (call periodically)
void uds_check_tester_present(void);

// Periodic task (call from loop)
void uds_task(void);

#endif // UDS_H

