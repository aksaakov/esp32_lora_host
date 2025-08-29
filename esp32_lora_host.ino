#include "DisplaySuite.h"
#include "LoraController.h"
#include "LoRaWan_APP.h"
#include "Helpers.h"
#include "Protocol.h"

volatile bool isArmed = false;

void sendAndAwait(const uint8_t* payload, size_t len, uint32_t ms) {
  sendMessage(payload, len);
  radioAwaitTxDone(ms);
}

void onMotion(int16_t rssi, int8_t snr) {
  Serial.write((uint8_t)0x01);
  Serial.write((uint8_t)rssi);
  Serial.write((uint8_t)snr);
  Serial.write('\n'); 
  Serial.println("received -> 0x01 code: Motion Detected.");
  isArmed ? sendAndAwait(START_ALARM_PKG, 1, 600) : sendAndAwait(STOP_ALARM_PKG, 1, 600);
}

void processReceivedPacket(const uint8_t* data, uint16_t len, int16_t rssi, int8_t snr) {
  Serial.printf("data=%u, rssi=%d, snr=%d\n", data[0], rssi, snr);

  switch(data[0]) {
    case 0x01:
      onMotion(rssi, snr);
      break; 
    case 0x20:
      Serial.println("Alarm is ON! Replying with current alarm switch status.");
      isArmed ? sendAndAwait(START_ALARM_PKG, 1, 600) : sendAndAwait(STOP_ALARM_PKG, 1, 600);
      radioAwaitTxDone(600);
      break;       
    case 0x80: {
      if (len >= 2) {
        uint8_t pct = data[1];
        Serial.write((uint8_t)0x80);
        Serial.write(pct);
        Serial.write('\n');
        Serial.printf("received -> 0x80 code: BATTERY %u%%\n", pct);
      }
      break;
    }
    default:
      Serial.println("received -> UNKNOWN code or data not binary.");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  displayLogo();
  syncArmStateFromHA();
  radioInit();
}

void loop() {
  while (Serial.available() > 0) {
    int receivedByte = Serial.read();
    if (receivedByte > -1) {
      switch (receivedByte) {
        case START_ALARM_CODE:
          Serial.println("HA: ARM");
          isArmed = true;
          break;
        case STOP_ALARM_CODE:
          Serial.println("HA: DISARM");
          isArmed = false;
          break;
        default:
        Serial.println("Unknown message from HA.");
      }
    } else {
      Serial.println("Ignoring: message is not accepted.");
    }
  }

  radioService();

  delay(50);
}
