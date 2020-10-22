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

#include <Wire.h>


#include "ESParaSite.h"
#include "Eeprom.h"
#include "Sensors.h"
#include "ConfigPortal.h"

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


//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***
extern ESParaSite::configData config;
extern ESParaSite::machineData machine;

// Since we use libraries by different authors and not all libraries talk to
// sensors in the same way, initI2cSensors and pingSensor give us a cleaner,
// abstracted format to check for the sensors before calling the
// init_xyz_sensor() methods which are unique to the sensor and library chosen.
// This will allow us to extend sensor support over time in a more elegant
// fashion.
void ESParaSite::Sensors::initI2cSensors(uint8_t sdaPin, uint8_t sclPin) {
  // initialize I2C bus
  Serial.print(F("Init I2C bus..."));
  Wire.begin(sdaPin, sclPin);
  Serial.println(F("\t\t\t\t\tOK!"));
  Serial.println();

  // initialize Print Chamber sensor
  Serial.println(F("Init Print Chamber Sensor..."));
  int8_t error = pingSensor(DHT_ADDR);
  if (error == 0) {
    Sensors::initDhtSensor();
  } else {
    machine.dhtDetected = 0;
  }
  Serial.println();

  // initialize UV Light sensor
  Serial.println(F("Init UV Light sensor..."));
  error = pingSensor(SI_ADDR);
  if (error == 0) {
    Sensors::initSiSensor();
  } else {
    machine.siDetected = 0;
  }
  Serial.println();


  // initialize Non-Contact temperature sensor
  Serial.println(F("Init Non-Contact temperature sensor..."));
  error = pingSensor(MLX_ADDR);
  if (error == 0) {
    Sensors::initMlxSensor();
  } else {
    machine.mlxDetected = 0;
  }
  Serial.println();

  // initialize Ambient temperature sensor
  Serial.println(F("Init BME280 sensor..."));
  error = pingSensor(BME_ADDR_A);
  if (error == 0) {
    machine.bmeI2cAddress = (BME_ADDR_A);
    Sensors::initBmeSensor();
  } else {
    error = pingSensor(BME_ADDR_B);
    if (error == 0) {
      machine.bmeI2cAddress = (BME_ADDR_B);
      Sensors::initBmeSensor();
    } else {
      machine.bmeDetected = 0;
    }
  }
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

}