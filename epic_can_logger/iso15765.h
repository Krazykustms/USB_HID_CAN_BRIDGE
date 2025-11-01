// ISO 15765-2/4 Transport Layer for Diagnostics over CAN (DoCAN)
// Implements multi-frame message handling and flow control

#ifndef ISO15765_H
#define ISO15765_H

#include <stdint.h>
#include <stdbool.h>
#include <ESP32-TWAI-CAN.hpp>

// ISO 15765-4 Physical Addressing
#define ISO_15765_PHYSICAL_REQUEST_BASE  0x7DF   // Physical functional request (functional addressing)
#define ISO_15765_PHYSICAL_RESPONSE_BASE 0x7E8   // Physical response base (0x7E8 + ECU_ID)

// ISO 15765-2 PCI Types
#define PCI_SINGLE_FRAME       0x00
#define PCI_FIRST_FRAME        0x10
#define PCI_CONSECUTIVE_FRAME  0x20
#define PCI_FLOW_CONTROL       0x30

// Flow Control types
#define FC_CONTINUE_TO_SEND    0x30
#define FC_WAIT                0x31
#define FC_OVERFLOW            0x32

// ISO 15765-2 Timing (milliseconds)
#define ISO_N_As     1000  // Request timeout (Ar = 1000ms)
#define ISO_N_Ar     5000  // Response timeout
#define ISO_N_Bs     1000  // Block size timeout
#define ISO_N_Cr     1000  // Consecutive frame timeout
#define ISO_STmin    0     // Minimum separation time (0ms = send all)

// Maximum message sizes
#define ISO_15765_MAX_MESSAGE_SIZE  4095  // ISO 15765-2 maximum
#define ISO_15765_BUFFER_SIZE       4096  // Internal buffer

// Function declarations

// Initialize ISO 15765 transport layer
bool iso15765_init(uint8_t ecu_id);

// Send single-frame message (≤7 bytes data)
bool iso15765_send_single(uint8_t* data, uint8_t length, uint32_t can_id);

// Send multi-frame message (>7 bytes data)
bool iso15765_send_multi(uint8_t* data, uint16_t length, uint32_t can_id);

// Receive single-frame message (returns true when complete)
bool iso15765_receive_single(CanFrame* frame, uint8_t* data, uint8_t* length);

// Receive multi-frame message (returns true when complete)
bool iso15765_receive_multi(CanFrame* frame, uint8_t* data, uint16_t* length);

// Process received CAN frame (call from handleCanRx)
void iso15765_process_rx(CanFrame* frame);

// Check for completed receive messages
bool iso15765_receive_complete(uint8_t* data, uint16_t* length);

// Get current ECU ID
uint8_t iso15765_get_ecu_id(void);

// Periodic task (call from loop)
void iso15765_task(void);

#endif // ISO15765_H

