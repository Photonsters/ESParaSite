// ESParaSite_Core.cpp

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
#include <ESP8266WiFi.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_Core.h"
#include "ESParaSite_DataDigest.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"

// +++ Advanced Settings +++
// VISIBLE_THRESHOLD adjusts the sensitivity of the SI1145 Sensor to Ambient
// light when used to detect print activity.
// Default is (280).
#define VISIBLE_THRESHOLD (280.00)

// *** It IS STRONGLY RECOMMENDED THAT YOU DO NOT MODIFY THE BELOW VALUES ***

// ALL_SENSOR_POLLING_SEC determines how often the sensor array should be
// polled for new data.

// Increasing the polling interval will reduce the performance requirement,
// however it will decrease the resolution of the lifetime counter.
#define ALL_SENSOR_POLLING_SEC (1)

// Since the DHT12 has a 0.5Hz Sample Rate, the minimum DHT_SENSOR_POLLING_SEC
// is (2) by default we use (3).
#define DHT_SENSOR_POLLING_SEC (3)

// EEPROM_WRITE_INTERVAL_SEC determines how often we update the EEPROM that
// contains the various lifetime counters.
// The value of 30s gives us over 13 years of EEPROM life.

// Reducing this value will burn out the EEPROM chip faster. Increasing it
// will increase the EEPROM life, but at the cost of a higher margin of
// error for the lifetime counters.

// The margin of error for the DEFAULT 30s Interval is ~2.5hours/year or
// roughly 0.02%.
#define EEPROM_WRITE_INTERVAL_SEC (30)

// *** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU.  Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

// D3 on NodeMCU and WeMos.
const int TRIGGER_PIN = 0;

// Alternate button, if an external button is desired.
// D0 on NodeMCU and WeMos.
const int TRIGGER_PIN2 = 16;

uint8_t isPrinting_counter = 0;

const uint16_t sensors_read_msec = (ALL_SENSOR_POLLING_SEC * 1000);
const uint16_t dht_read_msec = (DHT_SENSOR_POLLING_SEC * 1000);
const uint16_t eeprom_write_msec = (EEPROM_WRITE_INTERVAL_SEC * 1000);
const uint16_t history_msec = 5000;

extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::rtcEepromData rtcEepromResource;
extern ESParaSite::statusData statusResource;

WiFiClient Wifi;

uint16_t ESParaSite::Core::doReadSensors(uint16_t cur_loop_msec,
                                         uint16_t prev_sensor_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_sensor_msec) >=
      sensors_read_msec) {

    //HEARTBEAT X
    Serial.print(".");

#ifdef DEBUG_L1
    Serial.println(F("Reading the sensors"));
    Serial.println();
#endif

    statusResource.rtcCurrentSecond = Sensors::readRtcEpoch();
    enclosureResource.printerLifeSec =
        (statusResource.rtcCurrentSecond - rtcEepromResource.firstOnTimestamp);

#ifdef DEBUG_L2
    Serial.print(F("This Printer has been on for:\t"));
    Serial.print(enclosureResource.printerLifeSec);
    Serial.println(F("  seconds"));
    Serial.println();
#endif

    ESParaSite::Sensors::readBmeSensor();
    ESParaSite::Sensors::readMlxSensor();
    ESParaSite::Sensors::readRtcTemp();
    ESParaSite::Sensors::readSiSensor();

    doCheckPrinting();

    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT read the sensors"));
    Serial.println();
#endif

    return (prev_sensor_msec);
  }
}

uint16_t ESParaSite::Core::doReadDht(uint16_t cur_loop_msec,
                                     uint16_t prev_dht_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_dht_msec) >= dht_read_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Reading the DHT sensor"));
    Serial.println();
#endif

    ESParaSite::Sensors::readDhtSensor(true);
    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT read the DHT sensor"));
    Serial.println();
#endif

    return (prev_dht_msec);
  }
}

uint16_t ESParaSite::Core::doHandleEeprom(uint16_t cur_loop_msec,
                                          uint16_t prev_eeprom_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_eeprom_msec) >=
      eeprom_write_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Checking printing status before writing EEPROM"));
    Serial.println();
#endif

    statusResource.isPrintingFlag = static_cast<uint8_t>(isPrinting());

#ifdef DEBUG_L1
    Serial.print(F("Printing Status:\t"));
    Serial.println(statusResource.isPrintingFlag);
    Serial.println();
    Serial.println(F("Writing the EEPROM"));
    Serial.println();
#endif

    ESParaSite::RtcEeprom::doEepromWrite();

    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT write to the EEPROM"));
    Serial.println();
#endif

    return (prev_eeprom_msec);
  }
}

uint16_t ESParaSite::Core::doHandleHistory(uint16_t cur_loop_msec,
                                           uint16_t prev_history_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_history_msec) >=
      history_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Processing the History"));
    Serial.println();
#endif

    ESParaSite::DataDigest::fillRow();
    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT process the history"));
    Serial.println();
#endif

    return (prev_history_msec);
  }
}

void doCheckPrinting() {
  if (opticsResource.ledVisible >= VISIBLE_THRESHOLD) {

#ifdef DEBUG_L1
    Serial.println(F("Incrementing isPrinting_counter"));
    Serial.println();
#endif

    isPrinting_counter++;
  } else {

#ifdef DEBUG_L1
    Serial.println(F("Not incrementing isPrinting_counter"));
    Serial.println();
#endif
  }
}

bool isPrinting() {
  // if 50% or more of our poll intervals detect light we will set our
  // flag for the full write Interval.
  if (isPrinting_counter >=
      (static_cast<uint8_t>(
          (EEPROM_WRITE_INTERVAL_SEC / ALL_SENSOR_POLLING_SEC) / 2))) {
    isPrinting_counter = 0;
    rtcEepromResource.eepromScreenLifeSec += EEPROM_WRITE_INTERVAL_SEC;
    rtcEepromResource.eepromLedLifeSec += EEPROM_WRITE_INTERVAL_SEC;
    rtcEepromResource.eepromVatLifeSec += EEPROM_WRITE_INTERVAL_SEC;

#ifdef DEBUG_L1
    Serial.println(F("Currently printing during this interval"));
    Serial.println();
#endif

    return 1;
  } else {
    isPrinting_counter = 0;

#ifdef DEBUG_L1
    Serial.println(F("Not printing during this interval"));
    Serial.println();
#endif

    return 0;
  }
}
