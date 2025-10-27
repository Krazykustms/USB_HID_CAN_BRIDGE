# Active Context: USB_HID_CAN_BRIDGE

## Current Work Focus
Project is in stable operational state. Memory bank has been established to document the existing codebase and architecture. The firmware is functional and handles USB keyboard to CAN bus bridging.

## Recent Changes
- Created complete memory bank documentation in `.project/` directory
- Documented all 7 core memory bank files
- Captured existing hardware configuration, software architecture, and operational details
- No code changes - documentation of existing working system

## Next Steps
- Monitor system operation for stability
- Consider adding configuration options (CAN address, baud rate)
- Potential enhancement: Add support for keyboard LED feedback
- Consider adding EEPROM storage for persistent configuration
- Evaluate need for additional error diagnostics

## Active Decisions and Considerations
- **HID Locale**: Currently set to `HID_LOCAL_Japan_Katakana` - consider if this is optimal for target keyboards
- **Error Threshold**: 30 errors trigger auto-restart - this value may need tuning based on real-world usage
- **LED Feedback**: Simple on/off states work but could be enhanced with blink patterns for different conditions
- **Queue Sizes**: RX/TX queues set to 500 - adequate for current use but may need adjustment for high-throughput scenarios

## Important Patterns and Preferences
- **Error Recovery Over Graceful Degradation**: System prefers full restart over attempting to continue in degraded state
- **Visual Feedback First**: LED status changes happen before operations (user sees intent, then result)
- **Retry Logic**: 5 retries with 13ms delay is standard pattern for CAN operations
- **Serial Debugging**: Comprehensive logging available but diagnostic details commented out to reduce noise
- **Modifier Key Encoding**: Extends keycodes to 16-bit by adding `modifier * 0xFF` to base keycode

## Learnings and Project Insights
- ESP32-S3-USB-OTG board has onboard WS2812 LED on GPIO 48 - perfect for status indication
- USB HID reports follow standard 8-byte format: `[modifier][reserved][key1][key2][key3][key4][key5][key6]`
- CAN bus stability is critical - even brief issues can accumulate and require restart
- The 0x711 address suggests integration with a specific ECU/control system expecting button box input
- Japanese Katakana HID locale setting hints at potential Japanese market target or specific keyboard compatibility requirement

