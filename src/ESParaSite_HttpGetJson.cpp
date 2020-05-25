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

void ESParaSite::HttpHandler::getJsonChamber() {
  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["chmb_temp_c"] = chamberResource.chamberTempC;
  doc["chmb_humidity"] = chamberResource.chamberHumidity;
  doc["chmb_dewpoint"] = chamberResource.chamberDewPoint;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::getJsonOptics() {
  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["uvindex"] = opticsResource.ledUVIndex;
  doc["visible"] = opticsResource.ledVisible;
  doc["infrared"] = opticsResource.ledInfrared;
  doc["led_temp_c"] = opticsResource.ledTempC;
  doc["screen_temp_c"] = opticsResource.screenTempC;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::getJsonAmbient() {
  StaticJsonDocument<256> doc;

  doc["class"] = "ambient";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["amb_temp_c"] = ambientResource.ambientTempC;
  doc["amb_humidity"] = ambientResource.ambientHumidity;
  doc["amb_pressure"] = ambientResource.ambientBarometer;
  doc["altitude"] = ambientResource.ambientAltitude;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::getJsonEnclosure() {
  StaticJsonDocument<256> doc;

  doc["class"] = "enclosure";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["caseTempC_c"] = enclosureResource.caseTempC;
  doc["lifetime_sec"] = enclosureResource.printerLifeSec;
  doc["screen_sec"] = enclosureResource.lcdLifeSec;
  doc["ledLifeSec"] = enclosureResource.ledLifeSec;
  doc["vatLifeSec"] = enclosureResource.vatLifeSec;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::getJsonConfig() {
  StaticJsonDocument<256> doc;

  doc["class"] = "eeprom";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["first_on_time64"] = rtcEepromResource.firstOnTimestamp;
  doc["last_write_time64"] = rtcEepromResource.lastWriteTimestamp;
  doc["screen_printerLifeSec"] = rtcEepromResource.eepromScreenLifeSec;
  doc["led_printerLifeSec"] = rtcEepromResource.eepromLedLifeSec;
  doc["fep_printerLifeSec"] = rtcEepromResource.eepromVatLifeSec;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  server.send(200, "application/json", output);
}
