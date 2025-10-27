# Active Context: USB_HID_CAN_BRIDGE

## Current Work Focus
Project is in stable operational state with comprehensive user-facing documentation completed. The firmware is functional and handles USB keyboard to CAN bus bridging. Recent focus has been on creating professional documentation to help users build and deploy the system.

## Recent Changes
- Created comprehensive README.md for GitHub with full installation guide
- Added "Parts Needed" section with direct Amazon purchase links for all components
- Added "Hardware Setup Guide" with 9 step-by-step assembly photos from `pics/` directory
- Documented buck converter preparation (trace cutting, voltage adjustment)
- Documented CAN transceiver installation with visual guides
- Documented USB OTG bridge soldering
- README.md includes markdown tables, embedded images, and professional formatting
- No firmware code changes - focus on documentation and user experience

## Next Steps
- Monitor system operation for stability in field deployment
- Gather user feedback on documentation clarity and completeness
- Consider adding configuration options (CAN address, baud rate) if needed
- Potential enhancement: Add support for keyboard LED feedback
- Consider adding EEPROM storage for persistent configuration
- Evaluate need for additional error diagnostics
- Possible: Create video assembly guide to complement photo documentation

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
- Buck converter trace cutting required for voltage adjustment - critical for proper 5V output
- Visual documentation (photos) significantly improves user assembly experience
- Direct purchase links reduce friction in parts procurement for builders
- GitHub markdown with inline images provides professional presentation

