// SensorsCore.cpp

/* ESParasite Data Logger
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
#include <Arduino.h>
#include <BlueDot_BME280.h>
#include <DHT12.h>
#include <EepromAt24C32.h>
#include <RtcDS3231.h>
#include <Time.h>
#include <Wire.h>

#include "ESParaSite.h"
#include "ConfigPortal.h"
#include "DebugUtils.h"
#include "Eeprom.h"
#include "Sensors.h"
#include "Util.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure
// corrected for the sea level Otherwise leave the standard pressure as default
// (1013.25 hPa)

// Also put in the current average temperature outside (yes, really outside!)
// For slightly less precise altitude measurements, just leave the standard
// temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// DHT12 Sensor Address. Default (0x5C)
#define DHT_ADDR (0x5C)
// SI1145 Sensor Address. Default (0x60)
#define SI_ADDR (0x60)
// MLX90614 Sensor Address. Default (0x5A)
#define MLX_ADDR (0x5A)
// BME280 Sensor Address. Default (0x76) or (0x77)
#define BME_ADDR_A (0x76)
#define BME_ADDR_B (0x77)
// DS3231 Real Time Clock Address. Default (0x68)
#define RTC_ADDR (0x68)
// AT24C32 EEPROM Address. Default (0x50 to 0x57)
#define RTC_EEPROM_BASE_ADDR (0x50)
#define RTC_EEPROM_MAX_ADDR (0x57)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

#ifndef DEBUG_L2
bool dump_sensor_data = 0;
#endif
#ifdef DEBUG_L2
bool dump_sensor_data = 1;
#endif

int8_t bme_i2c_address;
int8_t eeprom_i2c_address;

extern ESParaSite::chamberData chamber;
extern ESParaSite::opticsData optics;
extern ESParaSite::ambientData ambient;
extern ESParaSite::enclosureData enclosure;
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::sensorExists exists;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> rtc(Wire);
BlueDot_BME280 bme;
DHT12 dht;

// This gives us a nicely formatted dump of all sensor data to Serial console.
void ESParaSite::Sensors::dumpSensor(bool in_seq_read) {
  dump_sensor_data = 1;
  Serial.println();
  Serial.println(F("Current Sensor Readings"));
  Serial.println(
      F("============================================================"));
  Serial.println();

  Serial.println(F("DS3231 Real-Time Clock Timestamp and Temperature:"));
  readRtcData();
  Serial.println();

  Serial.println(F("DHT12 Print Chamber Environmental Data:"));
  if (in_seq_read == true) {
    readDhtSensor(true);
  } else {
    readDhtSensor(false);
  }
  Serial.println();

  Serial.println(F("SI1145 UV and Light Sensor Data:"));
  readSiSensor();
  Serial.println();

  Serial.println(F("MLX90614 LED Temp Sensor Data:"));
  readMlxSensor();
  Serial.println();

  Serial.println(F("BME280 Ambient Temp Sensor Data:"));
  readBmeSensor();
  Serial.println();

  Serial.print(F("This Printer has been on for:\t"));
  Serial.print(enclosure.printerLifeSec);
  Serial.println(F("  seconds"));
  Serial.println();

#ifndef DEBUG_L2
  dump_sensor_data = 0;
#endif
}

int8_t ESParaSite::Sensors::pingSensor(uint16_t address) {
  // This function provides us a handy way to ping I2C addresses.
  byte error;
  Wire.beginTransmission(address);
  error = Wire.endTransmission();

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
  } else if (error == 4) {
    Serial.print(F("Unknown error at address 0x"));
    if (address < 16)
      Serial.print(F("0"));
    Serial.print(address, HEX);
    Serial.print(F("\t\t"));
  }
  return error;
}