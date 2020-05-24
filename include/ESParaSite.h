// ESParaSite.h

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

#include <stdbool.h>

#ifndef INCLUDE_ESPARASITE_H_
#define INCLUDE_ESPARASITE_H_

namespace ESParaSite {

struct printchamber {
  float chamberTempC{};
  float chamberHumidity{};
  float chamberDewPoint{};
};

struct optics {
  float ledUVIndex{};
  float ledVisible{};
  float ledInfrared{};
  float ledTempC{};
  float screenTempC{};
};

struct ambient {
  float ambientTempC{};
  float ambientHumidity{};
  float ambientBarometer{};
  float ambientAltitude{};
};

struct enclosure {
  uint32_t printerLifeSec{};
  uint32_t lcdLifeSec{};
  uint32_t ledLifeSec{};
  uint32_t vatLifeSec{};
  float caseTempC{};
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
  bool rtcDetected{};
  bool siDetected{};
};

}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_H_