// ISO 15765-2/4 Transport Layer Implementation
// Multi-frame message handling with flow control

#include "iso15765.h"
#include <string.h>

// Debug support
#ifndef DEBUG_PRINT
  #define DEBUG_PRINT(...)
#endif

// Internal state
static uint8_t iso_ecu_id = 1;
static iso15765_error_callback_t error_callback = NULL;

// Error tracking
static uint8_t rx_timeout_count = 0;
static uint8_t seq_error_count = 0;
static uint8_t tx_retry_count = 0;
static uint32_t tx_fc_wait_start = 0;
static uint8_t tx_overflow_count = 0;
static uint32_t tx_last_overflow_time = 0;

// Receive state machine
typedef enum {
    RX_STATE_IDLE,
    RX_STATE_WAITING_FF,
    RX_STATE_RECEIVING_CF,
    RX_STATE_WAITING_FC
} RxState;

static RxState rx_state = RX_STATE_IDLE;
static uint8_t rx_buffer[ISO_15765_BUFFER_SIZE];
static uint16_t rx_total_length = 0;
static uint16_t rx_received = 0;
static uint8_t rx_sequence = 0;
static uint32_t rx_can_id = 0;
static uint32_t rx_last_cf_time = 0;
static uint8_t rx_ready_buffer[ISO_15765_BUFFER_SIZE];
static uint16_t rx_ready_length = 0;
static bool rx_message_ready = false;

// Transmit state machine
typedef enum {
    TX_STATE_IDLE,
    TX_STATE_SENDING_FF,
    TX_STATE_SENDING_CF,
    TX_STATE_WAITING_FC
} TxState;

static TxState tx_state = TX_STATE_IDLE;
static uint8_t tx_buffer[ISO_15765_BUFFER_SIZE];
static uint16_t tx_total_length = 0;
static uint16_t tx_sent = 0;
static uint8_t tx_sequence = 0;
static uint32_t tx_can_id = 0;
static uint32_t tx_last_send_time = 0;
static uint8_t tx_block_size = 0;  // 0 = send all
static uint8_t tx_stmin = 0;  // Minimum separation time (ms)

// Initialize ISO 15765 transport layer
bool iso15765_init(uint8_t ecu_id) {
    iso_ecu_id = ecu_id;
    rx_state = RX_STATE_IDLE;
    tx_state = TX_STATE_IDLE;
    rx_message_ready = false;
    rx_ready_length = 0;
    rx_timeout_count = 0;
    seq_error_count = 0;
    tx_retry_count = 0;
    tx_overflow_count = 0;
    tx_fc_wait_start = 0;
    return true;
}

// Set error callback
void iso15765_set_error_callback(iso15765_error_callback_t callback) {
    error_callback = callback;
}

// Report error to callback
static void report_error(uint8_t error_code, const char* description) {
    if (error_callback) {
        error_callback(error_code, description);
    }
}

uint8_t iso15765_get_ecu_id(void) {
    return iso_ecu_id;
}

// Send single-frame message (≤7 bytes data)
bool iso15765_send_single(uint8_t* data, uint8_t length, uint32_t can_id) {
    if (length > 7 || length == 0) {
        return false;  // Single frame: 0-7 bytes data
    }
    
    CanFrame frame = {0};
    frame.identifier = can_id;
    frame.extd = 0;
    frame.data_length_code = length + 1;  // PCI byte + data
    
    // PCI: single frame (0x00-0x07 = length)
    frame.data[0] = PCI_SINGLE_FRAME | length;
    
    // Copy data
    memcpy(&frame.data[1], data, length);
    
    return ESP32Can.writeFrame(frame, 0);
}

// Send multi-frame message (>7 bytes data)
bool iso15765_send_multi(uint8_t* data, uint16_t length, uint32_t can_id) {
    if (length <= 7 || length > ISO_15765_MAX_MESSAGE_SIZE) {
        return false;
    }
    
    // Store message for transmission
    memcpy(tx_buffer, data, length);
    tx_total_length = length;
    tx_sent = 0;
    tx_sequence = 0;
    tx_can_id = can_id;
    tx_state = TX_STATE_SENDING_FF;
    tx_block_size = 0;  // Send all (no flow control)
    tx_stmin = 0;
    
    // Send first frame immediately
    CanFrame ff_frame = {0};
    ff_frame.identifier = can_id;
    ff_frame.extd = 0;
    ff_frame.data_length_code = 8;
    
    // First frame PCI: 0x1X YY YY YY
    // X = high nibble of length, YY YY = length (12 bits)
    uint8_t length_high = (length >> 8) & 0x0F;
    uint16_t length_low = length & 0xFFF;
    ff_frame.data[0] = PCI_FIRST_FRAME | length_high;
    ff_frame.data[1] = (length_low >> 8) & 0xFF;
    ff_frame.data[2] = length_low & 0xFF;
    
    // Copy first 5 bytes of data
    memcpy(&ff_frame.data[3], tx_buffer, 5);
    tx_sent = 5;
    
    bool sent = ESP32Can.writeFrame(ff_frame, 0);
    if (!sent) {
        tx_state = TX_STATE_IDLE;
        return false;
    }
    
    tx_last_send_time = millis();
    tx_fc_wait_start = millis();  // ERROR #7 FIX: Track FC wait start time
    tx_state = TX_STATE_SENDING_CF;
    tx_overflow_count = 0;  // Reset overflow counter for new transmission
    
    return true;
}

// Process multi-frame transmission (call periodically)
static void iso15765_tx_task(void) {
    uint32_t now = millis();
    
    // ERROR #7 FIX: Check for flow control timeout
    if (tx_state == TX_STATE_WAITING_FC) {
        if (tx_fc_wait_start > 0 && (now - tx_fc_wait_start) > ISO_N_Bs) {
            // Flow control timeout
            report_error(ISO_ERROR_FC_TIMEOUT, "Flow control response timeout");
            DEBUG_PRINT("ISO15765: Flow control timeout after %d ms\n", ISO_N_Bs);
            tx_state = TX_STATE_IDLE;
            tx_fc_wait_start = 0;
            return;
        }
        
        // Check if backoff period has elapsed (for overflow retry)
        if (tx_overflow_count > 0 && tx_last_overflow_time > 0) {
            uint32_t backoff_delay = ISO_FC_BACKOFF_MS;
            for (uint8_t i = 0; i < tx_overflow_count && i < 4; i++) {
                backoff_delay *= 2;
            }
            if (backoff_delay > ISO_FC_MAX_BACKOFF_MS) {
                backoff_delay = ISO_FC_MAX_BACKOFF_MS;
            }
            
            if ((now - tx_last_overflow_time) >= backoff_delay) {
                // Backoff elapsed, retry sending first frame
                DEBUG_PRINT("ISO15765: Backoff elapsed, retrying transmission\n");
                
                // Retry sending first frame
                CanFrame ff_frame = {0};
                ff_frame.identifier = tx_can_id;
                ff_frame.extd = 0;
                ff_frame.data_length_code = 8;
                
                uint8_t length_high = (tx_total_length >> 8) & 0x0F;
                uint16_t length_low = tx_total_length & 0xFFF;
                ff_frame.data[0] = PCI_FIRST_FRAME | length_high;
                ff_frame.data[1] = (length_low >> 8) & 0xFF;
                ff_frame.data[2] = length_low & 0xFF;
                
                memcpy(&ff_frame.data[3], tx_buffer, 5);
                tx_sent = 5;
                
                if (ESP32Can.writeFrame(ff_frame, 0)) {
                    tx_last_send_time = now;
                    tx_fc_wait_start = now;
                    tx_state = TX_STATE_SENDING_CF;
                    tx_last_overflow_time = 0;
                } else {
                    // Still can't send, keep waiting
                }
            }
        }
        return;
    }
    
    if (tx_state == TX_STATE_SENDING_CF) {
        // Check if we can send next consecutive frame
        if ((now - tx_last_send_time) >= tx_stmin) {
            // Send next consecutive frame
            if (tx_sent < tx_total_length) {
                CanFrame cf_frame = {0};
                cf_frame.identifier = tx_can_id;
                cf_frame.extd = 0;
                cf_frame.data_length_code = 8;
                
                // Consecutive frame PCI: 0x2X (X = sequence 0-15)
                cf_frame.data[0] = PCI_CONSECUTIVE_FRAME | (tx_sequence & 0x0F);
                
                // Copy next 7 bytes of data
                uint16_t to_send = min(7, tx_total_length - tx_sent);
                memcpy(&cf_frame.data[1], &tx_buffer[tx_sent], to_send);
                
                // Pad with 0xFF if needed
                for (uint16_t i = to_send + 1; i < 8; i++) {
                    cf_frame.data[i] = 0xFF;
                }
                
                // ERROR #3 FIX: Retry queue for write failures
                bool sent = ESP32Can.writeFrame(cf_frame, 0);
                if (sent) {
                    tx_sent += to_send;
                    tx_sequence = (tx_sequence + 1) & 0x0F;
                    tx_last_send_time = now;
                    tx_retry_count = 0;  // Reset retry count on success
                    
                    // Check if complete
                    if (tx_sent >= tx_total_length) {
                        tx_state = TX_STATE_IDLE;
                        tx_retry_count = 0;
                        tx_fc_wait_start = 0;
                    }
                } else {
                    // ERROR #3: Send failed - retry with exponential backoff
                    tx_retry_count++;
                    if (tx_retry_count < ISO_MAX_RETRIES) {
                        // Will retry next cycle
                        DEBUG_PRINT("ISO15765: CF send failed, retry %d/%d\n", tx_retry_count, ISO_MAX_RETRIES);
                    } else {
                        // Max retries exceeded
                        DEBUG_PRINT("ISO15765: CF send failed after %d retries, aborting\n", tx_retry_count);
                        tx_state = TX_STATE_IDLE;
                        tx_retry_count = 0;
                    }
                }
            } else {
                // Transmission complete
                tx_state = TX_STATE_IDLE;
            }
        }
    }
}

// Process received CAN frame
void iso15765_process_rx(CanFrame* frame) {
    if (!frame || frame->data_length_code == 0) {
        return;
    }
    
    uint8_t pci = frame->data[0];
    uint8_t pci_type = (pci & 0xF0) >> 4;
    
    switch (pci_type) {
        case 0:  // Single frame
            {
                uint8_t data_len = pci & 0x0F;
                if (data_len > 0 && data_len <= 7 && frame->data_length_code >= (data_len + 1)) {
                    // Complete single-frame message
                    memcpy(rx_ready_buffer, &frame->data[1], data_len);
                    rx_ready_length = data_len;
                    rx_message_ready = true;
                    rx_can_id = frame->identifier;
                }
            }
            break;
            
        case 1:  // First frame
            {
                if (rx_state != RX_STATE_IDLE) {
                    // Discard previous incomplete message
                    rx_state = RX_STATE_IDLE;
                }
                
                // Extract total length
                uint8_t length_high = pci & 0x0F;
                if (frame->data_length_code >= 3) {
                    uint16_t length_low = ((uint16_t)frame->data[1] << 8) | frame->data[2];
                    rx_total_length = (length_high << 12) | length_low;
                    
                    // ERROR #5 FIX: Validate against buffer size to prevent overflow
                    if (rx_total_length > 0 && 
                        rx_total_length <= ISO_15765_MAX_MESSAGE_SIZE && 
                        rx_total_length <= ISO_15765_BUFFER_SIZE) {
                        rx_received = 5;  // First frame has 5 data bytes
                        rx_sequence = 0;
                        rx_can_id = frame->identifier;
                        
                        // Copy first frame data
                        memcpy(rx_buffer, &frame->data[3], 5);
                        
                        // Send flow control: Continue to send (if needed)
                        // For now, we'll accept without flow control (block_size = 0)
                        CanFrame fc_frame = {0};
                        fc_frame.identifier = rx_can_id + 8;  // Response ID (functional = request+8)
                        fc_frame.extd = 0;
                        fc_frame.data_length_code = 3;
                        fc_frame.data[0] = FC_CONTINUE_TO_SEND;
                        fc_frame.data[1] = 0x00;  // Block size: 0 = send all
                        fc_frame.data[2] = ISO_STmin;  // STmin: 0ms
                        ESP32Can.writeFrame(fc_frame, 0);
                        
                        rx_state = RX_STATE_RECEIVING_CF;
                        rx_last_cf_time = millis();
                    } else {
                        // Invalid length - ERROR #5: Report buffer overflow
                        if (rx_total_length > ISO_15765_BUFFER_SIZE) {
                            report_error(ISO_ERROR_BUFFER_OVERFLOW, "First frame length exceeds buffer size");
                            DEBUG_PRINT("ISO15765: Buffer overflow - length %d > buffer %d\n", 
                                      rx_total_length, ISO_15765_BUFFER_SIZE);
                        }
                        rx_state = RX_STATE_IDLE;
                    }
                }
            }
            break;
            
        case 2:  // Consecutive frame
            {
                if (rx_state != RX_STATE_RECEIVING_CF) {
                    // Out of sequence
                    rx_state = RX_STATE_IDLE;
                    break;
                }
                
                // ERROR #1 FIX: Check timeout with error reporting
                if ((millis() - rx_last_cf_time) > ISO_N_Cr) {
                    // Timeout waiting for consecutive frame
                    rx_timeout_count++;
                    report_error(ISO_ERROR_TIMEOUT, "Consecutive frame timeout");
                    DEBUG_PRINT("ISO15765: Consecutive frame timeout (count: %d)\n", rx_timeout_count);
                    rx_state = RX_STATE_IDLE;
                    break;
                }
                
                uint8_t seq_num = pci & 0x0F;
                uint8_t expected_seq = (rx_sequence + 1) & 0x0F;
                
                // ERROR #4 FIX: Sequence error with recovery attempt
                if (seq_num != expected_seq) {
                    // Sequence error
                    seq_error_count++;
                    report_error(ISO_ERROR_SEQUENCE, "Consecutive frame sequence mismatch");
                    DEBUG_PRINT("ISO15765: Sequence error - expected %d, got %d (count: %d)\n", 
                              expected_seq, seq_num, seq_error_count);
                    
                    // If too many sequence errors, abort
                    if (seq_error_count >= ISO_SEQ_ERROR_MAX) {
                        DEBUG_PRINT("ISO15765: Too many sequence errors, aborting message\n");
                        rx_state = RX_STATE_IDLE;
                        seq_error_count = 0;
                    }
                    // Otherwise, continue to wait for correct sequence (frame might be reordered)
                    break;
                }
                
                // Reset sequence error count on successful frame
                seq_error_count = 0;
                
                rx_sequence = seq_num;
                rx_last_cf_time = millis();
                
                // Copy data (7 bytes per frame)
                uint16_t to_copy = min(7, rx_total_length - rx_received);
                if (frame->data_length_code >= 8) {
                    memcpy(&rx_buffer[rx_received], &frame->data[1], to_copy);
                    rx_received += to_copy;
                    
                    // Check if complete
                    if (rx_received >= rx_total_length) {
                        // Message complete
                        memcpy(rx_ready_buffer, rx_buffer, rx_total_length);
                        rx_ready_length = rx_total_length;
                        rx_message_ready = true;
                        rx_state = RX_STATE_IDLE;
                    }
                }
            }
            break;
            
        case 3:  // Flow control
            {
                if (tx_state == TX_STATE_SENDING_CF || tx_state == TX_STATE_WAITING_FC) {
                    // Extract flow control parameters
                    if (frame->data_length_code >= 3) {
                        uint8_t fc_type = pci & 0x0F;
                        tx_block_size = frame->data[1];
                        tx_stmin = frame->data[2];
                        
                        if (fc_type == 0) {  // Continue to send
                            tx_state = TX_STATE_SENDING_CF;
                            tx_fc_wait_start = 0;  // Clear FC wait timeout
                            tx_overflow_count = 0;  // Reset overflow count on successful FC
                        } else if (fc_type == 1) {  // Wait
                            tx_state = TX_STATE_WAITING_FC;
                            tx_fc_wait_start = millis();  // Start FC timeout timer
                        } else if (fc_type == 2) {  // Overflow
                            // ERROR #2 FIX: Flow control overflow with retry and backoff
                            tx_overflow_count++;
                            uint32_t now = millis();
                            
                            // Calculate exponential backoff
                            uint32_t backoff_delay = ISO_FC_BACKOFF_MS;
                            for (uint8_t i = 0; i < tx_overflow_count && i < 4; i++) {
                                backoff_delay *= 2;
                            }
                            if (backoff_delay > ISO_FC_MAX_BACKOFF_MS) {
                                backoff_delay = ISO_FC_MAX_BACKOFF_MS;
                            }
                            
                            // Only abort if we've exceeded retries
                            if (tx_overflow_count >= ISO_MAX_RETRIES) {
                                report_error(ISO_ERROR_FC_OVERFLOW, "Flow control overflow - max retries exceeded");
                                DEBUG_PRINT("ISO15765: FC overflow - aborting after %d retries\n", tx_overflow_count);
                                tx_state = TX_STATE_IDLE;
                                tx_overflow_count = 0;
                            } else {
                                // Schedule retry after backoff
                                tx_last_overflow_time = now;
                                tx_state = TX_STATE_WAITING_FC;
                                DEBUG_PRINT("ISO15765: FC overflow - retry %d after %d ms backoff\n", 
                                          tx_overflow_count, backoff_delay);
                                // Note: Retry will be handled in tx_task based on backoff
                            }
                        }
                    }
                }
            }
            break;
            
        default:
            // Unknown PCI type
            break;
    }
}

// Check for completed receive messages
bool iso15765_receive_complete(uint8_t* data, uint16_t* length) {
    if (rx_message_ready && data && length) {
        memcpy(data, rx_ready_buffer, rx_ready_length);
        *length = rx_ready_length;
        rx_message_ready = false;
        rx_ready_length = 0;
        return true;
    }
    return false;
}

// Receive single-frame (wrapper for process_rx)
bool iso15765_receive_single(CanFrame* frame, uint8_t* data, uint8_t* length) {
    iso15765_process_rx(frame);
    if (rx_message_ready && rx_ready_length <= 7) {
        if (data && length) {
            memcpy(data, rx_ready_buffer, rx_ready_length);
            *length = rx_ready_length;
            rx_message_ready = false;
            rx_ready_length = 0;
            return true;
        }
    }
    return false;
}

// Receive multi-frame (wrapper for process_rx)
bool iso15765_receive_multi(CanFrame* frame, uint8_t* data, uint16_t* length) {
    iso15765_process_rx(frame);
    return iso15765_receive_complete(data, length);
}

// Periodic task (call from loop)
void iso15765_task(void) {
    iso15765_tx_task();
    
    // ERROR #1 FIX: Check for receive timeout with error reporting
    if (rx_state == RX_STATE_RECEIVING_CF) {
        if ((millis() - rx_last_cf_time) > ISO_N_Cr) {
            // Timeout waiting for consecutive frame
            rx_timeout_count++;
            report_error(ISO_ERROR_TIMEOUT, "Receive consecutive frame timeout");
            DEBUG_PRINT("ISO15765: Receive timeout (count: %d)\n", rx_timeout_count);
            rx_state = RX_STATE_IDLE;
        }
    }
}

