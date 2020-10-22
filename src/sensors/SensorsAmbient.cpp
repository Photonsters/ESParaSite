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
#include <BlueDot_BME280.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Sensors.h"
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

extern ESParaSite::ambientData ambient;
extern ESParaSite::machineData machine;

extern BlueDot_BME280 dev_bme;

void ESParaSite::Sensors::initBmeSensor()
{
  // initialize BME280 temperature sensor
  dev_bme.parameter.communication = 0;
  dev_bme.parameter.I2CAddress = machine.bmeI2cAddress;
  dev_bme.parameter.sensorMode = 0b11;
  dev_bme.parameter.IIRfilter = 0b100;
  dev_bme.parameter.humidOversampling = 0b101;
  dev_bme.parameter.tempOversampling = 0b101;
  dev_bme.parameter.pressOversampling = 0b101;
  dev_bme.parameter.pressureSeaLevel = SEALEVELPRESSURE_HPA;
  dev_bme.parameter.tempOutsideCelsius = CURRENTAVGTEMP_C;
  dev_bme.parameter.tempOutsideFahrenheit = CURRENTAVGTEMP_F;

  if (dev_bme.init() != 0x60)
  {
    Serial.print(F("BME280 Sensor not found!"));
  }
  else
  {
    Serial.println(F("OK!"));
    machine.bmeDetected = 1;
  }
}

void ESParaSite::Sensors::readBmeSensor(bool print)
{
  if (machine.bmeDetected == 1)
  {
    float bmeTempC = dev_bme.readTempC();
    float bmeHumidity = dev_bme.readHumidity();
    ambient.ambientTempC = ESParaSite::Util::floatToTwo(bmeTempC);
    ambient.ambientHumidity = ESParaSite::Util::floatToTwo(bmeHumidity);
    ambient.ambientBarometer = dev_bme.readPressure();
    ambient.ambientAltitude = dev_bme.readAltitudeMeter();
    ambient.ambientDewPoint = ESParaSite::Util::dewPoint(bmeTempC, bmeHumidity);

#ifdef DEBUG_L2
    Serial.println("==========Ambient Conditions==========");
    print = true;
#endif

if (print == true){
    Serial.print(F("Temperature:\t\t\t"));
    Serial.print(dev_bme.readTempC());
    Serial.print("°C / ");
    Serial.print(dev_bme.readTempF());
    Serial.println("°F");
    Serial.print(F("Relative Humidity:\t\t"));
    Serial.print(dev_bme.readHumidity());
    Serial.println("%");
    Serial.print(F("Barometric Pressure:\t\t"));
    Serial.print(ambient.ambientBarometer);
    Serial.println(" hPa");
    Serial.print(F("Altitude:\t\t\t"));
    Serial.print(ambient.ambientAltitude);
    Serial.print("m / ");
    Serial.print(dev_bme.readAltitudeFeet());
    Serial.println("ft");
    Serial.print(F("Dew Point:\t\t\t"));
    Serial.print(ambient.ambientDewPoint);
    Serial.println("°C");
    Serial.println();
}
  }
  else
  {
    ambient.ambientTempC = 0;
    ambient.ambientHumidity = 0;
    ambient.ambientBarometer = 0;
    ambient.ambientAltitude = 0;
    ambient.ambientDewPoint = 0;

#ifdef DEBUG_L2
    Serial.print(F("BME280 Sensor not found"));
#endif
  }
}
