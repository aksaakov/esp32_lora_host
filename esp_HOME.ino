#include "LoraSuite.h"
#include "DisplaySuite.h"

void startReceiverTask(void *_) {
  uint8_t buf[32];
  int16_t rssi;
  int8_t  snr;

  for (;;) {
    int n = loraReceive(buf, sizeof(buf), &rssi, &snr, 0, true /*no sleep*/);

    if (n == 1) {
      uint8_t code = buf[0];
      // Serial.printf("EV:%02X RSSI:%d SNR:%d\n", code, rssi, snr);
        switch (buf[0]) {
          case 0x01: 
            Serial.write((uint8_t)0x01); // 1 byte = MOTION
            Serial.write((uint8_t)rssi); // signal strength (GOOD 0-90 BAD)
            Serial.write((uint8_t)snr); // signal-to-noise (GOOD > -5dB < BAD)
            Serial.write('\n'); 
            Serial.println("Event: MOTION"); 
          break;
          // case 0x02: Serial.println("Event: TODO");  
          // break;
          default:
            Serial.write((uint8_t)0xFF);  // unknown
            Serial.write((uint8_t)rssi); // signal strength (GOOD 0-90 BAD)
            Serial.write((uint8_t)snr); // signal-to-noise (GOOD > -5dB < BAD)
            Serial.write('\n');
            Serial.println("Event: UNKNOWN"); 
          break;
        }
      }

    // Small yield so FreeRTOS can switch tasks
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  displayLogo();
  xTaskCreatePinnedToCore(startReceiverTask, "LoRaRX", 4096, nullptr, 1, nullptr, 0);
}


void loop() {
}
