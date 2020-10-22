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

#include <Adafruit_SI1145.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Eeprom.h"
#include "Sensors.h"

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// SI1145 Sensor Address. Default (0x60)
#define SI_ADDR (0x60)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern ESParaSite::opticsData optics;
extern ESParaSite::machineData machine;

extern Adafruit_SI1145 dev_uv;

void ESParaSite::Sensors::initSiSensor() {
  if (!dev_uv.begin()) {
    Serial.print(F("SI1145 Initialization Failure!"));
  } else {
    Serial.println(F("OK!"));
    machine.siDetected = 1;
  }
}

void ESParaSite::Sensors::readSiSensor(bool print) {
  if (machine.siDetected == 1) {
    optics.ledUVIndex = dev_uv.readUV();
    optics.ledUVIndex /= 100.0;
    optics.ledVisible = dev_uv.readVisible();
    optics.ledInfrared = dev_uv.readIR();

#ifdef DEBUG_L2
    Serial.println(F("==========LED Light Sensor=========="));
    print = true;
#endif

    if (print == true) {
      Serial.print(F("UV Index:\t\t\t"));
      Serial.println(static_cast<int>(optics.ledUVIndex));
      Serial.print(F("Visible:\t\t\t"));
      Serial.println(optics.ledVisible);
      Serial.print(F("Infrared:\t\t\t"));
      Serial.println(optics.ledInfrared);
    }
  } else {
    optics.ledUVIndex = 0;
    optics.ledVisible = 0;
    optics.ledInfrared = 0;

#ifdef DEBUG_L2
    Serial.print(F("SI1145 Sensor not found"));
#endif
  }
}
