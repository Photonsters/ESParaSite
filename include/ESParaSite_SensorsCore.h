// ESParaSite_SensorsCore.h

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

#ifndef INCLUDE_ESPARASITE_SENSORSCORE_H_
#define INCLUDE_ESPARASITE_SENSORSCORE_H_

namespace ESParaSite {
namespace Sensors {
void initDhtSensor();
void initSiSensor();
void initMlxSensor();
void initBmeSensor();
void initRtcClock();
void readDhtSensor(bool);
void readSiSensor();
void readMlxSensor();
void readBmeSensor();
void readRtcData();
void checkRtcStatus();
void readRtcTemp();
time_t readRtcEpoch();
void initI2cSensors();
int pingSensor(uint16_t);
void dumpSensor(bool);
}; // namespace Sensors
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_SENSORSCORE_H_
