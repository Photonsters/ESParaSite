// ESParaSite_WiFiCore.cpp

/* ESParasite Data Logger v0.6
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

#include <ESP8266WiFi.h>

#include "ESParaSite.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_WiFiCore.h"

using namespace ESParaSite;

extern config_data config_resource;

// Start the access point
void Network::do_wifi_ap() {
  const char *ap_ssid = "ESParaSite";
  const char *ap_password = "thisbugsme!";

  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("Access Point \"");
  Serial.print(ap_ssid);
  Serial.println("\" started");

  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
}

int Network::do_init_wifi() {
  // Connect to WiFi network
  WiFi.begin(config_resource.cfg_wifi_ssid, config_resource.cfg_wifi_password);
  Serial.print("\n\r \n\rConnecting to Wifi");

  // Wait for connection
  uint16_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    float waited = (millis() - startedAt);
    if (waited >= 15000) {
      Serial.print("Wifi Connection Timeout");
      break;
    }
  }
  // return the WiFi connection status
  return WiFi.status();
}
