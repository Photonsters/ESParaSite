// ESParaSite.h

/* ESParasite Data Logger v0.9
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

#include <stdbool.h>

#define SPIFFS_FIXED_LOCATION 0x100000

#ifndef INCLUDE_H_
#define INCLUDE_H_

namespace ESParaSite {

struct ambient {
  float ambientTempC{};
  float ambientHumidity{};
  float ambientBarometer{};
  float ambientAltitude{};
};

struct chamber {
  float chamberTempC{};
  float chamberHumidity{};
  float chamberDewPoint{};
};

struct enclosure {
  uint32_t printerLifeSec{};
  uint32_t lcdLifeSec{};
  uint32_t ledLifeSec{};
  uint32_t vatLifeSec{};
  float caseTempC{};
};

struct optics {
  float ledUVIndex{};
  float ledVisible{};
  float ledInfrared{};
  float ledTempC{};
  float screenTempC{};
};

struct statusData {
  time_t rtcCurrentSecond{};
  int isPrintingFlag{};
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
  int cfgPinSda{};
  int cfgPinScl{};
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