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

#include <Arduino.h>
#include <DHT12.h>
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

extern ESParaSite::chamber chamberResource;
extern ESParaSite::sensorExists existsResource;

extern DHT12 dht;

void ESParaSite::Sensors::initDhtSensor() {
  int status = dht.read();
  switch (status) {
  case DHT12_OK:
    Serial.print(F("OK!\t"));
    existsResource.dhtDetected = 1;
    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.print(F("DHT12 Checksum error,\t"));
    break;
  case DHT12_ERROR_CONNECT:
    Serial.print(F("DHT12 Connect error,\t"));
    break;
  case DHT12_MISSING_BYTES:
    Serial.print(F("DHT12 Missing bytes,\t"));
    break;
  default:
    Serial.print(F("DHT12 Unknown error,\t"));
    break;
  }
}

void ESParaSite::Sensors::readDhtSensor(bool in_seq_read) {
  // First dht measurement is stale, so if in_seq_read is 'false' (when read is
  // out-of-sequence or, in other words, not called by a timer in the loop() ),
  // we read, wait 2 seconds, then read again. Normal in sequence reads do not
  // need this, so we send a 'true' to skip the additional delay.
  if (existsResource.dhtDetected == 1) {
    int status = dht.read();

    if (!in_seq_read) {
      switch (status) {
      case DHT12_OK:
        break;
      case DHT12_ERROR_CHECKSUM:
        Serial.print(F("Checksum error,\t"));
        break;
      case DHT12_ERROR_CONNECT:
        Serial.print(F("Connect error,\t"));
        break;
      case DHT12_MISSING_BYTES:
        Serial.print(F("Missing bytes,\t"));
        break;
      default:
        Serial.print(F("Unknown error,\t"));
        break;
      }
      delay(2000);
      status = dht.read();
    }

    switch (status) {
    case DHT12_OK:
      chamberResource.chamberTempC = dht.temperature;
      chamberResource.chamberHumidity = dht.humidity;
      chamberResource.chamberDewPoint = (Util::dewPoint(
          chamberResource.chamberTempC, chamberResource.chamberHumidity));

#ifdef DEBUG_L2
      Serial.println(F("==========Print Chamber=========="));
      Serial.print(F("Temperature:\t\t\t"));
      Serial.print(chamberResource.chamberTempC, 1);
      Serial.print("°C / ");
      Serial.print(Util::convertCtoF(chamberResource.chamberTempC));
      Serial.println("°F");
      Serial.print(F("Humidity:\t\t\t"));
      Serial.print(chamberResource.chamberHumidity, 1);
      Serial.println("%");
      Serial.print(F("Dew Point:\t\t\t"));
      Serial.print(static_cast<int>(chamberResource.chamberDewPoint));
      Serial.print("°C / ");
      Serial.print(Util::convertCtoF(chamberResource.chamberDewPoint));
      Serial.println("°F");
# endif

    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.print(F("DHT12 Checksum error,\t"));
    break;
  case DHT12_ERROR_CONNECT:
    Serial.print(F("DHT12 Connect error,\t"));
    break;
  case DHT12_MISSING_BYTES:
    Serial.print(F("DHT12 Missing bytes,\t"));
    break;
  default:
    Serial.print(F("DHT12 Unknown error,\t"));
    break;
  }

  } else {
    chamberResource.chamberTempC = 0;
    chamberResource.chamberHumidity = 0;
    chamberResource.chamberDewPoint = 0;

#ifdef DEBUG_L2
    Serial.print(F("DHT12 Sensor not found"));
#endif

  }
}
