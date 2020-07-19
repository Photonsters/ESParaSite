// ConfigPortal.cpp

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
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiManager.h>

#include "ConfigPortal.h"
#include "DebugUtils.h"
#include "ESParaSite.h"
#include "FileCore.h"
#include "Http.h"

using namespace ESParaSite;

#define PIN_SDA (4)
#define PIN_SCL (5)

// Onboard LED I/O pin on NodeMCU board
// D4 on NodeMCU and WeMos. Controls the onboard LED.
#define PIN_LED (2)

#define PORTAL_PASSWORD ("thisbugsme")

extern configData config;

void ConfigPortal::doConfigPortal() {

  Serial.println("Configuration portal requested");
  Serial.println();

  // Stop existing HTTP server. This is required in order to start a new HTTP
  // server for the captive portal.
  HttpCore::stopHttpServer();

  // We will give our Access Point a unique name based on the last 3 octets 
  // of the MAC Address
  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);

  char ap_name[18];
  snprintf(ap_name, sizeof(ap_name), "%s_%02x%02x%02x\n", "ESParaSite",
           macAddr[3], macAddr[4], macAddr[5]);

  // Default Pin configuration values
  config.cfgPinSda = PIN_SDA;
  config.cfgPinScl = PIN_SCL;

  pinMode(PIN_LED, OUTPUT);

  // Local intialization of WiFiManager.
  WiFiManager wifiManager;

  // reset settings - for testing
  // wifiManager.resetSettings();

  // sets timeout until configuration portal gets turned off in seconds
  wifiManager.setTimeout(120);

  // I2C SCL and SDA parameters are integers so we need to convert them to
  // char array
  char convertedValue[3];
  snprintf(convertedValue, sizeof(convertedValue), "%d",
           config.cfgPinSda);
  WiFiManagerParameter p_pinSda("pinsda", "I2C SDA pin", convertedValue, 3);
  snprintf(convertedValue, sizeof(convertedValue), "%d",
           config.cfgPinScl);
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
  int8_t len = strlen(customhtml);
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

  // if (!wifiManager.startConfigPortal(ap_name, "thisbugsme")) {
    if (!wifiManager.startConfigPortal(ap_name, PORTAL_PASSWORD)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  } else {
    // if you get here you have connected to the WiFi
    Serial.println("Connected...");
  }

  // Getting posted form values and overriding local variables parameters
  // Config file is written regardless the connection state
  config.cfgPinSda = atoi(p_pinSda.getValue());
  config.cfgPinScl = atoi(p_pinScl.getValue());

  if (strncmp(p_mdnsEnabled.getValue(), "T", 1) != 0) {
    Serial.println("mDNS Disabled");
    config.cfgMdnsEnabled = false;
  } else {
    config.cfgMdnsEnabled = true;

    snprintf(config.cfgMdnsName,
             sizeof(config.cfgMdnsName), "%s\n",
             p_mdnsName.getValue());

    Serial.println("mDNS Enabled");
  }

  if (!(FileCore::saveConfig())) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
    Serial.println();

    Serial.println("Resetting ESParaSite");
    Serial.println();
    delay(5000);

    // Turn LED off as we are not in configuration mode.
    digitalWrite(PIN_LED, HIGH);

    // We restart the ESP8266 to reload with changes.
    ESP.restart();
  }
}
