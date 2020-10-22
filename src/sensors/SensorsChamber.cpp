// SensorsChamber.cpp

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

#include <Arduino.h>
#include <DHT12.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Sensors.h"
#include "Util.h"

//+++ Advanced Settings +++

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// DHT12 Sensor Address. Default (0x5C)
#define DHT_ADDR (0x5C)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern DHT12 dev_dht;

extern ESParaSite::chamberData chamber;
extern ESParaSite::machineData machine;

void ESParaSite::Sensors::initDhtSensor() {
  dev_dht.begin();
  int8_t status = dev_dht.readStatus();
  switch (status) {
  case DHT12::OK:
    Serial.println(F("OK!"));
    machine.dhtDetected = 1;
    break;
  case DHT12::ERROR_CHECKSUM:
    Serial.println(F("Checksum error"));
    break;
  case DHT12::ERROR_TIMEOUT:
    Serial.println(F("Timeout error"));
    break;
  case DHT12::ERROR_TIMEOUT_LOW:
    Serial.println(
        F("Timeout error on low signal, try put high pullup resistance"));
    break;
  case DHT12::ERROR_TIMEOUT_HIGH:
    Serial.println(
        F("Timeout error on low signal, try put low pullup resistance"));
    break;
  case DHT12::ERROR_CONNECT:
    Serial.println(F("Connect error"));
    break;
  case DHT12::ERROR_ACK_L:
    Serial.println(F("AckL error"));
    break;
  case DHT12::ERROR_ACK_H:
    Serial.println(F("AckH error"));
    break;
  case DHT12::ERROR_UNKNOWN:
    Serial.println(F("Unknown error DETECTED"));
    break;
  case DHT12::NONE:
    Serial.println(F("No result"));
    break;
  default:
    Serial.println(F("Unknown error"));
    break;
  }
}

void ESParaSite::Sensors::readDhtSensor(bool inLoopRead, bool print) {
  // First dht measurement is stale, so if inLoopRead is 'false' (when read is
  // out-of-loop or, in other words, not called by a timer in the loop() ),
  // we read, wait 2 seconds, then read again. Normal in sequence reads do not
  // need this, so we send a 'true' to skip the additional delay.
  if (machine.dhtDetected == 1) {

    bool dht12Read = true;
    if (!inLoopRead) {
      // Read temperature as Celsius (the default)
      float dhtTempC = dev_dht.readTemperature();
      // Check if any reads failed and exit early (to try again).

      if (isnan(dhtTempC)) {
        Serial.println("Failed to read from DHT12 sensor!");

        dht12Read = false;
      }
      if (dht12Read) {
        delay(2000);
      }
    }

    if (dht12Read) {
      // Read temperature as Celsius (the default)
      float dhtTempC = dev_dht.readTemperature();
      chamber.chamberTempC = ESParaSite::Util::floatToTwo(dhtTempC);

      if (isnan(dhtTempC)) {
        Serial.println("Failed to read from DHT12 sensor!");
        chamber.chamberTempC = 0;
      }
    }

#ifdef DEBUG_L2
    Serial.println(F("==========Print Chamber Sensor=========="));
    print = true;
#endif

    if (print == true) {
      Serial.print(F("Temperature:\t\t\t"));
      Serial.print((chamber.chamberTempC));
      Serial.print("°C / ");
      Serial.print(Util::convertCtoF((chamber.chamberTempC)));
      Serial.println("°F");
    }
  }
}