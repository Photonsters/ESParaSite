// SensorsCore.cpp

/* ESParasite Data Logger v0.9
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
#include <BlueDot_BME280.h>
#include <Wire.h>

#include "ESParaSite.h"
#include "ConfigPortal.h"
#include "DebugUtils.h"
#include "RtcEepromCore.h"
#include "SensorsCore.h"
#include "Util.h"

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure
// corrected for the sea level Otherwise leave the standard pressure as default
// (1013.25 hPa)

// Also put in the current average temperature outside (yes, really outside!)
// For less precise altitude measurements, just leave the standard
// temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// BME280 Sensor Address. Default (0x76) or (0x77)
#define BME_ADDR_A (0x76)
#define BME_ADDR_B (0x77)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern int bme_i2c_address;

extern ESParaSite::ambient ambientResource;
extern ESParaSite::sensorExists existsResource;

extern BlueDot_BME280 bme;

void ESParaSite::Sensors::initBmeSensor() {
  // initialize BME280 temperature sensor
  bme.parameter.communication = 0;
  bme.parameter.I2CAddress = bme_i2c_address;
  bme.parameter.sensorMode = 0b11;
  bme.parameter.IIRfilter = 0b100;
  bme.parameter.humidOversampling = 0b101;
  bme.parameter.tempOversampling = 0b101;
  bme.parameter.pressOversampling = 0b101;
  bme.parameter.pressureSeaLevel = SEALEVELPRESSURE_HPA;
  bme.parameter.tempOutsideCelsius = CURRENTAVGTEMP_C;
  bme.parameter.tempOutsideFahrenheit = CURRENTAVGTEMP_F;

  if (bme.init() != 0x60) {
    Serial.print(F("BME280 Sensor not found!"));
  } else {
    Serial.print(F("OK!"));
    existsResource.bmeDetected = 1;
  }
}

void ESParaSite::Sensors::readBmeSensor() {
  if (existsResource.bmeDetected == 1) {
    ambientResource.ambientTempC = roundf(bme.readTempC() * 100) / 100;
    ambientResource.ambientHumidity = roundf(bme.readHumidity() * 100) / 100;
    ambientResource.ambientBarometer = bme.readPressure();
    ambientResource.ambientAltitude = bme.readAltitudeMeter();

#ifdef DEBUG_L2
    Serial.println("==========Ambient Conditions==========");
    Serial.print(F("Temperature:\t\t\t"));
    Serial.print(ambientResource.ambientTempC);
    Serial.print("°C / ");
    Serial.print(bme.readTempF());
    Serial.println("°F");
    Serial.print(F("Relative Humidity:\t\t"));
    Serial.print(ambientResource.ambientHumidity);
    Serial.println("%");
    Serial.print(F("Barometric Pressure:\t\t"));
    Serial.print(ambientResource.ambientBarometer);
    Serial.println(" hPa");
    Serial.print(F("Altitude:\t\t\t"));
    Serial.print(ambientResource.ambientAltitude);
    Serial.print("m / ");
    Serial.print(bme.readAltitudeFeet());
    Serial.println("ft");
#endif

  } else {
    ambientResource.ambientTempC = 0;
    ambientResource.ambientHumidity = 0;
    ambientResource.ambientBarometer = 0;
    ambientResource.ambientAltitude = 0;

#ifdef DEBUG_L2
    Serial.print(F("BME280 Sensor not found"));
#endif

  }
}
