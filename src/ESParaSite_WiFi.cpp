
#include <ESP8266WiFi.h>

#include "ESParaSite_Core.h"

extern config_data config_resource;

const char *ap_ssid = "ESParaSite";      // The name of the Wi-Fi network that will be created
const char *ap_password = "thisbugsme!"; // The password required to connect to it, leave blank for an open network

void do_wifi_ap()
{
    WiFi.softAP(ap_ssid, ap_password); // Start the access point
    Serial.print("Access Point \"");
    Serial.print(ap_ssid);
    Serial.println("\" started");

    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP()); // Send the IP address of the ESP8266 to the computer
}

int init_wifi()
{
    // Connect to WiFi network
    WiFi.begin(config_resource.cfg_wifi_ssid, config_resource.cfg_wifi_password);
    Serial.print("\n\r \n\rConnecting to Wifi");

    // Wait for connection
    unsigned long startedAt = millis();
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
    return WiFi.status(); // return the WiFi connection status
}