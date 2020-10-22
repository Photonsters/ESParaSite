// HttpHandler.cpp

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


#include "ESParaSite.h"
#include "API.h"
#include "Sensors.h"
#include "Eeprom.h"

extern ESParaSite::eepromData eeprom;

bool ESParaSite::APIHandler::handleResetLed(String doReset) {
  String isTrue = "true";
  if (doReset == isTrue) {
    Serial.print(F("Resetting LED Lifetime Counter"));
    eeprom.eepromLedLifeSec = 0;
    return true;
  } else {
    return false;
  }
}

bool ESParaSite::APIHandler::handleResetScreen(String doReset) {
  String isTrue = "true";
  if (doReset == isTrue) {
    Serial.print(F("Resetting LCD Screen Lifetime Counter"));
    eeprom.eepromScreenLifeSec = 0;
    return true;
  } else {
    return false;
  }
}

bool ESParaSite::APIHandler::handleResetVat(String doReset) {
  String isTrue = "true";
  if (doReset == isTrue) {
    Serial.print(F("Resetting Vat Lifetime Counter"));
    eeprom.eepromVatLifeSec = 0;
    return true;
  } else {
    return false;
  }
}

bool ESParaSite::APIHandler::handleSetClock(String tString) {
  Serial.println("Setting RTC Timestamp");
  if (tString == "") {
    Serial.println("RTC Timestamp not set");
    return false;
  } else {
    Serial.print("Old Timestamp\t");
    Serial.println(ESParaSite::Sensors::readRtcEpoch());
    
    Serial.print("Set Timestamp\t");
    Serial.println(tString);
    char tChar[(tString.length()) + 1];
    strncpy(tChar, tString.c_str(), tString.length());
    ESParaSite::Sensors::setRtcfromEpoch(atoll(tChar));
    Serial.print("New Timestamp\t");
    Serial.println(ESParaSite::Sensors::readRtcEpoch());
    return true;
 }
}
