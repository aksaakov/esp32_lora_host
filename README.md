## ESP32 LoRa Host

This project is the host side of a two-way LoRa communication system built with Heltec ESP32 v3 LoRa boards.
The host is intended to run on a Raspberry Pi (or similar) and acts as a bridge between LoRa devices and Home Assistant via MQTT.

It provides:
Reliable two-way communication with a standalone remote ESP32 LoRa device.
MQTT integration for motion detection, alarm control, and battery reporting.
_For the MQTT bridge code and HA configuration just vibe it_
Automatic re-arm logic (configurable from Home Assistant).
Simple serial logging for debugging.

**Features**

## Features

- **LoRa ↔ MQTT bridge**: Receives LoRa packets from the remote and publishes them to MQTT topics, and vice-versa.  
- **Alarm control**: Host can remotely arm/disarm the remote alarm.  
- **Motion events**: Motion detected by the remote is published as MQTT events.  
- **Battery reporting**: Remote device sends periodic battery % updates, which the host publishes as an MQTT sensor.  
- **Auto-arm toggle**: Configurable from Home Assistant. If enabled, the alarm re-arms automatically after a timeout.  
- **Debug logging**: Prints all LoRa activity and MQTT publishes/subscribes to the console.  

## Hardware Requirements

Host: Raspberry Pi (any model with USB), running Linux.
LoRa board: Heltec ESP32 v3 LoRa module (connected to Pi over USB).
Remote device: Another Heltec ESP32 v3 (and other components specified in the remote repo).
).

To see the host esp32 repo go [here](https://github.com/aksaakov/esp32_lora_remote).
