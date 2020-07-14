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
extern int8_t bme_i2c_address;
extern int8_t eeprom_i2c_address;

extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::sensorExists exists;

extern Adafruit_MLX90614 mlx;
extern Adafruit_SI1145 uv;
extern RtcDS3231<TwoWire> rtc;
extern BlueDot_BME280 bme;
extern DHT12 dht;

// Since we use libraries by different authors and not all libraries talk to
// sensors in the same way, initI2cSensors and pingSensor give us a cleaner,
// abstracted format to check for the sensors before calling the
// init_xyz_sensor() methods which are unique to the sensor and library chosen.
// This will allow us to extend sensor support over time in a more elegant
// fashion.

void ESParaSite::Sensors::initI2cSensors() {
  // initialize I2C bus
  Serial.print(F("Init I2C bus..."));
  Wire.begin(config.cfgPinSda, config.cfgPinScl);
  Serial.println(F("\t\t\t\t\tOK!"));
  Serial.println();

  // initialize Print Chamber sensor
  Serial.println(F("Init Print Chamber Sensor..."));
  int8_t error = pingSensor(DHT_ADDR);
  if (error == 0) {
    Sensors::initDhtSensor();
  } else {
    exists.dhtDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize UV Light sensor
  Serial.println(F("Init UV Light sensor..."));
  error = pingSensor(SI_ADDR);
  if (error == 0) {
    Sensors::initSiSensor();
  } else {
    exists.siDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Non-Contact temperature sensor
  Serial.println(F("Init Non-Contact temperature sensor..."));
  error = pingSensor(MLX_ADDR);
  if (error == 0) {
    Sensors::initMlxSensor();
  } else {
    exists.mlxDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Ambient temperature sensor
  Serial.println(F("Init BME280 sensor..."));
  error = pingSensor(BME_ADDR_A);
  if (error == 0) {
    bme_i2c_address = (BME_ADDR_A);
    Sensors::initBmeSensor();
  } else {
    error = pingSensor(BME_ADDR_B);
    if (error == 0) {
      bme_i2c_address = (BME_ADDR_B);
      Sensors::initBmeSensor();
    } else {
      exists.bmeDetected = 0;
    }
  }

  Serial.println();
  Serial.println();

  // initialize DS3231 RTC
  Serial.println(F("Init DS3231 RTC..."));
  error = pingSensor(RTC_ADDR);
  if (error == 0) {
    Serial.println(F("OK!"));
    initRtcClock();
  } else {
    Serial.println(F("No RTC Found. ESParaSite requires an RTC with EEPROM. "
                     "Launching config portal"));
    ESParaSite::ConfigPortal::doConfigPortal();
  }

  Serial.println();
  Serial.println();

  // initialize AR24C32 EEPROM
  Serial.println(F("Init AT24C32 EEPROM..."));

  for (eeprom_i2c_address = RTC_EEPROM_BASE_ADDR;
       eeprom_i2c_address <= RTC_EEPROM_MAX_ADDR; eeprom_i2c_address++) {
    error = pingSensor(eeprom_i2c_address);
    if (error == 0) {
      Serial.println(F("OK!"));
      Serial.println();
      RtcEeprom::initRtcEeprom();
      return;
    }
    Serial.println();
  }
  Serial.println(F("NO EEPROM FOUND!"));
  Serial.println();
}
