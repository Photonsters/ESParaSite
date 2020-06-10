// DataToJson.cpp

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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <cppQueue.h>

#include "ESParaSite.h"
#include "DataDigest.h"
#include "Http.h"

extern ESParaSite::chamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::rtcEepromData rtcEepromResource;
extern ESParaSite::sensorExists existsResource;

extern Queue fiveSecHistory;
extern Queue thirtySecHistory;
extern Queue threeHSecHistory;

extern ESP8266WebServer server;

void ESParaSite::DataToJson::getJsonHistory() {
  int position = 0;
  const int capacity = JSON_ARRAY_SIZE(28) + 28 * JSON_OBJECT_SIZE(8);
  DynamicJsonDocument parentDoc(capacity);
  DynamicJsonDocument nestedDoc(128);

  for (int i = 0; i <= THREEHSECMAXELEMENT; i++) {
    history tempStruct = {0};
    threeHSecHistory.peekIdx(&tempStruct, i);

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

  for (int i = 0; i <= THIRTYSECMAXELEMENT; i++) {
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

  for (int i = 0; i <= FIVESECMAXELEMENT; i++) {
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

  String output; //= "JSON = ";
  serializeJson(parentDoc, output);
  server.send(200, "application/json", output);

}

