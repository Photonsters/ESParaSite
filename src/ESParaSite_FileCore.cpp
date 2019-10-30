// ESParaSite_ConfigFile.cpp

/* ESParasite Data Logger v0.5
        Authors: Andy (DocMadmag) Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
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
#include <FS.h>

#include "ESParaSite.h"
#include "ESParaSite_FileCore.h"

extern ESParaSite::config_data config_resource;

bool ESParaSite::FileCore::loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file.");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large.");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  config_resource.cfg_wifi_ssid = doc["wifi_ssid"];
  config_resource.cfg_wifi_password = doc["wifi_password"];
  config_resource.cfg_pin_sda = doc["sda_pin"];
  config_resource.cfg_pin_scl = doc["scl_pin"];
  config_resource.cfg_mdns_enabled = doc["mdns_enabled"];
  strncpy(config_resource.cfg_mdns_name, doc["mdns_name"], 32);
  int len = strlen(config_resource.cfg_mdns_name);
  if (len > 0 && config_resource.cfg_mdns_name[len - 1] == '\n') {
    config_resource.cfg_mdns_name[len - 1] = '\0';
  }

  if (strcmp(config_resource.cfg_wifi_ssid, "") == 0) {
    Serial.println("No Wifi config set in config.json");
    Serial.println("");
  } else {
    Serial.println("Wifi Config loaded from config.json");
    Serial.print("SSID: ");
    Serial.println(config_resource.cfg_wifi_ssid);
    Serial.print("PASSWORD: ");
    Serial.println(config_resource.cfg_wifi_password);
    Serial.println("");
  }

  if (config_resource.cfg_mdns_enabled == 1) {
    Serial.println("mDNS enabled on URL:");
    Serial.print("http://");
    Serial.print(config_resource.cfg_mdns_name);
    Serial.println(".local");
    Serial.println("");
  }

  Serial.print("I2C Bus on Pins (SDA,SCL): ");
  Serial.print(config_resource.cfg_pin_sda);
  Serial.print(", ");
  Serial.println(config_resource.cfg_pin_scl);

  return true;
}

bool ESParaSite::FileCore::saveConfig() {
  StaticJsonDocument<200> doc;
  doc["wifi_ssid"] = config_resource.cfg_wifi_ssid;
  doc["wifi_password"] = config_resource.cfg_wifi_password;
  doc["sda_pin"] = config_resource.cfg_pin_sda;
  doc["scl_pin"] = config_resource.cfg_pin_scl;
  doc["mdns_enabled"] = config_resource.cfg_mdns_enabled;
  doc["mdns_name"] = config_resource.cfg_mdns_name;

  serializeJsonPretty(doc, Serial);
  Serial.println();

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}
