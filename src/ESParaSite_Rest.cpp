//ESParaSite_Rest.cpp

/* ESParasite Data Logger v0.3
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "ESParaSite_Settings.hxx"
#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"

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
  // http_rest_server.on("/enclosure", HTTP_POST, post_enclosure); //Not yet implemented
  // http_rest_server.on("/enclosure", HTTP_PUT, post_enclosure);  //Not yet implemented
}

void get_chamber()
{

  read_dht_sensor();
  read_rtc_data();
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = timestamp;
  doc["seconds_t"] = chamber_resource.dht_temp_c;
  doc["seconds_s"] = chamber_resource.dht_humidity;
  doc["seconds_l"] = chamber_resource.dht_dewpoint;

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

  read_si_sensor();
  read_mlx_sensor();
  read_rtc_data();
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = timestamp;
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

  read_bme_sensor();
  read_rtc_data();
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "ambient";
  doc["timestamp"] = timestamp;
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

  read_rtc_data();
  //  read_at24_data();   //Placeholder - Not yet Implemented
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "enclosure";
  doc["timestamp"] = timestamp;
  doc["case_temp"] = enclosure_resource.case_temp;
  doc["seconds_t"] = enclosure_resource.total_sec;  //Placeholder - Not yet Implemented
  doc["seconds_s"] = enclosure_resource.screen_sec; //Placeholder - Not yet Implemented
  doc["seconds_l"] = enclosure_resource.led_sec;    //Placeholder - Not yet Implemented

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}