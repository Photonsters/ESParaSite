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

#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_DataToJson.h"
#include "ESParaSite_Http.h"

extern ESP8266WebServer server;

extern ESParaSite::printchamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::rtcEepromData rtcEepromResource;

void ESParaSite::HttpHandler::handleRoot() {
  if (!ESParaSite::HttpFile::handleFileRead("/index.html")) {
    server.send(
        200, "text/html",
        "<p>Please Browse to:</p><a "
        "href=\"/printchamber\">Printchamber</a></br><a "
        "href=\"/optics\">Optics</a></br><a "
        "href=\"/ambient\">Ambient</a></br><a "
        "href=\"/enclosure\">Enclosure</a></br><a href=\"/config\">Config</a>");
  }
}

void ESParaSite::HttpHandler::handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
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
                "<input class=\"button\" type=\"submit\" value=\"Upload\">"
                "</form>");
  }
}

void ESParaSite::HttpHandler::handleHistory() {

  ESParaSite::DataToJson::historyToJson();
}

void ESParaSite::HttpHandler::getGuiData() {
  String message = ("Feed not Found.");

  for (int i = 0; i < server.args(); i++) {

    if (server.argName(i) == "rh") {
      ESParaSite::DataToJson::historyToJson();
    } else if (server.argName(i) == "rn") {
      ESParaSite::DataToJson::networkToJson();
    } else if (server.argName(i) == "rs") {
      ESParaSite::DataToJson::statusToJson();
    }
  }
  server.send(200, "text/plain", message); // Response to the HTTP request
}