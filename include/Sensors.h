// Sensors.h

/* ESParaSite-ESP32 Data Logger
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

#ifndef INCLUDE_SENSORSCORE_H_
#define INCLUDE_SENSORSCORE_H_

namespace ESParaSite {
namespace Sensors {
// Core Functions
void initI2cSensors(uint8_t, uint8_t);
int8_t pingSensor(uint16_t);
void dumpSensor(bool);
// Chamber Sensor Functions
void initDhtSensor();
void readDhtSensor(bool, bool);
// UV Light Sensor Functions
void initSiSensor();
void readSiSensor(bool);
// IR Temp Sensor Functions
void initMlxSensor();
void readMlxSensor(bool);
// Ambient Sensor Functions
void initBmeSensor();
void readBmeSensor(bool);
// RTC Functions
void initRtcClock();
void readRtcData(bool);
void readRtcTemp(bool);
time_t readRtcEpoch();
void checkRtcStatus();
void setRtcfromEpoch(time_t);

}; // namespace Sensors
}; // namespace ESParaSite

#endif // INCLUDE_SENSORSCORE_H_
