// HttpCore.cpp

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
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>

#include "ESParaSite.h"
#include "FileCore.h"
#include "HTTP.h"
#include "API.h"

#ifdef ESP32

#include <SPIFFS.h>
#define FileFS SPIFFS

#else

#include <LittleFS.h>
#define FileFS LittleFS

#endif

//+++ User Settings +++

// Enter the port on which you want the HTTP server to run (Default is 80).
// You will access the server at http://<ipaddress>:<port>
#define HTTP_REST_PORT 80

// *** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

AsyncWebServer server(HTTP_REST_PORT);

void ESParaSite::HttpCore::configHttpServerRouting() {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/index.html");
  });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (FileFS.exists("/index.html")) {
      request->send(FileFS, "/index.html");
    } else {
      request->send(200, "text/html",
                    "<p>Please upload GUI using ESParaSite GUI Uploader!</p>");
    }
  });

  server.on("/api", HTTP_ANY, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(4096);
    String response;
    if (request->hasParam("readAmbient")) {
      // APIHandlerData.cpp
      doc = ESParaSite::APIHandler::getJsonAmbient();
    } else if (request->hasParam("readChamber")) {
      doc = ESParaSite::APIHandler::getJsonChamber();
    } else if (request->hasParam("readCurrent")) {
      doc = ESParaSite::APIHandler::getJsonCurrent();
    } else if (request->hasParam("readEeprom")) {
      doc = ESParaSite::APIHandler::getJsonEeprom();
    } else if (request->hasParam("readEnclosure")) {
      doc = ESParaSite::APIHandler::getJsonEnclosure();
    } else if (request->hasParam("readI2C")) {
      doc = ESParaSite::APIHandler::getJsonI2C();
    } else if (request->hasParam("readNetwork")) {
      doc = ESParaSite::APIHandler::getJsonNetwork();
    } else if (request->hasParam("readOptics")) {
      doc = ESParaSite::APIHandler::getJsonOptics();
    } else if (request->hasParam("readStatus")) {
      doc = ESParaSite::APIHandler::getJsonStatus();
    } else if (request->hasParam("readFSInfo")) {
      // APIHandlerFSInfo.cpp
      doc = ESParaSite::APIHandler::getFSInfo(2);
    } else if (request->hasParam("readGraphData")) {
      // APIHandlerGraphData.cpp
      doc = ESParaSite::APIHandler::getJsonGraphData();
    } else if (request->hasParam("readFSList")) {
      doc = ESParaSite::APIHandler::getFSList(2);
    } else if (request->hasParam("resetLedLife")) {
      // APIHandlerReset.cpp
      String doReset = request->getParam("resetLedLife")->value();

      if (ESParaSite::APIHandler::handleResetLed(doReset)) {
        doc["response"] = "LED Lifetime Counter was reset";
      } else {
        doc["response"] = "LED Lifetime Counter was not reset";
      }

    } else if (request->hasParam("resetScreenLife")) {
      String doReset = request->getParam("resetLedLife")->value();

      if (ESParaSite::APIHandler::handleResetScreen(doReset)) {
        doc["response"] = "LED Lifetime Counter was reset";
      } else {
        doc["response"] = "LED Lifetime Counter was not reset";
      }
    } else if (request->hasParam("resetVatLife")) {
      String doReset = request->getParam("resetVatLife")->value();

      if (ESParaSite::APIHandler::handleResetVat(doReset)) {
        doc["response"] = "Vat Lifetime Counter was reset";
      } else {
        doc["response"] = "LED Lifetime Counter was not reset";
      }
    } else if (request->hasParam("setRtcClock")) {
      String timestamp = request->getParam("timestamp")->value();
      if (ESParaSite::APIHandler::handleSetClock(timestamp)) {
        doc["response"] = "Real Time Clock was set to current time was reset";
      } else {
        doc["response"] = "Real Time Clock was not reset";
      }
    } else if (request->hasParam("setFirstOnTime")) {
      ///Need to build this function
    } else{
      request->send(404);
    }
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on(
      "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {},
      ESParaSite::HTTPHandler::handleUpload);

  Serial.println("HTTP REST config complete");

  server.serveStatic("/", FileFS, "/");
}



void ESParaSite::HttpCore::startHttpServer() { server.begin(); }

void ESParaSite::HTTPHandler::handleUpload(AsyncWebServerRequest *request,
                                           String filename, size_t index,
                                           uint8_t *data, size_t len,
                                           bool final) {
  if (!index) {
    Serial.print("UploadStart: " + filename);
    // open the file on first call and store the file handle in the request
    // object
    filename = "/" + filename;
    request->_tempFile = FileFS.open(filename, "w");
  }
  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final) {
    Serial.print("UploadEnd: " + filename + "," + index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(200, "text/plain", "File Uploaded !");
  }
                                           }