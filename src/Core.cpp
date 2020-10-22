// Core.cpp

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

#include "Core.h"
#include "DataDigest.h"
#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Eeprom.h"
#include "ESP32.h"
#include "Sensors.h"

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

// HISTORY_DIGEST_INTERVAL_SEC determines how often we update and digest
// the historical data maintained for charting data.
// the function is designed to be triggered every 5 seconds.
// Changing this value is not suggested as it will throw off the intervals.
#define HISTORY_DIGEST_INTERVAL_SEC (5)

int8_t isPrintingCounter = 0;
// will store LED state
int ledState = LED_ON;

extern ESParaSite::enclosureData enclosure;
extern ESParaSite::opticsData optics;
extern ESParaSite::eepromData eeprom;
extern ESParaSite::statusData status;

// WiFiClient Wifi;

uint16_t ESParaSite::Core::doReadSensors(uint16_t curLoopMsec,
                                         uint16_t prevSensorMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevSensorMsec) >=
      (ALL_SENSOR_POLLING_SEC * 1000)) {

    // HEARTBEAT 
    Serial.print(".");

    // swap ledState LED_ON/LED_OFF
    if (ledState == LED_ON) {
      ledState = LED_OFF;
    } else {
      ledState = LED_ON;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LED_BUILTIN, ledState);

#ifdef DEBUG_L4
    Serial.println("Reading the sensors");
    Serial.println();
#endif

    status.rtcCurrentSecond = ESParaSite::Sensors::readRtcEpoch();
    enclosure.printerLifeSec =
        (status.rtcCurrentSecond - eeprom.firstOnTimestamp);

#ifdef DEBUG_L2
    Serial.print("This Printer has been on for:\t");
    Serial.print(enclosure.printerLifeSec);
    Serial.println("  seconds");
    Serial.println();
#endif

    ESParaSite::Sensors::readBmeSensor(false);
    ESParaSite::Sensors::readMlxSensor(false);
    ESParaSite::Sensors::readRtcTemp(false);
    ESParaSite::Sensors::readSiSensor(false);

    doCheckPrinting();

    return millis();
  } else {

#ifdef DEBUG_L4
    Serial.println("We did NOT read the sensors");
    Serial.println();
#endif

    return (prevSensorMsec);
  }
}

uint16_t ESParaSite::Core::doReadDht(uint16_t curLoopMsec,
                                     uint16_t prevDhtMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevDhtMsec) >=
      (DHT_SENSOR_POLLING_SEC * 1000)) {

#ifdef DEBUG_L4
    Serial.println("Reading the DHT sensor");
    Serial.println();
#endif

    ESParaSite::Sensors::readDhtSensor(true, false);
    return millis();
  } else {

#ifdef DEBUG_L4
    Serial.println("We did NOT read the DHT sensor");
    Serial.println();
#endif

    return (prevDhtMsec);
  }
}

uint16_t ESParaSite::Core::doHandleEeprom(uint16_t curLoopMsec,
                                          uint16_t prevEepromMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevEepromMsec) >=
      (EEPROM_WRITE_INTERVAL_SEC * 1000)) {

#ifdef DEBUG_L1
    Serial.println();
    Serial.println("Checking printing status before writing EEPROM");
    Serial.println();
#endif

    status.isPrintingFlag = static_cast<uint8_t>(isPrinting());

#ifdef DEBUG_L1
    Serial.println("==========EEPROM==========");
    Serial.print("Printing Status:\t");
    Serial.println(status.isPrintingFlag);
    Serial.println();
    Serial.println("Writing the EEPROM");
    Serial.println();
#endif

    ESParaSite::Eeprom::doEepromWrite();

    return millis();
  } else {

#ifdef DEBUG_L4
    Serial.println("We did NOT write to the EEPROM");
    Serial.println();
#endif

    return (prevEepromMsec);
  }
}

void doCheckPrinting() {
  if (optics.ledVisible >= VISIBLE_THRESHOLD) {

#ifdef DEBUG_L4
    Serial.println("Incrementing isPrintingCounter");
    Serial.println();
#endif

    isPrintingCounter++;
  } else {

#ifdef DEBUG_L4
    Serial.println("Not incrementing isPrintingCounter");
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

#ifdef DEBUG_L4
    Serial.println("Currently printing during this interval");
    Serial.println();
#endif

    return 1;
  } else {
    isPrintingCounter = 0;

#ifdef DEBUG_L4
    Serial.println("Not printing during this interval");
    Serial.println();
#endif

    return 0;
  }
}

uint16_t ESParaSite::Core::doHandleHistory(uint16_t curLoopMsec,
                                           uint16_t prevHistoryMsec) {
  if (static_cast<uint16_t>(curLoopMsec - prevHistoryMsec) >=
      (HISTORY_DIGEST_INTERVAL_SEC * 1000)) {

#ifdef DEBUG_L4
    Serial.println("Processing the History");
    Serial.println();
#endif

    ESParaSite::DataDigest::fillRow();
    return millis();
  } else {

#ifdef DEBUG_L4
    Serial.println("We did NOT process the history");
    Serial.println();
#endif

    return (prevHistoryMsec);
  }
}