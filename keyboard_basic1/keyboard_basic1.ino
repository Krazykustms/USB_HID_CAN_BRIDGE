#include <EspUsbHost.h>
#include <ESP32-TWAI-CAN.hpp>

#define RGB_PIN 48       // WS2812 data pin
#define TS_HW_BUTTONBOX1_CATEGORY 27 // hardware button box 1 -> lookup on ECU side via lookup curve table thing; 
#define CANBUS_BUTTONBOX_ADDRESS 0x711 // CANBUS BUTTONBOX



unsigned long tick = 0;
unsigned long last_tick = 0;
int gone = 1;
int color = 0;

#define CAN_TX 5
#define CAN_RX 4
unsigned long notWorking = 0;
CanFrame rxobdFrame         = {0};
CanFrame obdFrame         = {0};

void sendCMD(uint8_t modifier, uint8_t firstKey, uint8_t secondKey) {
  int retry = 5;
  while (retry > 0) {
    if (ESP32Can.canState()) break;
    delay(13);
    Serial.println("retry");
    retry--;
  }

  if (ESP32Can.canState() != 1) { 
    Serial.println("Can isn't working ?");
    notWorking++;
    return;
  }

    uint32_t crc32_res;
    uint8_t payload[5];

    /* This is from the ECU side of grabbing and using this data.
       This is here for reference only.
      if (frame.data8[0] == 0x5a && frame.data8[1] == 0 && frame.data8[2] == 27 ) {
      	button = frame.data8[3] << 8 | frame.data8[4];
     	  handleButtonBox(hwButtonBox1Lookup(button));
    }
  */
 
    payload[0] = 0x5A; // magic byte "Z" in hex - used to be "test buttons" marker
    payload[1] = 0; // reserved
    payload[2] = TS_HW_BUTTONBOX1_CATEGORY; // hardware button box 1 -> lookup on ECU side via lookup curve table thing;  27 decimal = 0x1B hex
    payload[3] = secondKey & 0xff; // data
    payload[4] = firstKey & 0xff; // data

    Serial.printf("sending %02x %02x", secondKey, firstKey);
    Serial.println();
  
    obdFrame.identifier       = CANBUS_BUTTONBOX_ADDRESS; // CANBUS BUTTONBOX address = 0x711 
    obdFrame.extd             = 0; // standard frame
    obdFrame.data_length_code = 5; // data length code 5 bytes
    obdFrame.data[0]          = payload[0]; // Z - test buttons
    obdFrame.data[1]          = payload[1];    // TS_BUTTONBOX1_CATEGORY = 26, 0x00 0x1A
    obdFrame.data[2]          = payload[2]; // hardware button box 1 -> lookup on ECU side via lookup curve table thing; 
    obdFrame.data[3]          = payload[3]; // data 
    obdFrame.data[4]          = payload[4];  // data

    retry=5;
    while (retry > 0) {
      if (ESP32Can.writeFrame(obdFrame, 5)) break;
      Serial.println("retry");
      retry--;
    }

    while (ESP32Can.inRxQueue() > 0) {
      ESP32Can.readFrame(rxobdFrame, 1);
    }
}

class MyEspUsbHost : public EspUsbHost {
  void onGone(const usb_host_client_event_msg_t *eventMsg) {
    gone = 1;
    Serial.println("device gone");
  };
  void onReceive(const usb_transfer_t *transfer){
      if (!transfer->data_buffer) return;
      int i=0;
      int modifier = 0;
      int firstKey = 0;
      int secondKey = 0;
      for (i = 0;i<transfer->data_buffer_size && i < 50;i++ ){
        Serial.printf("%02x ", transfer->data_buffer[i]);
      }
      Serial.println();

      if (transfer->num_bytes > 4 && transfer->data_buffer_size > 4) { // sanity somewhat here
        modifier  = (transfer->data_buffer[0]);
        firstKey  = (transfer->data_buffer[2]);
        secondKey = (transfer->data_buffer[3]);

         if (firstKey > 0) firstKey += (modifier * 0xff);
         if (secondKey > 0) secondKey += (modifier * 0xff);


        // return; // bit of mojibake here
        if (firstKey > 0) {
          sendCMD( modifier,  firstKey & 0xff,  (firstKey >> 8) & 0xff);
        } else if (secondKey > 0) {
          sendCMD( modifier,  secondKey & 0xff,  (secondKey >> 8) & 0xff);
        }

      }
  }
};

MyEspUsbHost usbHost;

void setup() {
  Serial.begin(115200);
//  delay(500);
  int i;
  tick = xTaskGetTickCount();

gone = 0;
  usbHost.begin();
  usbHost.setHIDLocal(HID_LOCAL_Japan_Katakana);
  usbHost.task();

  ESP32Can.setPins(CAN_TX, CAN_RX);
  ESP32Can.setRxQueueSize(500);
  ESP32Can.setTxQueueSize(500);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));
  while(!ESP32Can.begin());
}

void loop() {
  while (ESP32Can.inRxQueue() > 0) {
    ESP32Can.readFrame(rxobdFrame, 0);
  }
  if (xTaskGetTickCount() - tick > 1000) {
    tick = xTaskGetTickCount();


/*  Serial.printf("txq: %d, rxq: %d, rxerr: %d. txerr: %d, rxmis: %d, txmis: %d, buserr: %d, canstate: %d, notWorking: %d", 
   ESP32Can.inTxQueue(),
   ESP32Can.inRxQueue(),
   ESP32Can.rxErrorCounter(),
   ESP32Can.txErrorCounter(),
   ESP32Can.rxMissedCounter(),
   ESP32Can.txFailedCounter(),
   ESP32Can.busErrCounter(),
   ESP32Can.canState(), notWorking);
    Serial.println();
*/

  if (notWorking > 30 || ESP32Can.busErrCounter() > 30 ) {
    Serial.print(" busErrCounter - restarting esp");
    ESP.restart();
  } 
}
  usbHost.task();

}

