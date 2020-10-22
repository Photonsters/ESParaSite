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
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Eeprom.h"
#include "Sensors.h"
#include "Util.h"

//+++ Advanced Settings +++
// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// MLX90614 Sensor Address. Default (0x5A)
#define MLX_ADDR (0x5A)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern ESParaSite::opticsData optics;
extern ESParaSite::machineData machine;

extern Adafruit_MLX90614 dev_mlx;

void ESParaSite::Sensors::initMlxSensor() {
  if (!dev_mlx.begin()) {
    Serial.print(F("MLX90614 Initialization Failure"));
  } else {
    Serial.println(F("OK!"));
    machine.mlxDetected = 1;
  }
}

void ESParaSite::Sensors::readMlxSensor(bool print) {
  if (machine.mlxDetected == 1) {
    optics.ledTempC = ESParaSite::Util::floatToTwo(dev_mlx.readAmbientTempC());
    optics.screenTempC =
        ESParaSite::Util::floatToTwo(dev_mlx.readObjectTempC());

#ifdef DEBUG_L2
    Serial.println("==========LCD Temp Sensor==========");
    print = true;
#endif

    if (print == true) {
      Serial.print("LED Array:\t\t\t");
      Serial.print(dev_mlx.readAmbientTempC());
      Serial.print("°C / ");
      Serial.print(dev_mlx.readAmbientTempF());
      Serial.println("°F");
      Serial.print("LCD Panel:\t\t\t");
      Serial.print(dev_mlx.readObjectTempC());
      Serial.print("°C / ");
      Serial.print(dev_mlx.readObjectTempF());
      Serial.println("°F");
    }

  } else {
    optics.ledTempC = 0;
    optics.screenTempC = 0;

#ifdef DEBUG_L2
    Serial.print(F("MLX90614 Sensor not found, skipping"));
#endif
  }
}
