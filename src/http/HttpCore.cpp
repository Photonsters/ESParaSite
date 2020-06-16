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
#include <ESP8266Webserver.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESParaSite.h"
#include "Http.h"

//+++ User Settings +++

// Enter the port on which you want the HTTP server to run (Default is 80).
// You will access the server at http://<ipaddress>:<port>*/
#define HTTP_REST_PORT 80

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

ESP8266WebServer server;

void ESParaSite::HttpCore::configHttpServerRouting() {
  server.on(
      "/",
      ESParaSite::HttpHandler::handleRoot); // Call the 'handleRoot' function
                                            // when a client requests URI "/"
  server.onNotFound(
      ESParaSite::HttpHandler::handleWebRequests); // When a client requests an
                                                   // unknown URI (i.e.
                                                   // something other than "/"),
                                                   // call function
                                                   // "handleNotFound"
  server.on("/api", HTTP_ANY, ESParaSite::HttpHandler::handleApiRequest);

  // server.on("/reset_screen", HTTP_GET, ESParaSite::HttpHandler::getResetScreen);
  // server.on("/reset_fep", HTTP_GET, ESParaSite::HttpHandler::getResetFep);
  // server.on("/reset_led", HTTP_GET, ESParaSite::HttpHandler::getResetLed);
  // server.on("/reset_screen", HTTP_POST,
  //          ESParaSite::HttpHandler::handleResetScreen);
  // server.on("/reset_fep", HTTP_POST, ESParaSite::HttpHandler::handleResetFep);
  // server.on("/reset_led", HTTP_POST, ESParaSite::HttpHandler::handleResetLed);

  // server.on("/setRtc", HTTP_POST,
  //    ESParaSite::HttpHandler::handleSetClock);

  server.on("/upload", HTTP_GET, ESParaSite::HttpHandler::getHtmlUpload);
  server.on(
      "/upload", HTTP_POST, []() { server.send(200); },
      ESParaSite::HttpFile::handleFileUpload);

  Serial.println("HTTP REST config complete");
}

void ESParaSite::HttpCore::serveHttpClient() { server.handleClient(); }

void ESParaSite::HttpCore::startHttpServer() { server.begin(); }

void ESParaSite::HttpCore::stopHttpServer() { server.stop(); }
