// ESParaSite_DataToJson.cpp

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
#include <cppQueue.h>

#include "ESParaSite.h"
#include "ESParaSite_DataDigest.h"
#include "ESParaSite_DataToJson.h"

extern Queue fiveSecHistory;
extern Queue thirtySecHistory;
extern Queue threeHSecHistory;

extern ESP8266WebServer server;

void ESParaSite::DataToJson::historyToJson() {
  int position = 0;
  const int capacity =
      28 * JSON_ARRAY_SIZE(1) + 28 * JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(28);
  DynamicJsonDocument doc(capacity);
  
    for (int i = 0; i <= THREEHSECMAXELEMENT; i++) {
      history tempStruct = {0};
      threeHSecHistory.peekIdx(&tempStruct, i);

      if (tempStruct.dataTimestamp != 0) {
        String nest = String(position, DEC);
        JsonObject obj = doc.createNestedObject(nest);
        obj["ts"] = tempStruct.dataTimestamp;
        obj["at"] = tempStruct.ambientTempC;
        obj["ah"] = tempStruct.ambientHumidity;
        obj["ct"] = tempStruct.chamberTempC;
        obj["ch"] = tempStruct.chamberHumidity;
        obj["lt"] = tempStruct.ledTempC;
        obj["st"] = tempStruct.screenTempC;
        obj["lo"] = tempStruct.ledOn;

        position++;
      } else {
        position++;
      }
    }


    for (int i = 0; i <= THIRTYSECMAXELEMENT; i++)
    {
      history tempStruct = {0};
      thirtySecHistory.peekIdx(&tempStruct, i);

      if (tempStruct.dataTimestamp != 0) {
        String nest = String(position, DEC);
        JsonObject obj = doc.createNestedObject(nest);
        obj["ts"] = tempStruct.dataTimestamp;
        obj["at"] = tempStruct.ambientTempC;
        obj["ah"] = tempStruct.ambientHumidity;
        obj["ct"] = tempStruct.chamberTempC;
        obj["ch"] = tempStruct.chamberHumidity;
        obj["lt"] = tempStruct.ledTempC;
        obj["st"] = tempStruct.screenTempC;
        obj["lo"] = tempStruct.ledOn;

        position++;
      } else {
        position++;
      }
  }

  for (int i = 0; i <= FIVESECMAXELEMENT; i++) {
    history tempStruct = {0};
    fiveSecHistory.peekIdx(&tempStruct, i);

    if (tempStruct.dataTimestamp != 0) {
      String nest = String(position, DEC);
      JsonObject obj = doc.createNestedObject(nest);
      obj["ts"] = tempStruct.dataTimestamp;
      obj["at"] = tempStruct.ambientTempC;
      obj["ah"] = tempStruct.ambientHumidity;
      obj["ct"] = tempStruct.chamberTempC;
      obj["ch"] = tempStruct.chamberHumidity;
      obj["lt"] = tempStruct.ledTempC;
      obj["st"] = tempStruct.screenTempC;
      obj["lo"] = tempStruct.ledOn;

      position++;
    } else {
      position++;
    }
}

String output; //= "JSON = ";
serializeJson(doc, output);
server.send(200, "application/json", output);

/*
size_t size = serializeJson(doc, Serial);
Serial.println();
Serial.println(size);
Serial.println();
*/
}
