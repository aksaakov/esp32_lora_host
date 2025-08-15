#include "DisplaySuite.h"
#include "LoraController.h"
#include "LoRaWan_APP.h"

TaskHandle_t receiver = nullptr;

constexpr uint8_t ARM_CMD     = 0x10;
constexpr uint8_t DISARM_CMD  = 0x11;
constexpr uint8_t IGNORED_SERIAL_DATA = 0xFF;

void setup() {
  Serial.begin(115200);
  delay(200);
  displayLogo();
  radioInit();
}

void processReceivedPacket(const uint8_t* data, uint16_t len, int16_t rssi, int8_t snr) {
  Serial.printf("data=%u, rssi=%d, snr=%d\n", data[0], rssi, snr);

  switch(data[0]) {
    case 0x01:
      Serial.write((uint8_t)0x01); // 1 byte = MOTION
      Serial.write((uint8_t)rssi); // signal strength (GOOD 0-90 BAD)
      Serial.write((uint8_t)snr);  // signal-to-noise (GOOD > -5dB < BAD)
      Serial.write('\n'); 
      Serial.println("received -> 0x01 code: Motion Detected.");
      break; 
    default:
      Serial.println("received -> UNKNOWN code or data not binary.");
      break;
  }
}

void sendMessage(const uint8_t* package) {
  Radio.Standby();
  Serial.printf(">>> Sending package: 0x%02X <<<\n",  package[0]);
  Radio.Send( (uint8_t *)package, 1);
}

void loop() {

  while (Serial.available() > 0) {
    int receivedByte = Serial.read();
    if (receivedByte > -1 && (receivedByte == ARM_CMD || receivedByte == DISARM_CMD)) {
      const uint8_t byteFromSerial = (uint8_t)receivedByte;
      sendMessage(&byteFromSerial);
    } else {
      Serial.println("Ignoring: message is not accepted.");
    }
  }

  Radio.IrqProcess();
  delay(1000);
}
