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

#include <stdbool.h>

#define SPIFFS_FIXED_LOCATION 0x100000

#ifndef INCLUDE_H_
#define INCLUDE_H_

namespace ESParaSite {

struct ambientData {
  float ambientBarometer{};
  float ambientAltitude{};
  int16_t ambientDewPoint{};
  int16_t ambientTempC{};
  int16_t ambientHumidity{};
};

struct chamberData {
  int16_t chamberTempC{}; 
};

struct enclosureData {
  uint32_t printerLifeSec{};
  uint32_t lcdLifeSec{};
  uint32_t ledLifeSec{};
  uint32_t vatLifeSec{};
  int16_t caseTempC{};
};

struct opticsData {
  int16_t ledTempC{};
  int16_t screenTempC{};
  uint16_t ledVisible{};
  uint16_t ledInfrared{};
  uint16_t ledUVIndex{};
};

struct statusData {
  time_t rtcCurrentSecond{};
  bool isPrintingFlag{};
};

struct rtcEepromData {
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
  int8_t cfgPinSda{};
  int8_t cfgPinScl{};
  bool cfgMdnsEnabled{};
};

struct sensorExists {
  bool bmeDetected{};
  bool dhtDetected{};
  bool mlxDetected{};
  bool siDetected{};
};

}; // namespace ESParaSite

#endif // INCLUDE_H_