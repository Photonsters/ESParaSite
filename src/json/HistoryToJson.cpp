// DataToJson.cpp

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
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <cppQueue.h>

#include "ESParaSite.h"
#include "DataDigest.h"
#include "Http.h"
#include "Json.h"

extern ESParaSite::chamberData chamber;
extern ESParaSite::opticsData optics;
extern ESParaSite::ambientData ambient;
extern ESParaSite::enclosureData enclosure;
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::rtcEepromData eeprom;
extern ESParaSite::sensorExists exists;

extern Queue fiveSecHistory;
extern Queue thirtySecHistory;
extern Queue fiveMinHistory;
extern Queue oneHourHistory;

extern ESP8266WebServer server;

void ESParaSite::DataToJson::getJsonHistory() {
  int8_t position = 0;
  DynamicJsonDocument parentDoc(2048);
  DynamicJsonDocument nestedDoc(128);
  ESParaSite::HttpHandleJson::sendContentLengthUnknown();

  for (int8_t i = 0; i <= ONEHOURMAXELEMENT; i++) {

    history tempStruct = {0};
    oneHourHistory.peekIdx(&tempStruct, i);

    if (tempStruct.dataTimestamp != 0) {
      JsonObject nested = nestedDoc.to<JsonObject>();
      nested["ts"] = tempStruct.dataTimestamp;
      nested["at"] = tempStruct.ambientTempC;
      nested["ah"] = tempStruct.ambientHumidity;
      nested["ct"] = tempStruct.chamberTempC;
      nested["ch"] = tempStruct.chamberHumidity;
      nested["lt"] = tempStruct.ledTempC;
      nested["st"] = tempStruct.screenTempC;
      nested["lo"] = tempStruct.ledOn;

      String child;
      serializeJson(nestedDoc, child);
      parentDoc.add(serialized(child));
      position++;
    } else {
      position++;
    }
  }

  ESParaSite::HttpHandleJson::serializeSendJson(parentDoc);
  parentDoc.clear();

  for (int8_t i = 0; i <= FIVEMINMAXELEMENT; i++) {

    history tempStruct = {0};
    fiveMinHistory.peekIdx(&tempStruct, i);

    if (tempStruct.dataTimestamp != 0) {
      JsonObject nested = nestedDoc.to<JsonObject>();
      nested["ts"] = tempStruct.dataTimestamp;
      nested["at"] = tempStruct.ambientTempC;
      nested["ah"] = tempStruct.ambientHumidity;
      nested["ct"] = tempStruct.chamberTempC;
      nested["ch"] = tempStruct.chamberHumidity;
      nested["lt"] = tempStruct.ledTempC;
      nested["st"] = tempStruct.screenTempC;
      nested["lo"] = tempStruct.ledOn;

      String child;
      serializeJson(nestedDoc, child);
      parentDoc.add(serialized(child));
      position++;
    } else {
      position++;
    }
  }

  ESParaSite::HttpHandleJson::serializeSendJsonPartN(parentDoc);
  parentDoc.clear();

  for (int8_t i = 0; i <= THIRTYSECMAXELEMENT; i++) {
    history tempStruct = {0};
    thirtySecHistory.peekIdx(&tempStruct, i);

    if (tempStruct.dataTimestamp != 0) {
      JsonObject nested = nestedDoc.to<JsonObject>();
      nested["ts"] = tempStruct.dataTimestamp;
      nested["at"] = tempStruct.ambientTempC;
      nested["ah"] = tempStruct.ambientHumidity;
      nested["ct"] = tempStruct.chamberTempC;
      nested["ch"] = tempStruct.chamberHumidity;
      nested["lt"] = tempStruct.ledTempC;
      nested["st"] = tempStruct.screenTempC;
      nested["lo"] = tempStruct.ledOn;

      String child;
      serializeJson(nestedDoc, child);
      parentDoc.add(serialized(child));

      position++;
    } else {
      position++;
    }
  }

  ESParaSite::HttpHandleJson::serializeSendJsonPartN(parentDoc);
  parentDoc.clear();

  for (int8_t i = 0; i <= FIVESECMAXELEMENT; i++) {
    history tempStruct = {0};
    fiveSecHistory.peekIdx(&tempStruct, i);

    if (tempStruct.dataTimestamp != 0) {
      JsonObject nested = nestedDoc.to<JsonObject>();
      nested["ts"] = tempStruct.dataTimestamp;
      nested["at"] = tempStruct.ambientTempC;
      nested["ah"] = tempStruct.ambientHumidity;
      nested["ct"] = tempStruct.chamberTempC;
      nested["ch"] = tempStruct.chamberHumidity;
      nested["lt"] = tempStruct.ledTempC;
      nested["st"] = tempStruct.screenTempC;
      nested["lo"] = tempStruct.ledOn;

      String child;
      serializeJson(nestedDoc, child);
      parentDoc.add(serialized(child));

      position++;
    } else {
      position++;
    }
  }

  ESParaSite::HttpHandleJson::serializeSendJsonPartN(parentDoc);

}

