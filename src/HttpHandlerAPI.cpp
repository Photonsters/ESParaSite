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

#include "API.h"
#include "FileCore.h"
#include "HTTP.h"

// This code is currently redundant but I would prefer to use this function to
// handle the routing.
void ESParaSite::HTTPHandler::handleAPI(AsyncWebServerRequest *request) {
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
    // We need to figure out how to do this since we must chunk response.
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
    String timestamp = request->getParam("setRtcClock")->value();

    if (ESParaSite::APIHandler::handleSetClock(timestamp)) {
      doc["response"] = "Real Time Clock was set to current time was reset";
    } else {
      doc["response"] = "Real Time Clock was not reset";
    }
  } else if (request->hasParam("setFirstOnTime")) {
    /// Need to build this function
  } else {
    request->send(404);
  }
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}
