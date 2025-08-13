#include "LoraSuite.h"
#include "DisplaySuite.h"

#define ARM_CMD     0x10
#define DISARM_CMD  0x11

TaskHandle_t gRxTaskHandle = nullptr;
uint8_t arm_pkg[] = { ARM_CMD };
uint8_t disarm_pkg[] = { DISARM_CMD };

void sendSerialCmdTask(void *_) {
  for (;;) {
    if (Serial.available()) {
      int receivedByte = Serial.read();
      switch (receivedByte) {
        case ARM_CMD:
            loraSend(arm_pkg, sizeof(arm_pkg), 3000);
            Serial.println("# TX LoRa: ARM");
            break;
        case DISARM_CMD:
            loraSend(disarm_pkg, sizeof(disarm_pkg), 3000);
            Serial.println("# TX LoRa: DISARM");
            break;
        case '\n':
            break;
        default:
            Serial.println("UNKNOWN ");
            Serial.println(receivedByte);
            break;
      }
    }
    vTaskDelay(1);
  }
}

void receiverTask(void *_) {
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
  xTaskCreatePinnedToCore(receiverTask, "LoRaRX", 4096, nullptr, 1, &gRxTaskHandle, 0);
  xTaskCreatePinnedToCore(sendSerialCmdTask, "LoRaTX", 4096, nullptr, 1, nullptr, 0);
}


void loop() {
}
