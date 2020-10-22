// ESParaSite.h

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
#include <stdbool.h>

#ifndef INCLUDE_H_
#define INCLUDE_H_

namespace ESParaSite {

struct ambientData {
  float ambientBarometer{};
  float ambientAltitude{};
  float ambientDewPoint{};
  float ambientTempC{};
  float ambientHumidity{};
};

struct chamberData {
  float chamberTempC{}; 
};

struct enclosureData {
  uint32_t printerLifeSec{};
  uint32_t lcdLifeSec{};
  uint32_t ledLifeSec{};
  uint32_t vatLifeSec{};
  float caseTempC{};
};

struct opticsData {
  float ledTempC{};
  float screenTempC{};
  uint16_t ledVisible{};
  uint16_t ledInfrared{};
  uint16_t ledUVIndex{};
};

struct statusData {
  time_t rtcCurrentSecond{};
  bool isPrintingFlag{};
};

struct eepromData {
  time_t firstOnTimestamp{};
  time_t lastWriteTimestamp{};
  uint32_t eepromScreenLifeSec{};
  uint32_t eepromLedLifeSec{};
  uint32_t eepromVatLifeSec{};
  uint16_t lastSegmentAddress{};
};

struct configData {
  char cfgMdnsName[32];
  const char *cfgWifiSsid{};
  const char *cfgWifiPassword{};
  char cfgWifiSsidChar[32];
  char cfgWifiPasswordChar[64];
  int8_t cfgPinSda{};
  int8_t cfgPinScl{};
  bool cfgMdnsEnabled{};
};

struct machineData {
  bool bmeDetected{};
  bool dhtDetected{};
  bool mlxDetected{};
  bool siDetected{};
  uint8_t bmeI2cAddress{};
  uint8_t eepromI2cAddress{};
  uint16_t eepromSizeBytes{};
};

}; // namespace ESParaSite

#endif // INCLUDE_H_