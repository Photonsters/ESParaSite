// ESParaSite_ConfigPortal.cpp

/* ESParasite Data Logger v0.6
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <FS.h>

// needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/kentaylor/WiFiManager

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"


extern ESParaSite::config_data config_resource;

// Onboard LED I/O pin on NodeMCU board
// D4 on NodeMCU and WeMos. Controls the onboard LED.
const int PIN_LED = 2;


void ESParaSite::ConfigPortal::do_config_portal() {
  Serial.println(F("Configuration portal requested"));
  Serial.println();

  // Stop existing HTTP server. This is required in order to start a new HTTP
  // server for the captive portal.
  ESParaSite::HttpCore::stop_http_server();

  // We will give our Access Point a unique name based on the last 3 
  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);

  char ap_name[18];
  snprintf(ap_name, sizeof(ap_name), "%s_%02x%02x%02x\n", "ESParaSite",
           macAddr[3], macAddr[4], macAddr[5]);

  // Default configuration values
  config_resource.cfg_pin_sda = 4;
  config_resource.cfg_pin_scl = 5;

  pinMode(PIN_LED, OUTPUT);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it
  // around
  WiFiManager wifiManager;

  // reset settings - for testing
  // wifiManager.resetSettings();

  // sets timeout until configuration portal gets turned off
  // useful to make it all retry or go to sleep
  // in seconds
  wifiManager.setTimeout(120);

  // I2C SCL and SDA parameters are integers so we need to convert them to
  // char array but no other special considerations

  char convertedValue[3];
  snprintf(convertedValue, sizeof(convertedValue), "%d",
           config_resource.cfg_pin_sda);
  WiFiManagerParameter p_pinSda("pinsda", "I2C SDA pin", convertedValue, 3);
  snprintf(convertedValue, sizeof(convertedValue), "%d",
           config_resource.cfg_pin_scl);
  WiFiManagerParameter p_pinScl("pinscl", "I2C SCL pin", convertedValue, 3);

  // Extra parameters to be configured
  // After connecting, parameter.getValue() will get you the configured
  // value.
  // Format:
  // <ID> <Placeholder text> <default value> <length> <custom HTML>
  //  <label placement>

  // Hints for each section
  WiFiManagerParameter p_hint("<small>Enter your WiFi credentials above"
                              "</small>");
  WiFiManagerParameter p_hint2("<small>Enter the SDA and SCL Pins for your"
                               "ESParaSite</small>");
  WiFiManagerParameter p_hint3("</br><small>If you have multiple ESParaSites,"
                               " give each a unique name</small>");
  WiFiManagerParameter p_hint4("<small>Enable mDNS</small>");

  char customhtml[24];
  snprintf(customhtml, sizeof(customhtml), "%s", "type=\"checkbox\"");
  int len = strlen(customhtml);
  snprintf(customhtml + len, (sizeof customhtml) - len, "%s", " checked");
  WiFiManagerParameter p_mdnsEnabled("mdnsen", "Enable mDNS", "T", 2,
                                     customhtml);
  WiFiManagerParameter p_mdnsName("mdnsname", "mDNSName", "esparasite", 32);

  // add all parameters here

  wifiManager.addParameter(&p_hint);
  wifiManager.addParameter(&p_hint2);
  wifiManager.addParameter(&p_pinSda);
  wifiManager.addParameter(&p_pinScl);
  wifiManager.addParameter(&p_hint4);
  wifiManager.addParameter(&p_mdnsEnabled);
  wifiManager.addParameter(&p_hint3);
  wifiManager.addParameter(&p_mdnsName);

  if (!wifiManager.startConfigPortal(ap_name,"thisbugsme")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  } else {
  // if you get here you have connected to the WiFi
  Serial.println(F("Connected..."));
  }

  // Getting posted form values and overriding local variables parameters
  // Config file is written regardless the connection state
  config_resource.cfg_pin_sda = atoi(p_pinSda.getValue());
  config_resource.cfg_pin_scl = atoi(p_pinScl.getValue());

  if (strncmp(p_mdnsEnabled.getValue(), "T", 1) != 0) {
    Serial.println(F("mDNS Disabled"));
    config_resource.cfg_mdns_enabled = false;
  } else {
    config_resource.cfg_mdns_enabled = true;

    snprintf(config_resource.cfg_mdns_name,
             sizeof(config_resource.cfg_mdns_name), "%s\n",
             p_mdnsName.getValue());

    Serial.println(F("mDNS Enabled"));
    }

  if (!(ESParaSite::FileCore::saveConfig())) {
    Serial.println(F("Failed to save config"));
  } else {
    Serial.println(F("Config saved"));
    Serial.println();

    Serial.println(F("Resetting ESParaSite"));
    Serial.println();
    delay(5000);

    // Turn LED off as we are not in configuration mode.
    digitalWrite(PIN_LED, HIGH);

    // We restart the ESP8266 to reload with changes.
    ESP.restart();
  }
}
