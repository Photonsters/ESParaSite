//ESParaSite_ConfigPortal.cpp

/* ESParasite Data Logger v0.5
	Authors: Andy  (SolidSt8Dad)Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>   //Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h> //https://github.com/kentaylor/WiFiManager

#include "ESParaSite_Core.h"
#include "ESParaSite_PortalConfig.h"
#include "ESParaSite_FileConfig.h"

extern config_data config_resource;

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. Controls the onboard LED.

void do_config_portal()
{
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);

    char ap_name[18];
    char hexvalue[2];
    strcpy(ap_name, "ESParaSite");

    itoa(macAddr[3], hexvalue, 16);
    strcat(ap_name, hexvalue);
    itoa(macAddr[4], hexvalue, 16);
    strcat(ap_name, hexvalue);
    itoa(macAddr[5], hexvalue, 16);
    strcat(ap_name, hexvalue);

    // Default configuration values
    config_resource.cfg_pin_sda = 0;
    config_resource.cfg_pin_scl = 4;

    pinMode(PIN_LED, OUTPUT);

    Serial.println("before calling printdiag");
    WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed

    digitalWrite(PIN_LED, HIGH); // Turn led off as we are not in configuration mode.
    // For some unknown reason webserver can only be started once per boot up
    // so webserver can not be used again in the sketch.

    Serial.println("Configuration portal requested");
    digitalWrite(PIN_LED, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

    // Extra parameters to be configured
    // After connecting, parameter.getValue() will get you the configured value
    // Format: <ID> <Placeholder text> <default value> <length> <custom HTML> <label placement>

    // Just a quick hint
    WiFiManagerParameter p_hint("<small>Enter your WiFi credentials above</small>");
    WiFiManagerParameter p_hint2("<small>Enter the SDA and SCL Pins for your ESParaSite</small>");
    WiFiManagerParameter p_hint3("<small>If you have multiple ESParaSites, give each a unique name</small>");

    // I2C SCL and SDA parameters are integers so we need to convert them to char array but
    // no other special considerations

    char convertedValue[3];
    sprintf(convertedValue, "%d", config_resource.cfg_pin_sda);
    WiFiManagerParameter p_pinSda("pinsda", "I2C SDA pin", convertedValue, 3);
    sprintf(convertedValue, "%d", config_resource.cfg_pin_scl);
    WiFiManagerParameter p_pinScl("pinscl", "I2C SCL pin", convertedValue, 3);

    char customhtml[24] = "type=\"checkbox\"";
    strcat(customhtml, " checked");
    WiFiManagerParameter p_mdnsEnabled("mdnsen", "Enable mDNS", "T", 2, customhtml, WFM_LABEL_AFTER);
    WiFiManagerParameter p_mdnsName("mdnsname", "mDNSName", "esparasite", 32);

    // Initialize WiFIManager
    WiFiManager wifiManager;

    //add all parameters here

    wifiManager.addParameter(&p_hint);
    wifiManager.addParameter(&p_pinSda);
    wifiManager.addParameter(&p_pinScl);
    wifiManager.addParameter(&p_hint2);
    wifiManager.addParameter(&p_mdnsEnabled);
    wifiManager.addParameter(&p_mdnsName);

    // Sets timeout in seconds until configuration portal gets turned off.
    // If not specified device will remain in configuration mode until
    // switched off via webserver or device is restarted.
    // wifiManager.setConfigPortalTimeout(600);

    // It starts an access point
    // and goes into a blocking loop awaiting configuration.
    // Once the user leaves the portal with the exit button
    // processing will continue

    if (!wifiManager.startConfigPortal(ap_name, "thisbugsme!"))
    {
        Serial.println("Not connected to WiFi but continuing anyway.");
    }
    else
    {
        // If you get here you have connected to the WiFi
        Serial.println("Connected...yeey :)");
    }

    // Getting posted form values and overriding local variables parameters
    // Config file is written regardless the connection state
    config_resource.cfg_pin_sda = atoi(p_pinSda.getValue());
    config_resource.cfg_pin_scl = atoi(p_pinScl.getValue());

    if (strncmp(p_mdnsEnabled.getValue(), "T", 1) != 0)
    {
        Serial.println("mDNS Disabled");
        config_resource.cfg_mdns_enabled = false;
    }
    else
    {
        config_resource.cfg_mdns_enabled = true;
        //config_resource.cfg_mdns_name = p_mdnsName.getValue();
        char str1[32] = "";
        strcpy(config_resource.cfg_mdns_name, p_mdnsName.getValue());
        //config_resource.cfg_mdns_name = *str1;

        Serial.println("mDNS Enabled");
    }

    if (!saveConfig())
    {
        Serial.println("Failed to save config");
    }
    else
    {
        Serial.println("Config saved");

        digitalWrite(PIN_LED, HIGH); // Turn LED off as we are not in configuration mode.

        ESP.reset(); // We restart the ESP8266 to reload with changes.
    }
}
