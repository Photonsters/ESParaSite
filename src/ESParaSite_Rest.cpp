//ESParaSite_Rest.cpp

/* ESParasite Data Logger v0.4
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"
#include "ESParaSite_Eeprom.h"
#include "ESParaSite_Sensors.h"

//+++ User Settings +++

//Enter the port on which you want the HTTP server to run (Default is 80).
//You will access the server at http://<ipaddress>:<port>
#define HTTP_REST_PORT 80

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

void config_rest_server_routing()
{
  http_rest_server.on("/", HTTP_GET, []() {
    http_rest_server.send(200, "text/html",
                          "Welcome to the ESParasite REST Web Server");
  });
  http_rest_server.on("/printchamber", HTTP_GET, get_chamber);
  http_rest_server.on("/optics", HTTP_GET, get_optics);
  http_rest_server.on("/ambient", HTTP_GET, get_ambient);
  http_rest_server.on("/enclosure", HTTP_GET, get_enclosure);
  http_rest_server.on("/eeprom", HTTP_GET, get_eeprom);
  // http_rest_server.on("/enclosure", HTTP_POST, post_enclosure); //Not yet implemented
  // http_rest_server.on("/enclosure", HTTP_PUT, post_enclosure);  //Not yet implemented
  Serial.print("HTTP REST config complete!");
}

void do_client()
{
  http_rest_server.handleClient();
}

void start_http_server()
{
  http_rest_server.begin();
}

void get_chamber()
{
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
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_optics()
{
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
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_ambient()
{
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
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_enclosure()
{
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
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_eeprom()
{
  StaticJsonDocument<256> doc;

  doc["class"] = "eeprom";
  doc["timestamp"] = status_resource.rtc_current_second;
  doc["first_on_time64"] = rtc_eeprom_resource.first_on_timestamp;
  doc["last_write_time64"] = rtc_eeprom_resource.last_write_timestamp;
  doc["screen_life_sec"] = rtc_eeprom_resource.screen_life_seconds;
  doc["led_life_sec"] = rtc_eeprom_resource.led_life_seconds;
  doc["fep_life_sec"] = rtc_eeprom_resource.fep_life_seconds;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}