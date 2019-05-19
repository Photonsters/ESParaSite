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
#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"


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

  read_dht_sensor();
  read_rtc_data();
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = rtc_timestamp;
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

  read_si_sensor();
  read_mlx_sensor();
  read_rtc_data();
  //create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = rtc_timestamp;
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
  doc["timestamp"] = rtc_timestamp;
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
  doc["timestamp"] = rtc_timestamp;
  doc["case_temp"] = enclosure_resource.case_temp;
  doc["sec_tot"] = enclosure_resource.total_sec;  //Placeholder - Not yet Implemented
  doc["sec_scr"] = enclosure_resource.screen_sec; //Placeholder - Not yet Implemented
  doc["sec_led"] = enclosure_resource.led_sec;    //Placeholder - Not yet Implemented
  doc["sec_fep"] = enclosure_resource.fep_sec;    //Placeholder - Not yet Implemented

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}