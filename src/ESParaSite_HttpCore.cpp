// ESParaSite_HttpCore.cpp

/* ESParasite Data Logger v0.6
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
#include <ESP8266Webserver.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_HttpFile.h"
#include "ESParaSite_HttpHandler.h"

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
  server.on("/printchamber", HTTP_GET, ESParaSite::HttpHandler::getJsonChamber);
  server.on("/optics", HTTP_GET, ESParaSite::HttpHandler::getJsonOptics);
  server.on("/ambient", HTTP_GET, ESParaSite::HttpHandler::getJsonAmbient);
  server.on("/enclosure", HTTP_GET, ESParaSite::HttpHandler::getJsonEnclosure);
  server.on("/config", HTTP_GET, ESParaSite::HttpHandler::getJsonConfig);
  server.on("/upload", HTTP_GET, ESParaSite::HttpHandler::getHtmlUpload);
  server.on("/reset_screen", HTTP_GET, ESParaSite::HttpHandler::getResetScreen);
  server.on("/reset_fep", HTTP_GET, ESParaSite::HttpHandler::getResetFep);
  server.on("/reset_led", HTTP_GET, ESParaSite::HttpHandler::getResetLed);
  // if the client posts to the upload page
  // Send status 200 (OK) to tell the client we are ready to receive
  // Receive and save the file
  server.on(
      "/upload", HTTP_POST, []() { server.send(200); },
      ESParaSite::HttpFile::handleFileUpload);
  server.on(
      "/reset_screen", HTTP_POST, ESParaSite::HttpHandler::handleResetScreen);
  server.on("/reset_fep", HTTP_POST, ESParaSite::HttpHandler::handleResetFep);
  server.on("/reset_led", HTTP_POST, ESParaSite::HttpHandler::handleResetLed);

  Serial.println("HTTP REST config complete");
}

void ESParaSite::HttpCore::serveHttpClient() { server.handleClient(); }

void ESParaSite::HttpCore::startHttpServer() { server.begin(); }

void ESParaSite::HttpCore::stopHttpServer() { server.stop(); }

/* Code to implement Async Web Server (far future)

AsyncWebServer server(HTTP_REST_PORT);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void ESParaSite::HttpCore::onWsEvent(AsyncWebSocket *server,
                                     AsyncWebSocketClient *client,
                                     AwsEventType type, void *arg,
                                     uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
                  *((uint16_t *)arg), (char *)data);
  } else if (type == WS_EVT_PONG) {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len,
                  (len) ? (char *)data : "");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(),
                    client->id(), (info->opcode == WS_TEXT) ? "text" : "binary",
                    info->len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char)data[i];
        }
      } else {
        char buff[3];
        for (size_t i = 0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      // message is comprised of multiple frames or the frame is split into
      // multiple packets
      if (info->index == 0) {
        if (info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(),
                        client->id(),
                        (info->message_opcode == WS_TEXT) ? "text" : "binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(),
                      client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(),
                    client->id(), info->num,
                    (info->message_opcode == WS_TEXT) ? "text" : "binary",
                    info->index, info->index + len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < len; i++) {
          msg += (char)data[i];
        }
      } else {
        char buff[3];
        for (size_t i = 0; i < len; i++) {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if ((info->index + len) == info->len) {
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(),
                      client->id(), info->num, info->len);
        if (info->final) {
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(),
                        client->id(),
                        (info->message_opcode == WS_TEXT) ? "text" : "binary");
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

void ESParaSite::HttpCore::config_rest_server_routing() {
  ws.onEvent(ESParaSite::HttpCore::onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(),
                  request->url().c_str());

    if (request->contentLength()) {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++) {
      AsyncWebHeader *h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isFile()) {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(),
                      p->value().c_str(), p->size());
      } else if (p->isPost()) {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String &filename,
                         size_t index, uint8_t *data, size_t len, bool final) {
    if (!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char *)data);
    if (final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data,
                          size_t len, size_t index, size_t total) {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char *)data);
    if (index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  });
}

void ESParaSite::HttpCore::start_http_server() { server.begin(); }

void ESParaSite::HttpCore::stop_http_server() { server.end(); }

void ESParaSite::HttpCore::cleanup_http_client() { ws.cleanupClients(); }

*/