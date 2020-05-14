// ESParaSite_HttpHandler.cpp

/* ESParasite Data Logger v0.6
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

#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_HttpHandler.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <WiFiClient.h>

extern ESP8266WebServer http_server;

extern ESParaSite::printchamber chamber_resource;
extern ESParaSite::optics optics_resource;
extern ESParaSite::ambient ambient_resource;
extern ESParaSite::enclosure enclosure_resource;
extern ESParaSite::status_data status_resource;
extern ESParaSite::config_data config_resource;
extern ESParaSite::rtc_eeprom_data rtc_eeprom_resource;

void ESParaSite::HttpHandler::handleRoot() {
  http_server.send(
      200, "text/html",
      "<p>Please Browse to:</p><a "
      "href=\"/printchamber\">Printchamber</a></br><a "
      "href=\"/optics\">Optics</a></br><a href=\"/ambient\">Ambient</a></br><a "
      "href=\"/enclosure\">Enclosure</a></br><a href=\"/config\">Config</a>");
  // Send HTTP status 200 (Ok) and send some
  // text to the browser/client
}

void ESParaSite::HttpHandler::handleNotFound() {
  http_server.send(
      404, "text/plain",
      "404: Not found"); // Send HTTP status 404 (Not Found) when there's no
                         // handler for the URI in the request
}

void ESParaSite::HttpHandler::get_chamber() {
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
  http_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::get_optics() {
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
  http_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::get_ambient() {
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
  http_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::get_enclosure() {
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
  http_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void ESParaSite::HttpHandler::get_config() {
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
  http_server.send(200, "application/json", output);
}
