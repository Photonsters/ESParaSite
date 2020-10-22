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

#include <Arduino.h>
#include <EepromAt24C32.h>
#include <RtcDS3231.h>
#include <Time.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Sensors.h"
#include "Util.h"


// DS3231 Real Time Clock Address. Default (0x68)
#define RTC_ADDR (0x68)
// AT24C32 EEPROM Address. Default (0x50 to 0x57)
#define RTC_EEPROM_BASE_ADDR (0x50)
#define RTC_EEPROM_MAX_ADDR (0x57)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::enclosureData enclosure;
extern ESParaSite::machineData machine;

extern RtcDS3231<TwoWire> dev_rtc;

void ESParaSite::Sensors::initRtcClock() {
  dev_rtc.Begin();

  Serial.println();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Util::printDateTime(compiled);
  Serial.println();

  if (!dev_rtc.IsDateTimeValid()) {
    if (dev_rtc.LastError() != 0) {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print(F("RTC communications error = "));
      Serial.println(dev_rtc.LastError());
    } else {
      // Common Cuases:
      //  1) first time you ran and the device wasn't running yet
      //  2) the battery on the device is low or even missing

      Serial.println(F("RTC lost confidence in the DateTime!"));

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      dev_rtc.SetDateTime(compiled);
    }
  }

  if (!dev_rtc.GetIsRunning()) {
    Serial.println(F("RTC was not actively running, starting now"));
    dev_rtc.SetIsRunning(true);
  }

  RtcDateTime now = dev_rtc.GetDateTime();
  if (now < compiled) {
    Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
    dev_rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println(F("RTC is newer than compile time. (this is expected)"));
  } else if (now == compiled) {
    Serial.println(
        F("RTC is the same as compile time! (not expected but all is fine)"));
  }

  Serial.println();

  status.rtcCurrentSecond = (dev_rtc.GetDateTime() + 946684800);

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  dev_rtc.Enable32kHzPin(false);
  dev_rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void ESParaSite::Sensors::readRtcData(bool print) {
  ESParaSite::Sensors::checkRtcStatus();
  // RtcDateTime now = dev_rtc.GetDateTime();
  // Epoch64 Conversion
  status.rtcCurrentSecond = (dev_rtc.GetDateTime() + 946684800);

#ifdef DEBUG_L2
  Serial.println("==========Real Time Clock==========");
  print = true;
#endif

  if (print == true) {

    Util::printDateTime(dev_rtc.GetDateTime());
    Serial.println();
    Serial.print("Epoch64:\t\t\t");
    Serial.println(status.rtcCurrentSecond);
  }
}

void ESParaSite::Sensors::checkRtcStatus() {
  if (!dev_rtc.IsDateTimeValid()) {
    if (dev_rtc.LastError() != 0) {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print(F("RTC communications error = "));
      Serial.println(dev_rtc.LastError());
      Serial.println(F("RTC not responding...Restarting"));
      ESP.restart();
    } else {
      // Common Cuases:
      //  1) the battery on the device is low or even missing and the power
      //  line was disconnected
      Serial.println(F("RTC lost confidence in the DateTime!"));
    }
  }
}

void ESParaSite::Sensors::readRtcTemp(bool print) {
  RtcTemperature temp = (dev_rtc.GetTemperature());
  enclosure.caseTempC = ESParaSite::Util::floatToTwo(temp.AsFloatDegC());

#ifdef DEBUG_L2
  Serial.println("==========Case Temperature=========");
  print = true;
#endif

  if (print == true) {
    Serial.print(F("Case Temperature:\t\t"));
    temp.Print(Serial);
    Serial.print("°C / ");
    Serial.print(Util::convertCtoF(enclosure.caseTempC));
    Serial.println("°F");
  }
}

// The real time clock uses Jan 1, 2000 00:00:00 as a time basis so we need
// to add or subtract 946684800 when converting to/from epoch
time_t ESParaSite::Sensors::readRtcEpoch() {

  ESParaSite::Sensors::checkRtcStatus();
  time_t rtc_return = (dev_rtc.GetDateTime() + 946684800);
  return rtc_return;
}

void ESParaSite::Sensors::setRtcfromEpoch(time_t epoch) {
  ESParaSite::Sensors::checkRtcStatus();
  time_t rtc_set = (epoch - 946684800);
  dev_rtc.SetDateTime(rtc_set);
}