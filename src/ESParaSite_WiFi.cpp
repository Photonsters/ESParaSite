
#include <ESP8266WiFi.h>

#include "ESParaSite_Core.h"

extern config_data config_resource;

// Start the access point
void do_wifi_ap()
{
    const char *ap_ssid = "ESParaSite";
    const char *ap_password = "thisbugsme!";

    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("Access Point \"");
    Serial.print(ap_ssid);
    Serial.println("\" started");

    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());
}

int init_wifi()
{
    // Connect to WiFi network
    WiFi.begin(config_resource.cfg_wifi_ssid, config_resource.cfg_wifi_password);
    Serial.print("\n\r \n\rConnecting to Wifi");

    // Wait for connection
    uint16_t startedAt = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        float waited = (millis() - startedAt);
        if (waited >= 15000)
        {
            Serial.print("Wifi Connection Timeout");
            break;
        }
    }
    // return the WiFi connection status
    return WiFi.status();
}
