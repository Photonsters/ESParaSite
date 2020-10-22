// SensorsCore.cpp

/* ESParaSite-ESP32 Data Logger
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
        source.

        All Original content is free and unencumbered software released into the
        public domain.

        The Author(s) are extremely grateful for the amazing open source
        communities that work to support all of the sensors, microcontrollers,
        web standards, etc.
*/

#include <Adafruit_MLX90614.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_Sensor.h>
#include <BlueDot_BME280.h>
#include <DHT12.h>
#include <I2C_eeprom.h>
#include <RtcDS3231.h>
#include <Wire.h>

#include "ESParaSite.h"
#include "Sensors.h"


DHT12 dev_dht;
Adafruit_SI1145 dev_uv;
Adafruit_MLX90614 dev_mlx;
BlueDot_BME280 dev_bme;
RtcDS3231<TwoWire> dev_rtc(Wire);

int8_t ESParaSite::Sensors::pingSensor(uint16_t address) {
  // This function provides us a handy way to ping I2C addresses.
  byte error;
  Wire.beginTransmission(address);
  Wire.write(0);
  error = Wire.endTransmission(true);

  if (error == 0) {
    Serial.print(F("I2C device found at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  } else if (error == 2) {
    Serial.print(F("No device (Ping recieved NACK) at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  } else if (error == 3) {
    Serial.print(F("Device not Responding (Timeout) at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  } else if (error == 4) {
    Serial.print(F("Unknown error at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  } else {
    Serial.print(F("I2C Error "));
    Serial.print(error);
    Serial.print(F(" at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  }
  return error;
}

// This gives us a nicely formatted dump of all sensor data to Serial console.
void ESParaSite::Sensors::dumpSensor(bool in_seq_read) {
  Serial.println();
  Serial.println(F("Current Sensor Readings"));
  Serial.println(
      F("============================================================"));
  Serial.println();

  Serial.println(F("DS3231 Real-Time Clock Timestamp and Temperature:"));
  ESParaSite::Sensors::readRtcData(true);
  ESParaSite::Sensors::readRtcTemp(true);
  Serial.println();

  Serial.println(F("DHT12 Print Chamber Environmental Data:"));
  if (in_seq_read == true) {
    ESParaSite::Sensors::readDhtSensor(true, true);
  } else {
    ESParaSite::Sensors::readDhtSensor(false, true);
  }
  Serial.println();

  Serial.println(F("SI1145 UV and Light Sensor Data:"));
  ESParaSite::Sensors::readSiSensor(true);
  Serial.println();

  Serial.println(F("MLX90614 LED Temp Sensor Data:"));
  ESParaSite::Sensors::readMlxSensor(true);
  Serial.println();

  Serial.println(F("BME280 Ambient Temp Sensor Data:"));
  ESParaSite::Sensors::readBmeSensor(true);
  Serial.println();


}
