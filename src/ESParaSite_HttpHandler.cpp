// ESParaSite_HttpHandler.cpp

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

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESParaSite.h"
#include "ESParaSite_Http.h"
#include "ESParaSite_FileCore.h"

extern ESP8266WebServer server;

extern ESParaSite::chamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::rtcEepromData rtcEepromResource;

void ESParaSite::HttpHandler::handleRoot() {
  if (!ESParaSite::HttpFile::handleFileRead("/index.html")) {
    server.send(200, "text/html",
                "<p>Please upload GUI using ESParaSite GUI Uploader!</p>");
  }
}

void ESParaSite::HttpHandler::handleWebRequests() {
  if (ESParaSite::HttpFile::loadFromLittleFS(server.uri()))
    return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message +=
        " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void ESParaSite::HttpHandler::getHtmlUpload() {

  if (!ESParaSite::HttpFile::handleFileRead("/upload.html")) {
    server.send(200, "text/html",
                "<form method=\"post\" enctype=\"multipart/form-data\">"
                "<input type=\"file\" name=\"name\">"
                "<input class=\"button\" type=\"submit\" formmethod=\"post\" "
                "value=\"Upload\">"
                "</form>");
  }
}

void ESParaSite::HttpHandler::getGuiData() {
  String message = ("Data feed not found.");

  for (int i = 0; i < server.args(); i++) {

    if (server.argName(i) == "readAmbient") {
      ESParaSite::DataToJson::getJsonAmbient();
    } else if (server.argName(i) == "readChamber") {
      ESParaSite::DataToJson::getJsonChamber();
    } else if (server.argName(i) == "readEeprom") {
      ESParaSite::DataToJson::getJsonEeprom();
    } else if (server.argName(i) == "readEnclosure") {
      ESParaSite::DataToJson::getJsonEnclosure();
    } else if (server.argName(i) == "readHistory") {
      ESParaSite::DataToJson::getJsonHistory();
    } else if (server.argName(i) == "readI2C") {
      ESParaSite::DataToJson::getJsonI2C();
    } else if (server.argName(i) == "readNetwork") {
      ESParaSite::DataToJson::getJsonNetwork();
    } else if (server.argName(i) == "readOptics") {
      ESParaSite::DataToJson::getJsonOptics();
    } else if (server.argName(i) == "readStatus") {
      ESParaSite::DataToJson::getJsonStatus();
    } else if (server.argName(i) == "readFSInfo"){
      ESParaSite::FileCore::getFSInfo(2);
    } else if (server.argName(i) == "readFSList") {
      ESParaSite::FileCore::getFSInfo(3);
    } else {
      server.send(404, "text/plain", message); // Response to the HTTP request
    }
  }
}