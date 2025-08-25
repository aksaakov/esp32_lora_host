#include "DisplaySuite.h"
#include "LoraController.h"
#include "LoRaWan_APP.h"
#include "Helpers.h"
#include "Protocol.h"

TaskHandle_t receiver = nullptr;

volatile uint8_t  expectedAck = 0;
volatile bool     ackReceived = false;
volatile bool     isArmed = false;

constexpr uint8_t STATUS_OPT_NONE   = 0x00;
constexpr uint8_t STATUS_OPT_MOTION = 0x01;

void setup() {
  Serial.begin(115200);
  delay(300);
  displayLogo();
  syncArmStateFromHA();
  radioInit();
}

bool sendMessage(const uint8_t* package,
                 uint8_t maxAttempts = 15,
                 uint16_t ackTimeoutMs = 1200)
{
  const uint8_t code = package[0];
  for (uint8_t attempt = 1; attempt <= maxAttempts; ++attempt) {
    expectedAck = code;
    ackReceived = false;

    Radio.Standby();
    Serial.printf(">>> Sending package: 0x%02X (attempt %u/%u) <<<\n", code, attempt, maxAttempts);
    Radio.Send((uint8_t*)package, 1);

    uint32_t startMs = millis();
    while (millis() - startMs < ackTimeoutMs) {
      Radio.IrqProcess();
      if (ackReceived) { expectedAck = 0; return true; }

      if (onNewCommand(code)) {
        Serial.println(">>> New opposite command arrived — cancel current send");
        expectedAck = 0;
        return false;
      }

      delay(1);
    }

    // small backoff
    uint16_t backoffMs = 30 + (uint16_t)random(0, 90);
    uint32_t b0 = millis();
    while (millis() - b0 < backoffMs) { Radio.IrqProcess(); delay(1); }
  }
  expectedAck = 0;
  Serial.println("!! No ACK received after retries.");
  return false;
}

void onMotion(int16_t rssi, int8_t snr) {
  Serial.write((uint8_t)0x01); // 1 byte = MOTION
  Serial.write((uint8_t)rssi); // signal strength (GOOD 0-90 BAD)
  Serial.write((uint8_t)snr);  // signal-to-noise (GOOD > -5dB < BAD)
  Serial.write('\n'); 
  Serial.println("received -> 0x01 code: Motion Detected.");
}

void processReceivedPacket(const uint8_t* data, uint16_t len, int16_t rssi, int8_t snr) {
  Serial.printf("data=%u, rssi=%d, snr=%d\n", data[0], rssi, snr);
  const uint8_t* alarmStatusReply = isArmed ? ARM_PKG : DISARM_PKG;

  switch(data[0]) {
    case 0x01:
      onMotion(rssi, snr);
      break; 
    case 0x90: {
      Serial.println("received -> 0x90 code: Request ARM/UNARM status.");
      const uint8_t optionalByte = (len >= 2) ? data[1] : STATUS_OPT_NONE;

      if (optionalByte == STATUS_OPT_MOTION) {
        onMotion(rssi, snr);
      }

      sendMessage(alarmStatusReply);
      break;
    }
    case 0x10:
      if (expectedAck == 0x10) { 
        ackReceived = true; 
        Serial.println("[Ack] Arm OK"); 
      }
      else { 
        Serial.println("received -> 0x10 (unexpected)"); 
      }
      break;
    case 0x11:
      if (expectedAck == 0x11) { 
        ackReceived = true; 
        Serial.println("[Ack] Disarm OK"); 
      }
      else { 
        Serial.println("received -> 0x11 (unexpected)");
      }
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

void loop() {
  while (Serial.available() > 0) {
    int receivedByte = Serial.read();
    if (receivedByte > -1) {
      switch (receivedByte) {
        case ARM_CMD:
          Serial.println("HA: ARM");
          isArmed = true;
          sendMessage(ARM_PKG);
          break;
        case DISARM_CMD:
          Serial.println("HA: DISARM");
          isArmed = false;
          sendMessage(DISARM_PKG);
          break;
        default:
        Serial.println("Unknown message from HA.");
      }
    } else {
      Serial.println("Ignoring: message is not accepted.");
    }
  }

  Radio.IrqProcess();
  delay(1000);
}
