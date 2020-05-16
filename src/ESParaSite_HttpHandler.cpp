// ESParaSite_HttpHandler.cpp

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
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_HttpFile.h"
#include "ESParaSite_HttpHandler.h"

// extern AsyncWebServer server;
extern ESP8266WebServer server;

extern ESParaSite::printchamber chamber_resource;
extern ESParaSite::optics optics_resource;
extern ESParaSite::ambient ambient_resource;
extern ESParaSite::enclosure enclosure_resource;
extern ESParaSite::status_data status_resource;
extern ESParaSite::config_data config_resource;
extern ESParaSite::rtc_eeprom_data rtc_eeprom_resource;

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
  // Send HTTP status 200 (Ok) and send some
  // text to the browser/client
}

void ESParaSite::HttpHandler::handleNotFound() {
  server.send(
      404, "text/plain",
      "404: Not found"); // Send HTTP status 404 (Not Found) when there's no
                         // handler for the URI in the request
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
  rtc_eeprom_resource.screen_life_seconds = 0;
}

void ESParaSite::HttpHandler::handleResetFep() {
  server.send(200, "text/html", "Success!");
  Serial.print(F("Resetting FEP Counter"));
  rtc_eeprom_resource.fep_life_seconds = 0;
}

void ESParaSite::HttpHandler::handleResetLed() {
  server.send(200, "text/html", "Success!");
  Serial.print(F("Resetting LED Counter"));
  rtc_eeprom_resource.led_life_seconds = 0;
}

void ESParaSite::HttpHandler::getJsonChamber() {
  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["chmb_temp_c"] = chamber_resource.dht_temp_c;
  doc["chmb_humidity"] = chamber_resource.dht_humidity;
  doc["chmb_dewpoint"] = chamber_resource.dht_dewpoint;

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
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["uvindex"] = optics_resource.si_uvindex;
  doc["visible"] = optics_resource.si_visible;
  doc["infrared"] = optics_resource.si_infrared;
  doc["led_temp_c"] = optics_resource.mlx_amb_temp_c;
  doc["screen_temp_c"] = optics_resource.mlx_obj_temp_c;

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
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["amb_temp_c"] = ambient_resource.bme_temp_c;
  doc["amb_humidity"] = ambient_resource.bme_humidity;
  doc["amb_pressure"] = ambient_resource.bme_barometer;
  doc["altitude"] = ambient_resource.bme_altitude;

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
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["case_temp_c"] = enclosure_resource.case_temp;
  doc["lifetime_sec"] = enclosure_resource.life_sec;
  doc["screen_sec"] = enclosure_resource.lcd_sec;
  doc["led_sec"] = enclosure_resource.led_sec;
  doc["fep_sec"] = enclosure_resource.fep_sec;

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
  StaticJsonDocument<256> doc2;

  doc["class"] = "eeprom";
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["first_on_time64"] = rtc_eeprom_resource.first_on_timestamp;
  doc["last_write_time64"] = rtc_eeprom_resource.last_write_timestamp;
  doc["screen_life_sec"] = rtc_eeprom_resource.screen_life_seconds;
  doc["led_life_sec"] = rtc_eeprom_resource.led_life_seconds;
  doc["fep_life_sec"] = rtc_eeprom_resource.fep_life_seconds;

  serializeJson(doc, Serial);
  Serial.println();

  serializeJson(doc2, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  serializeJsonPretty(doc2, output);
  server.send(200, "application/json", output);
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
