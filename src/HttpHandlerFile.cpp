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
#include <ESPAsyncWebServer.h>

#include "HTTP.h"

#ifdef ESP32

#include <SPIFFS.h>
#define FileFS SPIFFS

#else

#include <LittleFS.h>
#define FileFS LittleFS

#endif

// This code is currently redundant but I would prefer to use this function to
// handle the routing.
/*
void ESParaSite::HTTPHandler::handleUpload(AsyncWebServerRequest *request,
                                        String filename, size_t index,
                                        uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.print("UploadStart: " + filename);
    // open the file on first call and store the file handle in the request
    // object
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
  }
  if (final) {
    Serial.print("UploadEnd: " + filename + "," + index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    request->send(200, "text/plain", "File Uploaded !");
  }
}
*/