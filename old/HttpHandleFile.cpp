// HttpFile.cpp

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
/*
#include <Arduino.h>
//#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include "Http.h"

extern ESP8266WebServer server;

File fsUploadFile; // a File object to temporarily store
                   // the received file

// convert the file extension to the MIME type
String ESParaSite::HttpFile::getContentType(String filename) {
  if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".svg")) {
    return "image/svg+xml";
  } else if (filename.endsWith(".tar")) {
    return "application/x-tar";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/zip";
  } else {
    return "text/plain";
  }
}

// send the right file to the client (if it exists)
bool ESParaSite::HttpFile::handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  // If a folder is requested, send the index file
  if (path.endsWith("/")) {
    path += "index.html";
  }
  // Get the MIME type
  String contentType = ESParaSite::HttpFile::getContentType(path);
  String pathWithGz = path + ".gz";
  // If the file exists, either as a compressed archive, or normal
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
    // If there's a compressed version available
    if (LittleFS.exists(pathWithGz)) {
      // Use the compressed verion
      path += ".gz";
    }
    // Open the file
    File file = LittleFS.open(path, "r");
    // Send it to the client
    //size_t sent = 
    server.streamFile(file, contentType);
    // Close the file again
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  } else {
    // If the file doesn't exist, return false
    Serial.println(String("\tFile Not Found: ") + path);
    return false;
  }
}

void ESParaSite::HttpFile::handleFileUpload() { // upload a new file to the
                                                // LittleFS
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    // Open the file for writing in LittleFS
    // (create if it doesn't exist)
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write the received bytes to the file
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {     // If the file was successfully created
      fsUploadFile.close(); // Close the file again
      Serial.print("handleFileUpload Size: ");
      Serial.println(upload.totalSize);
      // Redirect the client to the success page
      server.sendHeader("Location", "/success.html");
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

bool ESParaSite::HttpFile::loadFromLittleFS(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.htm";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else
    dataType = ESParaSite::HttpFile::getContentType(path);
  
  File dataFile = LittleFS.open(path.c_str(), "r");
  if (server.hasArg("download"))
    dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

*/