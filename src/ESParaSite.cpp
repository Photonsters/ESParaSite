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
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManager.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_Core.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"
#include "ESParaSite_WiFiCore.h"

// Dumping too much data to Serial seems to be problematic so we will add a
// flag to allow us to toggle verbose text and delays (not fully implemented)

//#define DEBUG_L1
//#define DEBUG_L2

// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU.  Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

// D3 on NodeMCU and WeMos.
const byte TRIGGER_PIN = 0;

// Alternate button, if an external button is desired.
// D7 on NodeMCU and WeMos.
const byte TRIGGER_PIN2 = 13;

// Onboard LED I/O pin on NodeMCU board
// D4 on NodeMCU and WeMos. Controls the onboard LED.
const int PIN_LED = 2;

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;

// Variables used to service the loop
uint16_t cur_loop_msec = 0;
uint16_t prev_sensor_msec = 0;
uint16_t prev_dht_msec = 0;
uint16_t prev_eeprom_msec = 0;

// Variables used to handle interrupts
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;

// The _resource structs used to carry data globally
ESParaSite::ambient ambient_resource;
ESParaSite::config_data config_resource;
ESParaSite::enclosure enclosure_resource;
ESParaSite::optics optics_resource;
ESParaSite::printchamber chamber_resource;
ESParaSite::rtc_eeprom_data rtc_eeprom_resource;
ESParaSite::status_data status_resource;
ESParaSite::sensor_exists exists_resource;

void loop(void) {

#ifdef DEBUG_L1
  Serial.println();
  Serial.print(F("Beginning Loop"));
  Serial.println();
  Serial.println();
#endif

  if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW) ||
      (initialConfig)) {
    Serial.println("Configuration portal requested.");
    ESParaSite::ConfigPortal::do_config_portal();
  }

#ifdef DEBUG_L1
  Serial.println(F("updating mDNS"));
  Serial.println();
#endif

  MDNS.update();

#ifdef DEBUG_L1
  Serial.println(F("servicing HTTP Client"));
  Serial.println();
#endif

  ESParaSite::HttpCore::serve_http_client();

#ifdef DEBUG_L1
  Serial.println(F("refreshing cur_loop_msec"));
  Serial.println();
#endif

  cur_loop_msec = millis();

#ifdef DEBUG_L1
  Serial.println(cur_loop_msec);
  Serial.println(prev_sensor_msec);
  Serial.println(prev_dht_msec);
  Serial.println(prev_eeprom_msec);
#endif

  prev_sensor_msec =
      ESParaSite::Core::do_read_sensors(cur_loop_msec, prev_sensor_msec);

  prev_dht_msec = ESParaSite::Core::do_read_dht(cur_loop_msec, prev_dht_msec);

  prev_eeprom_msec =
      ESParaSite::Core::do_handle_eeprom(cur_loop_msec, prev_eeprom_msec);

#ifdef DEBUG_L1
  Serial.println(F("End of loop waiting 500msec"));
  delay(500);
#endif
}

// ICACHE_RAM_ATTR void handleInterrupt() { interruptCounter++; }

void setup(void) {
  pinMode(PIN_LED, OUTPUT);

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println();

#ifdef DEBUG_L1
  WiFi.printDiag(Serial); // Remove this line if you do not want to see WiFi
                          // password printed
#endif

  Serial.println();
  Serial.println(F("ESParaSite Data Logging Server"));
  Serial.println(
      F("https://github.com/Photonsters/ESParaSite_TemperatureLogger"));
  Serial.print(F("Compiled: "));
  Serial.print(F(__DATE__));
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();

#ifdef DEBUG_L1
  Serial.println(F("Mounting SPIFFS"));
  Serial.println();
  delay(500);
#endif

  // Mount the SPIFFS Filesystem
  Serial.println(F("Mounting FS..."));
  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount file system"));
    // ESParaSite::ConfigPortal::do_error_portal(1);
  } else {
    Serial.println(F("File system mounted"));
  }

#ifdef DEBUG_L1
  Serial.println(F("Loading config.json"));
  Serial.println();
  delay(500);
#endif

  //  Load the config.json file
  if (!ESParaSite::FileCore::loadConfig()) {
    Serial.println(
        F("Failed to load config. Booting in AP Config Portal mode."));

    ESParaSite::ConfigPortal::do_config_portal();
  } else {
    Serial.println(F("config.json loaded"));
  }

#ifdef DEBUG_L1
  Serial.println(F("Wifi Configuration"));
  Serial.println();
  delay(500);
#endif

  if (WiFi.SSID() == "") {
    Serial.println(
        "We haven't got any access point credentials, so get them now");
    initialConfig = true;
  } else {
    WiFi.reconnect();
    digitalWrite(PIN_LED,
                 HIGH); // Turn led off as we are not in configuration mode.
    // WiFi.mode(WIFI_STA); // Force to station mode because if device was
    // switched
    // off while in access point mode it will start up next
    // time in access point mode.
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    Serial.print(waited / 1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  }

#ifdef DEBUG_L1
  Serial.println(F("Wifi SHowing IP"));
  Serial.println();
  delay(500);
#endif

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("failed to connect, finishing setup anyway");
  } else {
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
  }

#ifdef DEBUG_L1
  Serial.println(F("mDNS Config"));
  Serial.println();
  delay(500);
#endif

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

#ifdef DEBUG_L1
  Serial.println(F("Starting Webserver"));
  Serial.println();
  delay(500);
#endif

  // Start http server
  ESParaSite::HttpCore::config_rest_server_routing();
  ESParaSite::HttpCore::start_http_server();

  Serial.println(F("HTTP server started"));
  Serial.println();

#ifdef DEBUG_L1
  Serial.println(F("Initialize i2c bus, sensors, rtc and eeprom"));
  Serial.println();
  delay(500);
#endif

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

  Serial.println(F("Waiting 3 Seconds..."));
  delay(3000);
}
