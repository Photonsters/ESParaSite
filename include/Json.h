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

#include <arduino.h>
#include <ArduinoJson.h>

class http_rest_server;

#ifndef INCLUDE_JSON_H_
#define INCLUDE_JSON_H_

namespace ESParaSite {
namespace DataToJson {


void getJsonStatus();
void getJsonAmbient();
void getJsonChamber();
void getJsonConfig();
void getJsonCurrent();
void getJsonEeprom();
void getJsonEnclosure();
void getJsonHistory();
void getJsonI2C();
void getJsonNetwork();
void getJsonOptics();



} // namespace DataToJson
} // namespace ESParaSite

#endif // INCLUDE_JSON_H_
