// ConfigFile.cpp

/* ESParasite Data Logger
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
#include <LittleFS.h>

#include "ESParaSite.h"
#include "DebugUtils.h"
#include "FileCore.h"
#include "Http.h"

using namespace ESParaSite;

extern configData configResource;

bool FileCore::loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {

    Serial.println(F("Failed to open config file."));
    return false;
  }

  size_t size = configFile.size();

  if (size > 1024) {
    Serial.println(F("Config file size is too large."));
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println(F("Failed to parse config file"));
    return false;
  }

  configResource.cfgPinSda = doc["sda_pin"];
  configResource.cfgPinScl = doc["scl_pin"];
  configResource.cfgMdnsEnabled = doc["mdns_enabled"];
  strncpy(configResource.cfgMdnsName, doc["mdns_name"], 32);
  int len = strlen(configResource.cfgMdnsName);
  if (len > 0 && configResource.cfgMdnsName[len - 1] == '\n') {
    configResource.cfgMdnsName[len - 1] = '\0';
  }

  Serial.print(F("I2C Bus on Pins (SDA,SCL): "));
  Serial.print(configResource.cfgPinSda);
  Serial.print(F(", "));
  Serial.println(configResource.cfgPinScl);

  return true;
}

bool FileCore::saveConfig() {
  StaticJsonDocument<200> doc;
  doc["sda_pin"] = configResource.cfgPinSda;
  doc["scl_pin"] = configResource.cfgPinScl;
  doc["mdns_enabled"] = configResource.cfgMdnsEnabled;
  doc["mdns_name"] = configResource.cfgMdnsName;

  serializeJsonPretty(doc, Serial);
  Serial.println();

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}