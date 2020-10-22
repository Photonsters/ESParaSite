// HttpHandler.cpp

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
/*
#include <ArduinoJson.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>

#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESParaSite.h"
#include "Http.h"

extern ESP8266WebServer server;

void ESParaSite::HttpHandleJson::serializeSendJson(const JsonDocument& doc) {

  //serializeJsonPretty(doc, Serial);
  //Serial.println();

  String output = ""; //"JSON = ";
  serializeJson(doc, output);
  server.send(200, "application/json", output);

  return;
}

void ESParaSite::HttpHandleJson::sendContentLengthUnknown(){
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
}

void ESParaSite::HttpHandleJson::serializeSendJsonPartN(const JsonDocument &doc) {

  // serializeJsonPretty(doc, Serial);
  // Serial.println();

  String output = ""; //"JSON = ";
  serializeJson(doc, output);
  server.sendContent(output);

  return;
}
*/