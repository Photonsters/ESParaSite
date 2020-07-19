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

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// DHT12 Sensor Address. Default (0x5C)
#define DHT_ADDR (0x5C)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

extern ESParaSite::chamberData chamber;
extern ESParaSite::sensorExists exists;

extern DHT12 dht;

void ESParaSite::Sensors::initDhtSensor()
{
  int8_t status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    Serial.print(F("OK!\t"));
    exists.dhtDetected = 1;
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

void ESParaSite::Sensors::readDhtSensor(bool inLoopRead)
{
  // First dht measurement is stale, so if inLoopRead is 'false' (when read is
  // out-of-loop or, in other words, not called by a timer in the loop() ),
  // we read, wait 2 seconds, then read again. Normal in sequence reads do not
  // need this, so we send a 'true' to skip the additional delay.
  if (exists.dhtDetected == 1)
  {
    int8_t status = dht.read();

    if (!inLoopRead)
    {
      switch (status)
      {
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

    switch (status)
    {
    case DHT12_OK:
      chamber.chamberTempC = ESParaSite::Util::floatToInt(dht.temperature);
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

#ifdef DEBUG_L2
    Serial.println(F("==========Print Chamber=========="));
    Serial.print(F("Temperature:\t\t\t"));
    Serial.print(chamber.chamberTempC, 1);
    Serial.print("°C / ");
    Serial.print(Util::convertCtoF(chamber.chamberTempC));
    Serial.println("°F");
#endif
  }
  else
  {
    chamber.chamberTempC = 0;

#ifdef DEBUG_L2
    Serial.print(F("DHT12 Sensor not found"));
#endif
  }
}
