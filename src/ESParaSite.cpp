// ESParaSite.cpp

/* ESParasite Data Logger v0.5
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_Core.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"
#include "ESParaSite_WiFiCore.h"

// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU.  Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

//#define DEBUG1

// D3 on NodeMCU and WeMos.
const byte TRIGGER_PIN = 0;

// Alternate button, if an external button is desired.
// D0 on NodeMCU and WeMos.
const byte TRIGGER_PIN2 = 16;

// Onboard LED I/O pin on NodeMCU board
// D4 on NodeMCU and WeMos. Controls the onboard LED.
const int PIN_LED = 2;

uint16_t cur_loop_msec = 0;
uint16_t prev_sensor_msec = 0;
uint16_t prev_dht_msec = 0;
uint16_t prev_eeprom_msec = 0;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;

ESParaSite::ambient ambient_resource;
ESParaSite::config_data config_resource;
ESParaSite::enclosure enclosure_resource;
ESParaSite::optics optics_resource;
ESParaSite::printchamber chamber_resource;
ESParaSite::rtc_eeprom_data rtc_eeprom_resource;
ESParaSite::status_data status_resource;
ESParaSite::sensor_exists exists_resource;

void loop(void) {
  Serial.println();
  Serial.print(F("Beginning Loop"));
  Serial.println();
  Serial.println();

  if (interruptCounter > 0) {

    interruptCounter--;
    numberOfInterrupts++;

    Serial.print(F("Configuration Reset Interrupt Triggered. Total: "));
    Serial.println(numberOfInterrupts);

    ESParaSite::Core::do_config_trigger_check();
  }

  Serial.println(F("updating mDNS"));
  Serial.println();
  MDNS.update();

  Serial.println(F("servicing HTTP Client"));
  Serial.println();
  ESParaSite::HttpCore::serve_http_client();

  Serial.println(F("refreshing cur_loop_msec"));
  Serial.println();
  cur_loop_msec = millis();

  Serial.println(cur_loop_msec);
  Serial.println(prev_sensor_msec);
  Serial.println(prev_dht_msec);
  Serial.println(prev_eeprom_msec);

  prev_sensor_msec = ESParaSite::Core::do_read_sensors(cur_loop_msec,
                                                    prev_sensor_msec);

  prev_dht_msec = ESParaSite::Core::do_read_dht(cur_loop_msec, prev_dht_msec);
  
  prev_eeprom_msec = ESParaSite::Core::do_handle_eeprom(cur_loop_msec,
                                                     prev_eeprom_msec);

#ifdef DEBUG1
  Serial.println(F("End of loop waiting 500msec"));
  delay(500);
#endif
}

ICACHE_RAM_ATTR void handleInterrupt() { interruptCounter++; }

void setup(void) {
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), handleInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN2), handleInterrupt, CHANGE);

  Serial.begin(115200);
  Serial.println();

  Serial.println();
  Serial.println(F("ESParaSite Data Logging Server"));
  Serial.println(F("https://github.com/Photonsters/ESParaSite_TemperatureLogger"));
  Serial.print(F("Compiled: "));
  Serial.print(F(__DATE__));
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();

  // Mount the SPIFFS Filesystem
  Serial.println(F("Mounting FS..."));
  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount file system"));
    ESParaSite::ConfigPortal::do_error_portal(1);
  } else {
    Serial.println(F("File system mounted"));
  }

  //  Load the config.json file
  if (!ESParaSite::FileCore::loadConfig()) {
    Serial.println(F("Failed to load config. Booting in AP Config Portal mode."));

    ESParaSite::ConfigPortal::do_config_portal();
  } else {
    Serial.println(F("config.json loaded"));
  }

  config_resource.cfg_pin_sda = 4;
  config_resource.cfg_pin_scl = 5;

  Serial.println(F("Waiting 3 Seconds..."));
  delay(3000);

  // First we check the Wifi Autoconnect Status, if it is not connected, we
  // call do_init_wifi with the credentials saved in config.json if both
  // fail, or anything weird happens we call the config portal
  if (!WiFi.getAutoConnect()) {
    if (ESParaSite::Network::do_init_wifi() == WL_CONNECTED) {
      Serial.println();
      Serial.print(F("Using config.json"));
      Serial.println();
      Serial.print(F("Connected to: "));
      Serial.println(config_resource.cfg_wifi_ssid);
    } else {
      Serial.println(F(
          "Error connecting to WiFi Network - Starting Config Portal"));

      ESParaSite::ConfigPortal::do_config_portal();
    }
  } else if (WiFi.getAutoConnect()) {
    Serial.println();
    Serial.println();
    Serial.print(F("Using Autoconnect"));
    Serial.println();
    Serial.println();
    Serial.print(F("Connected to: "));
    Serial.println(WiFi.SSID());
  } else {
    Serial.println(F("Error connecting to WiFi Network - Starting Config Portal"));

    ESParaSite::ConfigPortal::do_config_portal();
  }

  // Dump IP to Serial Console
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
  Serial.println();

  if (config_resource.cfg_mdns_enabled == 1) {
    const char *mdns_n = config_resource.cfg_mdns_name;
    if (!MDNS.begin(mdns_n)) {
      // Start the mDNS responder <cfg_mdns_name>.local
      Serial.println(F("Error setting up MDNS responder!"));
      ESParaSite::ConfigPortal::do_config_portal();
    } else {
      Serial.println(F("mDNS responder started"));
    }
  }

  // Start http server
  ESParaSite::HttpCore::start_http_server();
  ESParaSite::HttpCore::config_rest_server_routing();
  Serial.println(F("HTTP server started"));
  Serial.println();

  // Initialize i2c bus, sensors, rtc and eeprom
  ESParaSite::Sensors::init_i2c_sensors();

  // Dump all sensor data to serial console
  ESParaSite::Sensors::dump_sensors();

  // Find the most recent EEPROM segment and populate our eeprom_data struct.
  uint16_t mru_segment_addr = ESParaSite::RtcEeprom::do_eeprom_first_read();

  Serial.print(F("The most recent write address:\t"));
  Serial.println(mru_segment_addr);
  Serial.println();

  ESParaSite::RtcEeprom::do_eeprom_read(mru_segment_addr);

  Serial.println(F("Startup Complete!"));
  Serial.println();
}


