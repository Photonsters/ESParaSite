// Core.cpp

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
#include <ESP8266WiFi.h>

#include "ESParaSite.h"
#include "ConfigPortal.h"
#include "Core.h"
#include "DataDigest.h"
#include "DebugUtils.h"
#include "FileCore.h"
#include "Http.h"
#include "Eeprom.h"
#include "Sensors.h"
#include "Util.h"

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

uint8_t isPrintingCounter = 0;

const uint16_t sensorsReadMsec = (ALL_SENSOR_POLLING_SEC * 1000);
const uint16_t dhtReadMsec = (DHT_SENSOR_POLLING_SEC * 1000);
const uint16_t eepromWriteMsec = (EEPROM_WRITE_INTERVAL_SEC * 1000);
const uint16_t historyMsec = 5000;

extern ESParaSite::enclosureData enclosure;
extern ESParaSite::opticsData optics;
extern ESParaSite::rtcEepromData eeprom;
extern ESParaSite::statusData status;

WiFiClient Wifi;

uint16_t ESParaSite::Core::doReadSensors(uint16_t curLoopMsec,
                                         uint16_t prevSensorMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevSensorMsec) >=
      sensorsReadMsec) {

    //HEARTBEAT X
    Serial.print(".");

#ifdef DEBUG_L1
    Serial.println(F("Reading the sensors"));
    Serial.println();
#endif

    status.rtcCurrentSecond = Sensors::readRtcEpoch();
    enclosure.printerLifeSec =
        (status.rtcCurrentSecond - eeprom.firstOnTimestamp);

#ifdef DEBUG_L2
    Serial.print(F("This Printer has been on for:\t"));
    Serial.print(enclosure.printerLifeSec);
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

    return (prevSensorMsec);
  }
}

uint16_t ESParaSite::Core::doReadDht(uint16_t curLoopMsec,
                                     uint16_t prevDhtMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevDhtMsec) >= dhtReadMsec) {

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

    return (prevDhtMsec);
  }
}

uint16_t ESParaSite::Core::doHandleEeprom(uint16_t curLoopMsec,
                                          uint16_t prevEepromMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevEepromMsec) >=
      eepromWriteMsec) {

#ifdef DEBUG_L1
    Serial.println(F("Checking printing status before writing EEPROM"));
    Serial.println();
#endif

    status.isPrintingFlag = static_cast<uint8_t>(isPrinting());

#ifdef DEBUG_L1
    Serial.print(F("Printing Status:\t"));
    Serial.println(status.isPrintingFlag);
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

    return (prevEepromMsec);
  }
}

uint16_t ESParaSite::Core::doHandleHistory(uint16_t curLoopMsec,
                                           uint16_t prevHistoryMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevHistoryMsec) >=
      historyMsec) {

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

    return (prevHistoryMsec);
  }
}

void doCheckPrinting() {
  if (optics.ledVisible >= VISIBLE_THRESHOLD) {

#ifdef DEBUG_L1
    Serial.println(F("Incrementing isPrintingCounter"));
    Serial.println();
#endif

    isPrintingCounter++;
  } else {

#ifdef DEBUG_L1
    Serial.println(F("Not incrementing isPrintingCounter"));
    Serial.println();
#endif
  }
}

bool isPrinting() {
  // if 50% or more of our poll intervals detect light we will set our
  // flag for the full write Interval.
  if (isPrintingCounter >=
      (static_cast<uint8_t>(
          (EEPROM_WRITE_INTERVAL_SEC / ALL_SENSOR_POLLING_SEC) / 2))) {
    isPrintingCounter = 0;
    eeprom.eepromScreenLifeSec += EEPROM_WRITE_INTERVAL_SEC;
    eeprom.eepromLedLifeSec += EEPROM_WRITE_INTERVAL_SEC;
    eeprom.eepromVatLifeSec += EEPROM_WRITE_INTERVAL_SEC;

#ifdef DEBUG_L1
    Serial.println(F("Currently printing during this interval"));
    Serial.println();
#endif

    return 1;
  } else {
    isPrintingCounter = 0;

#ifdef DEBUG_L1
    Serial.println(F("Not printing during this interval"));
    Serial.println();
#endif

    return 0;
  }
}
