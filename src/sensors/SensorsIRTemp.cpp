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
// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// MLX90614 Sensor Address. Default (0x5A)
#define MLX_ADDR (0x5A)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern ESParaSite::opticsData optics;
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::sensorExists exists;

extern Adafruit_MLX90614 mlx;

void ESParaSite::Sensors::initMlxSensor() {
  if (!mlx.begin()) {
    Serial.print(F("MLX90614 Initialization Failure"));
  } else {
    Serial.print(F("OK!"));
    exists.mlxDetected = 1;
  }
}

void ESParaSite::Sensors::readMlxSensor() {
  if (exists.mlxDetected == 1) {
    optics.ledTempC = roundf(mlx.readAmbientTempC() *100) / 100;
    optics.screenTempC = roundf(mlx.readObjectTempC() * 100) / 100;

#ifdef DEBUG_L2
    Serial.println("==========LCD Temp Sensor==========");
    Serial.print("Ambient:\t\t\t");
    Serial.print(optics.ledTempC);
    Serial.print("°C / ");
    Serial.print(mlx.readAmbientTempF());
    Serial.println("°F");
    Serial.print("LCD Panel:\t\t\t");
    Serial.print(optics.screenTempC);
    Serial.print("°C / ");
    Serial.print(mlx.readObjectTempF());
    Serial.println("°F");
#endif

  } else {
    optics.ledTempC = 0;
    optics.screenTempC = 0;

#ifdef DEBUG_L2
    Serial.print(F("MLX90614 Sensor not found"));
#endif
  }
}
