// ConfigFile.cpp

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
// BASED ON -
// Example: storing JSON configuration file in flash file system
//
// Uses ArduinoJson library by Benoit Blanchon.
// https://github.com/bblanchon/ArduinoJson
//
// Created Aug 10, 2015 by Ivan Grokhotkov.
//
// This example code is in the public domain.

#include <ArduinoJson.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "FileCore.h"

#ifdef ESP32

#include <SPIFFS.h>
#define FileFS SPIFFS

#else

#include <LittleFS.h>
#define FileFS LittleFS

#endif

extern ESParaSite::configData config;

bool ESParaSite::FileCore::loadConfig() {
  // Open the config.json file for reading.
  File configFile = FileFS.open("/config.json", "r");
  if (!configFile) {

    Serial.println(F("Failed to open config file."));
    return false;
  }
  // Check to make sure the file will fit into the buffer.
  size_t size = configFile.size();

  if (size > 1024) {
    Serial.println(F("Config file size is too large."));
    return false;
  }
  // Read config.json into buffer
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  //Parse Buffer into JSON doc
  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println(F("Failed to parse config file"));
    return false;
  }


#ifdef DEBUG_L2
  // WARNING!!!: This wil print your WiFi Password in plain text!
  Serial.println("");
  Serial.println("config.json File Contents:");
  serializeJsonPretty(doc, Serial);
  Serial.println("");
  Serial.println("");
#endif

  //Convert JSON values into config variables
  strncpy(config.cfgWifiSsidChar, doc["wifi_ssid"], 32);
  strncpy(config.cfgWifiPasswordChar, doc["wifi_passwd"], 64);
  config.cfgMdnsEnabled = doc["mdns_enabled"];
  strncpy(config.cfgMdnsName, doc["mdns_name"], 32);
  int8_t len = strlen(config.cfgMdnsName);
  if (len > 0 && config.cfgMdnsName[len - 1] == '\n') {
    config.cfgMdnsName[len - 1] = '\0';
  }
  JsonVariant pinSda = doc["sda_pin"];
  JsonVariant pinScl = doc["scl_pin"];
  config.cfgPinSda = pinSda.as<int>();
  config.cfgPinScl = pinScl.as<int>();


  Serial.print(F("I2C Bus on Pins (SDA,SCL): "));
  Serial.print(config.cfgPinSda);
  Serial.print(F(", "));
  Serial.println(config.cfgPinScl);

  return true;
}

bool ESParaSite::FileCore::saveConfig() {

  StaticJsonDocument<200> doc;
  doc["sda_pin"] = config.cfgPinSda;
  doc["scl_pin"] = config.cfgPinScl;
  doc["wifi_ssid"] = config.cfgWifiSsid;
  doc["wifi_passwd"] = config.cfgWifiPassword;
  doc["mdns_enabled"] = config.cfgMdnsEnabled;
  doc["mdns_name"] = config.cfgMdnsName;

  serializeJsonPretty(doc, Serial);
  Serial.println();

  File configFile = FileFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  serializeJson(doc, configFile);

  return true;
}
