// ESParaSite.cpp

/* ESParasite Data Logger v0.9
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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266Webserver.h>
//#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_Core.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"

// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU.  Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

// D3 on NodeMCU and WeMos.
const byte TRIGGER_PIN = 0;

// Alternate button, if an external button is desired.
// D0 on NodeMCU and WeMos D1 Mini
const byte TRIGGER_PIN2 = 16;

// Onboard LED I/O pin on NodeMCU board
// D4 on NodeMCU and WeMos. Controls the onboard LED.
const int PIN_LED = 2;

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;

// Variables used to service the loop
uint16_t curLoopMillis = 0;
uint16_t prevSensorMillis = 0;
uint16_t prevDhtMillis = 0;
uint16_t prevEepromMillis = 0;
uint16_t prevHistoryMillis = 0;

// The _resource structs used to carry data globally
ESParaSite::ambient ambientResource;
ESParaSite::configData configResource;
ESParaSite::enclosure enclosureResource;
ESParaSite::optics opticsResource;
ESParaSite::printchamber chamberResource;
ESParaSite::rtcEepromData rtcEepromResource;
ESParaSite::statusData statusResource;
ESParaSite::sensorExists existsResource;

//*************************************************************************
// LOOP
//*************************************************************************
void loop(void) {

#ifdef DEBUG_L1
  Serial.println();
  Serial.print(F("Beginning Loop"));
  Serial.println();
  Serial.println();
#endif

  // Check to see if Config Mode Triggered
  if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW) ||
      (initialConfig)) {
    Serial.println("Configuration portal requested.");
    ESParaSite::ConfigPortal::doConfigPortal();
  }

#ifdef DEBUG_L1
  Serial.println(F("updating mDNS"));
  Serial.println();
#endif

  // Refresh mDNS
  MDNS.update();

#ifdef DEBUG_L1
  Serial.println(F("servicing HTTP Client"));
  Serial.println();
#endif

  // Run the HTTP Sever
    ESParaSite::HttpCore::serveHttpClient();

#ifdef DEBUG_L1
  Serial.println(F("refreshing curLoopMillis"));
  Serial.println();
#endif

  // Refresh the Current Loop Millisecond value
  curLoopMillis = millis();

#ifdef DEBUG_L3
  Serial.print(F("curLoopMillis:\t"));
  Serial.println(curLoopMillis);
  Serial.print(F("prevSensorMillis:\t"));
  Serial.println(prevSensorMillis);
  Serial.print(F("prevDhtMillis:\t"));
  Serial.println(prevDhtMillis);
  Serial.print(F("prevEepromMillis:\t"));
  Serial.println(prevEepromMillis);
  Serial.println();
#endif

#ifdef DEBUG_L1
  Serial.println(F("checking to see if we need to do anything"));
  Serial.println();
  Serial.println(F("Sensors"));
  Serial.println();
#endif

  // Read sensors if it has been the right interval
  prevSensorMillis =
      ESParaSite::Core::doReadSensors(curLoopMillis, prevSensorMillis);

#ifdef DEBUG_L1
  Serial.println(F("DHT12 Sensor"));
  Serial.println();
#endif

  // Read the DHT12 sensor if it has been the right interval
  prevDhtMillis = ESParaSite::Core::doReadDht(curLoopMillis, prevDhtMillis);

#ifdef DEBUG_L1
  Serial.println(F("Write to EEPROM"));
  Serial.println();
#endif

  // Update EEPROM and memory values
  prevEepromMillis =
      ESParaSite::Core::doHandleEeprom(curLoopMillis, prevEepromMillis);

  // Keep track of historical data and update Web UI data
  prevHistoryMillis =
      ESParaSite::Core::doHandleHistory(curLoopMillis, prevHistoryMillis);

#ifdef DEBUG_L1
  Serial.println(F("End of loop waiting 500msec"));
  delay(500);
#endif
}

//*************************************************************************
// SETUP
//*************************************************************************
void setup(void) {
  // Set up our pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);

  // Begin Serial Output
  Serial.begin(115200);
  Serial.println();

  digitalWrite(PIN_LED, LOW);

  Serial.println();
  Serial.println(F("ESParaSite Data Logging Server"));
  Serial.println(
      F("https://github.com/Photonsters/ESParaSite_TemperatureLogger"));
  Serial.print(F("Compiled: "));
  Serial.print(F(__DATE__));
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();

#ifdef DEBUG_L2
  Serial.println(F("Dumping Wifi Diagnostics"));
  Serial.println();
  WiFi.printDiag(Serial); // Remove this line if you do not want to see WiFi
                          // password printed
#endif

#ifdef DEBUG_L1
  Serial.println(F("Mounting LittleFS"));
  Serial.println();
  delay(500);
#endif

#ifdef DEBUG_L1
  Serial.println(F("Mounting FS..."));
#endif

  // Mount the LittleFS Filesystem
  if (!LittleFS.begin()) {
    Serial.println(F("Failed to mount LittleFS file system. Booting in AP "
                     "Config Portal mode."));
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("File system mounted"));
#endif
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
    ESParaSite::ConfigPortal::doConfigPortal();
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
        F("WiFi credentials unset, Booting in AP Config Portal mode."));
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    // Force to station mode because if device was switched off while in
    // access point mode it will start up next time in access point mode.
    WiFi.mode(WIFI_STA);

    // Connect to Access Point
    Serial.println(F("Connecting to Wifi..."));
    Serial.println();
    WiFi.reconnect();

#ifndef DEBUG_L1
    WiFi.waitForConnectResult();
#endif

#ifdef DEBUG_L1
    unsigned long startedAt = millis();
    Serial.print(F("After waiting "));
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    Serial.print(waited / 1000);
    Serial.print(F(" secs in setup() connection result is "));
    Serial.println(connRes);
#endif
  }

#ifdef DEBUG_L1
  Serial.println(F("Wifi Showing IP"));
  Serial.println();
#endif

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect, Booting in AP Config Portal mode."));
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    Serial.println(F("Wifi Connected"));
    Serial.println();
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
    Serial.println();
  }

#ifdef DEBUG_L1
  Serial.println(F("mDNS Config"));
  Serial.println();
#endif

  delay(500);

  if (configResource.cfgMdnsEnabled == 1) {
    const char *mdns_n = configResource.cfgMdnsName;
    if (!MDNS.begin(mdns_n)) {
      // Start the mDNS responder <cfgMdnsName>.local
      Serial.println(F("Error setting up MDNS responder!"));
      ESParaSite::ConfigPortal::doConfigPortal();
    } else {
      Serial.println(F("mDNS responder started"));
      Serial.println();
    }
  }

  Serial.println(F("Starting Webserver"));
  Serial.println();

#ifdef DEBUG_L1
  delay(1000);
#endif

  // Start http server
  ESParaSite::HttpCore::configHttpServerRouting();
  ESParaSite::HttpCore::startHttpServer();

  Serial.println();
  Serial.println(F("HTTP server started"));
  Serial.println();

#ifdef DEBUG_L1
  Serial.println(F("Initialize i2c bus, sensors, rtc and eeprom"));
  Serial.println();
  delay(500);
#endif

  // Initialize i2c bus, sensors, rtc and eeprom
  ESParaSite::Sensors::initI2cSensors();

  // Dump all sensor data to serial console
  ESParaSite::Sensors::dumpSensor(false);

  // Find the most recent EEPROM segment and populate our eeprom_data struct.
  uint16_t mru_segment_addr = ESParaSite::RtcEeprom::doEepromFirstRead();

#ifdef DEBUG_L1
  Serial.print(F("The most recent write address:\t"));
  Serial.println(mru_segment_addr);
  Serial.println();
#endif

  ESParaSite::RtcEeprom::doEepromRead(mru_segment_addr);

  Serial.println(F("Startup Complete!"));
  Serial.println();

#ifdef DEBUG_L1
  Serial.println(F("Waiting 3 Seconds..."));
  delay(3000);
#endif

  // Turn led off as we are finished booting.
  digitalWrite(PIN_LED, HIGH);
}
