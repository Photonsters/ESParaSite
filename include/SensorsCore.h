// SensorsCore.h

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

#ifndef INCLUDE_SENSORSCORE_H_
#define INCLUDE_SENSORSCORE_H_

namespace ESParaSite {
namespace Sensors {
// Core Functions
void initI2cSensors();
int pingSensor(uint16_t);
void dumpSensor(bool);
// Chamber Sensor Functions
void initDhtSensor();
void readDhtSensor(bool);
// UV Light Sensor Functions
void initSiSensor();
void readSiSensor();
// IR Temp Sensor Functions
void initMlxSensor();
void readMlxSensor();
// Ambient Sensor Functions
void initBmeSensor();
void readBmeSensor();
// RTC Functions
void initRtcClock();
void readRtcData();
void readRtcTemp();
time_t readRtcEpoch();
void checkRtcStatus();
void setRtcfromEpoch(time_t);

}; // namespace Sensors
}; // namespace ESParaSite

#endif // INCLUDE_SENSORSCORE_H_
