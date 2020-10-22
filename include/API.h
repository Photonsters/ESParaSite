// HttpCore.h

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

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <arduino.h>

#ifndef INCLUDE_API_H_
#define INCLUDE_API_H_

namespace ESParaSite {

namespace APIHandler {
    

// APIHandlerData.cpp
DynamicJsonDocument getJsonAmbient();
DynamicJsonDocument getJsonChamber();
DynamicJsonDocument getJsonCurrent();
DynamicJsonDocument getJsonEeprom();
DynamicJsonDocument getJsonEnclosure();
DynamicJsonDocument getJsonHistory();
DynamicJsonDocument getJsonI2C();
DynamicJsonDocument getJsonNetwork();
DynamicJsonDocument getJsonOptics();
DynamicJsonDocument getJsonStatus();
// APIHandlerFSInfo.cpp
DynamicJsonDocument getFSInfo(int8_t);
DynamicJsonDocument getFSList(int8_t);
// APIHandlerGraphData.cpp
DynamicJsonDocument getJsonGraphData();
// APIHandlerReset.cpp
bool handleResetLed(String);
bool handleResetScreen(String);
bool handleResetVat(String);
bool handleSetClock(String);


}; // namespace APIHandler
} // namespace ESParaSite
#endif