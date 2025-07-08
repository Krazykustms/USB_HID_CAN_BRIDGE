use arduino ide
add esp32-s3 board package
pick esp32-s3-usb-otg board type
"include library" the libraries in this repo, ESP32-TWAI-CAN-master.zip and EspUsbHost2-master.zip
connect canbus phy to board,  TX pin5  RX pin4
connect 12v to 5v adapter to appropriate pins on the esp32 board
connect canbus to where it should go CAN-H, CAN-L

tl;dr: this takes usb keyboard, and sends keycode with some bytes using 0x711 address




