// ESParaSite.cpp

/* ESParaSite-ESP32 Data Logger
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

#include <Arduino.h>
#include <FS.h>
#include <I2C_eeprom.h>

#include "API.h"
#include "ConfigPortal.h"
#include "Core.h"
#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Eeprom.h"
#include "FileCore.h"
#include "Http.h"
#include "Sensors.h"
#include "Util.h"


#ifdef ESP32

#include <ESPmDNS.h>
#include <WiFi.h>

#include "ESP32.h"

#else

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "ESP8266.h"

#endif

#ifdef ESP32

#include <SPIFFS.h>
FS *filesystem = &SPIFFS;
#define FileFS SPIFFS

#else

#include <LittleFS.h>
FS *filesystem = &LittleFS;
#define FileFS LittleFS

#endif

// The structs used to carry data globally
ESParaSite::ambientData ambient;
ESParaSite::configData config;
ESParaSite::enclosureData enclosure;
ESParaSite::opticsData optics;
ESParaSite::chamberData chamber;
ESParaSite::eepromData eeprom;
ESParaSite::statusData status;
ESParaSite::machineData machine;

#ifdef ESP32
// Trigger for inititating config mode is Pin D0 and also flash button on
// NodeMCU. Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

// Pin D0 mapped to pin GPIO0/BOOT/ADC11/TOUCH1 of ESP32
const int TRIGGER_PIN = PIN_D0;

// Alternative trigger pin. Needs to be connected to a button to use this pin.
// It must be a momentary connection not connected permanently to ground. Either
// trigger pin will work.

// Pin D25 mapped to pin GPIO25/ADC18/DAC1 of ESP32
const int TRIGGER_PIN2 = PIN_D25;

#else
// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU. Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

const byte TRIGGER_PIN = PIN_D3;

// Alternative trigger pin. Needs to be connected to a button to use this pin.
// It must be a momentary connection not connected permanently to ground. Either
// trigger pin will work.

// Pin D0 mapped to pin GPIO16/USER/WAKE of ESP8266.
const byte TRIGGER_PIN2 = PIN_D0;
#endif

// Variables used to service the loop
uint16_t curLoopMillis = 0;
uint16_t prevSensorMillis = 0;
uint16_t prevDhtMillis = 0;
uint16_t prevEepromMillis = 0;
uint16_t prevHistoryMillis = 0;
uint16_t prevLedMillis = 0;

// will store LED state
// int ledState = LED_ON;

// interval at which to blink (milliseconds)
// const long interval = 1000;

// **********************************************************
// SETUP
// **********************************************************
void setup() {
  // Set up pins
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize trigger pins
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);

  // Begin Serial Output
  Serial.begin(115200);
  Serial.println();

  Serial.println();
  Serial.println("ESParaSite Data Logging Server");
  Serial.println("https://github.com/Photonsters/ESParaSite-ESP32");
  Serial.print("Compiled: ");
  Serial.print(__DATE__);
  Serial.print("\t");
  Serial.println(__TIME__);
  Serial.println();

  // Turn onboard LED on
  digitalWrite(LED_BUILTIN, LED_ON);

  // For Development Only.  FORMAT_FileFS can be uncommented in DebugUtils.h.
#ifdef FORMAT_FileFS
  Serial.println("Formatting SPIFFS/LittleFS...");
  FileFS.format();
#endif

  // Attempt to mount the FileFS Filesystem. If failure assumethat this is
  // first boot and format FileFS Filesystem.

  Serial.println("Mounting FS...");

  if (!FileFS.begin()) {
    Serial.println("Failed to mount FileFS file system. This is likely the "
                   "first boot of this device. Formatting FileFS");
    FileFS.format();
  } else {
    Serial.println("File system mounted");
    Serial.println();
  }

  //#ifdef DEBUG_L2
  // Dump the contents of SPIFFS/LittleFS filesystem to serial console.
  ESParaSite::APIHandler::getFSInfo(1);
  ESParaSite::APIHandler::getFSList(1);
  //#endif

  // Attempt to load the config.json file. If it does not exist, launch Wifi
  // Config Portal. If it exists, parse it.
  if (!ESParaSite::FileCore::loadConfig()) {
    Serial.println("There was a problem with the config.json file. Booting "
                   "in Config Portal mode.");
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    Serial.println("config.json loaded.");
  }

  // Check if Wifi is configured. Failure forces boot into captive portal.
  Serial.println("Configuring Wifi...");
  Serial.println();

  if (strcmp(config.cfgWifiSsidChar, "") == 0) {
    Serial.println(
        F("WiFi credentials unset, Booting in AP Config Portal mode."));
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    // Connect to Access Point
    Serial.println("Connecting to Wifi...");
    WiFi.begin(config.cfgWifiSsidChar, config.cfgWifiPasswordChar);

#ifndef DEBUG_L1
    // Wait for WiFi connection.
    WiFi.waitForConnectResult();
#endif

#ifdef DEBUG_L1
    // Wait for WiFi connection. Time how long it takes to connect.
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int8_t connRes = WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    Serial.print(waited / 1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
#endif
  }

  // Print Wifi Status and IP. If Wifi connection times out, launch config
  // portal.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, Booting in AP Config Portal"
                   "mode.");
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    Serial.print("Wifi Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  }

  // Start mDNS if enabled
  Serial.println("Configuring mDNS...");

  if (config.cfgMdnsEnabled == 1) {
    const char *mdnsN = config.cfgMdnsName;
    if (!MDNS.begin(mdnsN)) {
      // Start the mDNS responder <cfgMdnsName>.local
      Serial.println("Error setting up MDNS responder!");
      ESParaSite::ConfigPortal::doConfigPortal();
    } else {
      Serial.println("mDNS responder started");
      Serial.println("mDNS enabled on URL:");
      Serial.print("http://");
      Serial.print(config.cfgMdnsName);
      Serial.println(".local");
      Serial.println();
    }
  }

  // Initialize I2C bus, sensors and RTC module.
  Serial.println("Initializing i2c bus, sensors and rtc...");
  Serial.println();
  if (!(config.cfgPinSda) || !(config.cfgPinScl)) {
    ESParaSite::Sensors::initI2cSensors(PIN_SDA, PIN_SCL);
  } else {
    ESParaSite::Sensors::initI2cSensors(config.cfgPinSda, config.cfgPinScl);
  }

#ifdef DEBUG_L2
  // Dump all sensor data to serial console
  ESParaSite::Sensors::dumpSensor(false);
#endif

  // Initialize i2c eeprom
  Serial.println("Initializing i2c eeprom...");
  Serial.println();

  if (!ESParaSite::Eeprom::initI2cEeprom()) {
    Serial.println("No EEPROM Device Found, Please Check I2C Pins. Booting "
                   "in Config Portal mode.");
    ESParaSite::ConfigPortal::doConfigPortal();
  } else {
    ESParaSite::Eeprom::initEeprom();
  }

  // For Development Only.  FORMAT_EEPROM can be uncommented in DebugUtils.h.
#ifdef FORMAT_EEPROM
  ESParaSite::RtcEeprom::doEepromFormat(1);
#endif

  // Find the most recent EEPROM segment and populate our eeprom_data struct.
  uint16_t mruPageAddr = ESParaSite::Eeprom::doEepromFirstRead();

#ifdef DEBUG_L1
  Serial.print("The most recent write address:\t");
  Serial.print(mruPageAddr);
  Serial.print(" (0x");
  Serial.print(mruPageAddr, HEX);
  Serial.println(")");
  Serial.println();
#endif

  ESParaSite::Eeprom::doEepromRead(mruPageAddr);

  Serial.println("Startup Complete!");
  Serial.println();

#ifdef RESET_LIFE_COUNTERS
  eeprom.eepromVatLifeSec = 0;
  eeprom.eepromLedLifeSec = 0;
  eeprom.eepromScreenLifeSec = 0;
#endif

  ESParaSite::HttpCore::configHttpServerRouting();
  ESParaSite::HttpCore::startHttpServer();

#ifdef DEBUG_L1
  Serial.println("Waiting 3 Seconds...");
  delay(3000);
#endif

  // Turn led off as we are finished booting.
  digitalWrite(LED_BUILTIN, LED_OFF);
}

// **********************************************************
// LOOP
// **********************************************************
void loop() {
  // Check to see if Config Mode Triggered
  if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW)) {
    Serial.println("Configuration portal requested.");
    ESParaSite::ConfigPortal::doConfigPortal();
  }

  // Refresh the Current Loop Millisecond value
  curLoopMillis = millis();

#ifdef DEBUG_L4
  Serial.print("curLoopMillis:\t");
  Serial.println(curLoopMillis);
  Serial.print("prevSensorMillis:\t");
  Serial.println(prevSensorMillis);
  Serial.print("prevDhtMillis:\t");
  Serial.println(prevDhtMillis);
  Serial.print("prevEepromMillis:\t");
  Serial.println(prevEepromMillis);
  Serial.println();
#endif

#ifdef DEBUG_L4
  Serial.println("Checking to see if we need to read Sensors");
  Serial.println();
#endif

  // Read sensors if it has been the right interval
  prevSensorMillis =
      ESParaSite::Core::doReadSensors(curLoopMillis, prevSensorMillis);

#ifdef DEBUG_L4
  Serial.println("Checking to see if we need to read DHT12 Sensor");
  Serial.println();
#endif

  // Read the DHT12 sensor if it has been the right interval
  prevDhtMillis = ESParaSite::Core::doReadDht(curLoopMillis, prevDhtMillis);

#ifdef DEBUG_L4
  Serial.println("Checking to see if we need to Write to EEPROM");
  Serial.println();
#endif

  // Update EEPROM and memory values
  prevEepromMillis =
      ESParaSite::Core::doHandleEeprom(curLoopMillis, prevEepromMillis);

#ifdef DEBUG_L4
  Serial.println("Create History Digest");
  Serial.println();
#endif

  // Keep track of historical data and update Web UI data
  prevHistoryMillis =
      ESParaSite::Core::doHandleHistory(curLoopMillis, prevHistoryMillis);

#ifdef DEBUG_L2
  Serial.println("End of loop waiting 500msec");
  delay(500);
#endif
}