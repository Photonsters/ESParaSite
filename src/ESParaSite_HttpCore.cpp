// ESParaSite_HttpCore.cpp

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

#include "ESParaSite_HttpCore.h"
#include "ESP32-targz.h"
#include "ESParaSite.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_HttpHandler.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <WiFiClient.h>


//+++ User Settings +++

// Enter the port on which you want the HTTP server to run (Default is 80).
// You will access the server at http://<ipaddress>:<port>*/
#define HTTP_REST_PORT 80

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

ESP8266WebServer http_server(HTTP_REST_PORT);

extern ESParaSite::printchamber chamber_resource;
extern ESParaSite::optics optics_resource;
extern ESParaSite::ambient ambient_resource;
extern ESParaSite::enclosure enclosure_resource;
extern ESParaSite::status_data status_resource;
extern ESParaSite::config_data config_resource;
extern ESParaSite::rtc_eeprom_data rtc_eeprom_resource;


File fsUploadFile; // a File object to temporarily store the received file

void ESParaSite::HttpCore::config_rest_server_routing() {
  http_server.on(
      "/",
      ESParaSite::HttpHandler::handleRoot); // Call the 'handleRoot' function
                                            // when a client requests URI "/"
  http_server.onNotFound(
      ESParaSite::HttpHandler::handleNotFound); // When a client requests an
                                                // unknown URI (i.e. something
                                                // other than "/"), call
                                                // function "handleNotFound"
  http_server.on("/printchamber", HTTP_GET,
                 ESParaSite::HttpHandler::get_chamber);
  http_server.on("/optics", HTTP_GET, ESParaSite::HttpHandler::get_optics);
  http_server.on("/ambient", HTTP_GET, ESParaSite::HttpHandler::get_ambient);
  http_server.on("/enclosure", HTTP_GET,
                 ESParaSite::HttpHandler::get_enclosure);
  http_server.on("/config", HTTP_GET, ESParaSite::HttpHandler::get_config);
  // if the client posts to the upload page
  // Send status 200 (OK) to tell the client we are ready to receive
  // Receive and save the file
  //http_server.on("/upload", HTTP_POST,[]()
  //{http_server.send(200);},handleFileUpload);
  
  Serial.println("HTTP REST config complete");
}

void ESParaSite::HttpCore::serve_http_client() { http_server.handleClient(); }

void ESParaSite::HttpCore::start_http_server() { http_server.begin(); }

void ESParaSite::HttpCore::stop_http_server() { http_server.stop(); }

String
getContentType(String filename) { // convert the file extension to the MIME type
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
  else if (filename.endsWith(".tar"))
    return "application/x-tar";
  return "text/plain";
}
/*
bool handleFileRead(
    String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html"; // If a folder is requested, send the index file
  String contentType = getContentType(path); // Get the MIME type
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) ||
      LittleFS.exists(path)) {       // If the file exists, either as a
compressed
                                   // archive, or normal
    if (LittleFS.exists(pathWithGz)) // If there's a compressed version
available path += ".gz";               // Use the compressed verion File file =
LittleFS.open(path, "r");                 // Open the file size_t sent
=http_server.streamFile(file, contentType); // Send it to the client
    file.close();                                       // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") +
                 path); // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() { // upload a new file to the LittleFS
  HTTPUpload &upload =http_server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile =
        LittleFS.open(filename, "w"); // Open the file for writing in LittleFS
                                    // (create if it doesn't exist)
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(
          upload.buf,
          upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {     // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
     http_server.sendHeader(
          "Location",
          "/success.html"); // Redirect the client to the success page
     http_server.send(303);
    } else {
     http_server.send(500, "text/plain", "500: couldn't create file");
    }

  }
   */
