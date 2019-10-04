// ESParaSite_Rest.cpp

/* ESParasite Data Logger v0.5
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <FS.h>

#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"
#include "ESParaSite_Eeprom.h"
#include "ESParaSite_Sensors.h"

//+++ User Settings +++

// Enter the port on which you want the HTTP server to run (Default is 80).
// You will access the server at http://<ipaddress>:<port>*/
#define HTTP_REST_PORT 80

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

ESP8266WebServer http_rest_server(HTTP_REST_PORT);

String getContentType(String filename);
bool handleFileRead(String path);
const char *htmlfile = "/index.html";

File fsUploadFile; // a File object to temporarily store the received file

void config_rest_server_routing()
{
  http_rest_server.on("/", HTTP_GET, []() {
    http_rest_server.sendHeader("Location", "/index.html", true);
    http_rest_server.send(302, "text/plain", "");
  });
  http_rest_server.on("/printchamber", HTTP_GET, get_chamber);
  http_rest_server.on("/optics", HTTP_GET, get_optics);
  http_rest_server.on("/ambient", HTTP_GET, get_ambient);
  http_rest_server.on("/enclosure", HTTP_GET, get_enclosure);
  http_rest_server.on("/config", HTTP_GET, get_config);
  http_rest_server.on("/upload", HTTP_GET, []() { // if the client requests the upload page
    if (!do_web_gui("/upload.html"))              // send it if it exists
      http_rest_server.send(200, "text/html", "<form method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form>");
    http_rest_server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  http_rest_server.on("/upload", HTTP_POST,                 // if the client posts to the upload page
                      []() { http_rest_server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
                      handleFileUpload);                    // Receive and save the file

  // http_rest_server.on("/enclosure", HTTP_POST, post_enclosure); //Not yet implemented
  // http_rest_server.on("/enclosure", HTTP_PUT, post_enclosure);  //Not yet implemented
  // If the client requests any URI, send it if it exists, otherwise, respond with a 404 (Not Found) error
  http_rest_server.onNotFound([]() {
    if (!do_web_gui(http_rest_server.uri()))
      http_rest_server.send(404, "text/plain", "404: Not Found");
  });
  Serial.println("HTTP REST config complete!");
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

void get_config()
{
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
  http_rest_server.send(200, "application/json", output);
}

boolean do_web_gui(String path)
{ // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  // If a folder is requested, send the index file
  if (path.endsWith("/"))
    path += "index.html";
  // Get the MIME type
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";

  // If the file exists, either as a compressed archive, or normal
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    // If there's a compressed version available
    if (SPIFFS.exists(pathWithGz))
      // Use the compressed version
      path += ".gz";
    // Open the file
    File file = SPIFFS.open(path, "r");
    // Send it to the client
    http_rest_server.streamFile(file, contentType);
    // Close the file again
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  // If the file doesn't exist, return false
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

void handleFileUpload()
{ // upload a new file to the SPIFFS
  HTTPUpload &upload = http_rest_server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
    {                       // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      http_rest_server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
      http_rest_server.send(303);
    }
    else
    {
      http_rest_server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}
