// ESParaSite_HttpHandler.cpp

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

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_DataToJson.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_HttpFile.h"
#include "ESParaSite_HttpHandler.h"

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
// if the client requests the upload page
// send it if it exists
// otherwise, respond with a basic HTML form
void ESParaSite::HttpHandler::getHtmlUpload() {

  if (!ESParaSite::HttpFile::handleFileRead("/upload.html")) {
    server.send(200, "text/html",
                "<form method=\"post\" enctype=\"multipart/form-data\">"
                "<input type=\"file\" name=\"name\">"
                "<input class=\"button\" type=\"submit\" value=\"Upload\">"
                "</form>");
  }
}

void ESParaSite::HttpHandler::handleResetScreen() {
  server.send(200, "text/html", "Success!");
  Serial.print(F("Resetting LCD Screen Counter"));
  rtcEepromResource.eepromScreenLifeSec = 0;
}

void ESParaSite::HttpHandler::handleResetFep() {
  server.send(200, "text/html", "Success!");
  Serial.print(F("Resetting FEP Counter"));
  rtcEepromResource.eepromVatLifeSec = 0;
}

void ESParaSite::HttpHandler::handleResetLed() {
  server.send(200, "text/html", "Success!");
  Serial.print(F("Resetting LED Counter"));
  rtcEepromResource.eepromLedLifeSec = 0;
}

void ESParaSite::HttpHandler::getResetScreen() {
  server.send(200, "text/html",
              "<font size=\"+3\">WARNING - This will reset the lifetime"
              "counter of your LCD Screen to 0!!!</font><br>"
              "<form method=\"post\">"
              "<input type=\"submit\" name=\"name\" "
              "value=\"Reset\">"
              "</form>"
              "Please do not immediately turn off your printer. This change"
              "may take up to 30 seconds to be saved.<br>");
}

void ESParaSite::HttpHandler::getResetFep() {
  server.send(200, "text/html",
              "<font size=\"+3\">WARNING - This will reset the lifetime"
              "counter of your Vat FEP to 0!!!</font><br>"
              "<form method=\"post\">"
              "<input type=\"submit\" name=\"name\" "
              "value=\"Reset\">"
              "</form>"
              "Please do not immediately turn off your printer. This change"
              "may take up to 30 seconds to be saved.<br>");
}

void ESParaSite::HttpHandler::getResetLed() {
  server.send(200, "text/html",
              "<font size=\"+3\">WARNING - This will reset the lifetime"
              "counter of your LED Array to 0!!!</font><br>"
              "<form method=\"post\">"
              "<input type=\"submit\" name=\"name\" "
              "value=\"Reset\">"
              "</form>"
              "Please do not immediately turn off your printer. This change"
              "may take up to 30 seconds to be saved.<br>");
}

void ESParaSite::HttpHandler::handleHistory() {

  ESParaSite::DataToJson::historyToJson();
}

void ESParaSite::HttpHandler::handleGuiData() {
  String message = ("Feed not Found.");

  for (int i = 0; i < server.args(); i++) {

    if (server.argName(i) == "readHistory") {
      ESParaSite::DataToJson::historyToJson();
    } else if (server.argName(i) == "status") {
      ESParaSite::DataToJson::statusToJson();
    }
  }
  server.send(200, "text/plain", message); // Response to the HTTP request
}