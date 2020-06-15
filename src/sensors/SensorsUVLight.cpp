// SensorsCore.cpp

/* ESParasite Data Logger
        Authors: Andy (DocMadmag) Eakin

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

#include <Adafruit_SI1145.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
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

extern ESParaSite::optics opticsResource;
extern ESParaSite::sensorExists existsResource;

extern Adafruit_SI1145 uv;

void ESParaSite::Sensors::initSiSensor() {
  if (!uv.begin()) {
    Serial.print(F("SI1145 Initialization Failure!"));
  } else {
    Serial.print(F("OK!"));
    existsResource.siDetected = 1;
  }
}

void ESParaSite::Sensors::readSiSensor() {
  if (existsResource.siDetected == 1) {
    opticsResource.ledUVIndex = uv.readUV();
    opticsResource.ledUVIndex /= 100.0;
    opticsResource.ledVisible = uv.readVisible();
    opticsResource.ledInfrared = uv.readIR();

#ifdef DEBUG_L2
    Serial.println(F("==========LED Light Sensor=========="));
    Serial.print(F("UV Index:\t\t\t"));
    Serial.println(static_cast<int>(opticsResource.ledUVIndex));
    Serial.print(F("Visible:\t\t\t"));
    Serial.println(opticsResource.ledVisible);
    Serial.print(F("Infrared:\t\t\t"));
    Serial.println(opticsResource.ledInfrared);
#endif

  } else {
    opticsResource.ledUVIndex = 0;
    opticsResource.ledVisible = 0;
    opticsResource.ledInfrared = 0;

#ifdef DEBUG_L2
    Serial.print(F("SI1145 Sensor not found"));
#endif

  }
}
