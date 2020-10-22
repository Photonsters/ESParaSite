/****************************************************************************************************************************
  Async_ConfigOnSwitch.ino
  For ESP8266 / ESP32 boards
  ESPAsync_WiFiManager is a library for the ESP8266/Arduino platform, using
 (ESP)AsyncWebServer to enable easy configuration and reconfiguration of WiFi
 credentials using a Captive Portal. Modified from
  1. Tzapu               (https://github.com/tzapu/WiFiManager)
  2. Ken Taylor          (https://github.com/kentaylor)
  3. Alan Steremberg     (https://github.com/alanswx/ESPAsyncWiFiManager)
  4. Khoi Hoang          (https://github.com/khoih-prog/ESP_WiFiManager)
  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
  Version: Version: 1.1.2
  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.11   K Hoang      21/08/2020 Initial coding to use (ESP)AsyncWebServer
 instead of (ESP8266)WebServer. Bump up to v1.0.11 to sync with ESP_WiFiManager
 v1.0.11 1.1.1    K Hoang      29/08/2020 Add MultiWiFi feature to autoconnect
 to best WiFi at runtime to sync with ESP_WiFiManager v1.1.1. Add setCORSHeader
 function to allow flexible CORS 1.1.2    K Hoang      17/09/2020 Fix bug in
 examples.
 *****************************************************************************************************************************/
/****************************************************************************************************************************
   This example will open a configuration portal when no WiFi configuration has
 been previously entered or when a button is pushed. It is the easiest scenario
 for configuration but requires a pin and a button on the ESP8266 device. The
 Flash button is convenient for this on NodeMCU devices. Also in this example a
 password is required to connect to the configuration portal network. This is
 inconvenient but means that only those who know the password or those already
 connected to the target WiFi network can access the configuration portal and
   the WiFi network credentials will be sent from the browser over an encrypted
 connection and can not be read by observers.
 *****************************************************************************************************************************/
//
#include <Arduino.h>
#include <WiFiClient.h>


#include "ConfigPortal.h"
#include "ESParaSite.h"
#include "FileCore.h"


#ifdef ESP32

#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "ESP32.h"

// From v1.1.1
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

#else

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "ESP8266.h"

// From v1.1.0
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#define ESP_getChipId() (ESP.getChipId())

#endif



// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _ESPASYNC_WIFIMGR_LOGLEVEL_ 3

// Default is 30s, using 20s now
#define TIME_BETWEEN_MODAL_SCANS 20000

// Default is 60s, using 30s now
#define TIME_BETWEEN_MODELESS_SCANS 30000

// SSID and PW for Config Portal

String ssid = "ESParaSite_" + String(ESP_getChipId(), HEX);
const char *password = "thisbugsme!";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

#define MIN_AP_PASSWORD_SIZE 8

#define SSID_MAX_LEN 32
// From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN 64

typedef struct {
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw[PASS_MAX_LEN];
} WiFi_Credentials;

typedef struct {
  String wifi_ssid;
  String wifi_pw;
} WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS 2

typedef struct {
  WiFi_Credentials WiFi_Creds[NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config WM_config;

// Indicates whether ESP has WiFi credentials saved from previous session, or
// double reset detected
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of
// Config Portal Comment out or use true to display Available Pages in
// Information Page of Config Portal Must be placed before #include
// <ESP_WiFiManager.h>
#define USE_AVAILABLE_PAGES true

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal
// from sketch. Valid only if DHCP is used. You'll loose the feature of
// dynamically changing from DHCP to static IP, or vice versa You have to
// explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to
// access Config Portal. See Issue 23: On Android phone ConfigPortal is
// unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP false

// Use true to enable CloudFlare NTP service. System can hang if you don't have
// Internet access while accessing CloudFlare See Issue #21: CloudFlare link in
// the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP false

// New in v1.0.11
#define USING_CORS_FEATURE true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you
// have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP true
#else
// You can select DHCP or Static IP here
//#define USE_DHCP_IP     true
#define USE_DHCP_IP false
#endif

//#if (USE_DHCP_IP ||
//     (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP))
// Use DHCP
//#warning Using DHCP IP
IPAddress stationIP = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP = IPAddress(192, 168, 2, 1);
IPAddress netMask = IPAddress(255, 255, 255, 0);
//#else
// Use static IP
//#warning Using static IP

// IPAddress stationIP = IPAddress(192, 168, 2, 232);
// IPAddress gatewayIP = IPAddress(192, 168, 2, 1);
// IPAddress netMask = IPAddress(255, 255, 255, 0);

#define USE_CONFIGURABLE_DNS true

IPAddress dns1IP = gatewayIP;
IPAddress dns2IP = IPAddress(8, 8, 8, 8);

#include <ESPAsync_WiFiManager.h> //https://github.com/khoih-prog/ESPAsync_WiFiManager

#define HTTP_PORT 80

extern ESParaSite::configData config;

uint8_t connectMultiWiFi(void) {

// For ESP32, this better be 0 to shorten the connect time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS 0

#define WIFI_MULTI_CONNECT_WAITING_MS 100L

  uint8_t status;

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ((Router_SSID != "") && (Router_Pass != "")) {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID,
              F(", Router_Pass = "), Router_Pass);
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") &&
        (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE)) {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid,
                F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
    }
  }

  LOGERROR(F("Connecting MultiWifi..."));

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  WiFi.config(stationIP, gatewayIP, netMask, dns1IP, dns2IP);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  WiFi.config(stationIP, gatewayIP, netMask);
#endif
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ((i++ < 20) && (status != WL_CONNECTED)) {
    status = wifiMulti.run();

    if (status == WL_CONNECTED)
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if (status == WL_CONNECTED) {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP());
  } else
    LOGERROR(F("WiFi not connected"));

  return status;
}

void toggleLED() {
  // toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void heartBeatPrint(void) {
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print("H"); // H means connected to WiFi
  else
    Serial.print("F"); // F means not connected to WiFi

  if (num == 80) {
    Serial.println();
    num = 1;
  } else if (num++ % 10 == 0) {
    Serial.print(" ");
  }
}

void check_WiFi(void) {
  if ((WiFi.status() != WL_CONNECTED)) {
    Serial.println("\nWiFi lost. Call connectMultiWiFi in loop");
    connectMultiWiFi();
  }
}

void check_status(void) {
  static ulong checkstatus_timeout = 0;
  static ulong LEDstatus_timeout = 0;
  static ulong checkwifi_timeout = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL 1000L
#define LED_INTERVAL 2000L
#define HEARTBEAT_INTERVAL 10000L

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0)) {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0)) {
    // Toggle LED at LED_INTERVAL = 2s
    toggleLED();
    LEDstatus_timeout = current_millis + LED_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0)) {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}

void ESParaSite::ConfigPortal::doConfigPortal() {

  AsyncWebServer webServer(HTTP_PORT);
  DNSServer dnsServer;

  unsigned long startedAt = millis();
  // Default Pin configuration values
  config.cfgPinSda = PIN_SDA;
  config.cfgPinScl = PIN_SCL;

  // Local intialization. Once its business is done, there is no need to keep it
  // around
  // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  // ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer,
                                            "ESParaSite");

  ESPAsync_wifiManager.setDebugOutput(true);

  // Use only to erase stored WiFi Credentials
  // resetSettings();
  ESPAsync_wifiManager.resetSettings();

  // set custom ip for portal
  // ESPAsync_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 100, 1),
  // IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));

  // I2C SCL and SDA parameters are integers so we need to convert them to
  // char array
  char convertedValue[3];
  snprintf(convertedValue, sizeof(convertedValue), "%d", config.cfgPinSda);
  ESPAsync_WMParameter p_pinSda("pinsda", "I2C SDA pin", convertedValue, 3);

  snprintf(convertedValue, sizeof(convertedValue), "%d", config.cfgPinScl);
  ESPAsync_WMParameter p_pinScl("pinscl", "I2C SCL pin", convertedValue, 3);

  // Extra parameters to be configured
  // After connecting, parameter.getValue() will get you the configured
  // value.
  // Format:
  // <ID> <Placeholder text> <default value> <length> <custom HTML>
  //  <label placement>

  // Hints for each section
  ESPAsync_WMParameter p_hint("<small>Enter your WiFi credentials above"
                              "</small>");
  ESPAsync_WMParameter p_hint2("<small>Enter the SDA and SCL Pins for your"
                               "ESParaSite</small>");
  ESPAsync_WMParameter p_hint3("</br><small>If you have multiple ESParaSites,"
                               " give each a unique name</small>");
  ESPAsync_WMParameter p_hint4("<small>Enable mDNS</small>");

  char customhtml[24];
  snprintf(customhtml, sizeof(customhtml), "%s", "type=\"checkbox\"");
  int8_t len = strlen(customhtml);
  snprintf(customhtml + len, (sizeof customhtml) - len, "%s", " checked");
  ESPAsync_WMParameter p_mdnsEnabled("mdnsen", "Enable mDNS", "T", 2,
                                     customhtml);
  ESPAsync_WMParameter p_mdnsName("mdnsname", "mDNSName", "esparasite", 32);

  // add all parameters here

  ESPAsync_wifiManager.addParameter(&p_hint);
  ESPAsync_wifiManager.addParameter(&p_hint2);
  ESPAsync_wifiManager.addParameter(&p_pinSda);
  ESPAsync_wifiManager.addParameter(&p_pinScl);
  ESPAsync_wifiManager.addParameter(&p_hint4);
  ESPAsync_wifiManager.addParameter(&p_mdnsEnabled);
  ESPAsync_wifiManager.addParameter(&p_hint3);
  ESPAsync_wifiManager.addParameter(&p_mdnsName);

  ESPAsync_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESPAsync_wifiManager.setConfigPortalChannel(0);
  //////

#if !USE_DHCP_IP
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  ESPAsync_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask,
                                            dns1IP, dns2IP);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  ESPAsync_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask);
#endif
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // We can't use WiFi.SSID() in ESP32as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are
  // also cleared in reboot Have to create a new function to store in
  // EEPROM/SPIFFS for this purpose
  Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
  Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

  // Remove this line if you do not want to see WiFi password printed
  Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  // SSID to uppercase
  ssid.toUpperCase();

  /*  // From v1.1.0, Don't permit NULL password
    if ((Router_SSID != "") && (Router_Pass != "")) {
      LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
      wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());

      ESPAsync_wifiManager.setConfigPortalTimeout(
          120); // If no access point name has been previously entered disable
                // timeout.
      Serial.println("Got stored Credentials. Timeout 120s for Config Portal");
    } else {
      Serial.println(
          "Open Config Portal without Timeout: No stored Credentials.");

      initialConfig = true;
    }

    if (initialConfig) {

    */
  Serial.println("Starting configuration portal.");
  digitalWrite(LED_BUILTIN,
               LED_ON); // Turn led on as we are in configuration mode.

  // sets timeout in seconds until configuration portal gets turned off.
  // If not specified device will remain in configuration mode until
  // switched off via webserver or device is restarted.
  // ESPAsync_wifiManager.setConfigPortalTimeout(600);

  // Starts an access point
  if (!ESPAsync_wifiManager.startConfigPortal((const char *)ssid.c_str(),
                                              password))
    Serial.println("Not connected to WiFi but continuing anyway.");
  else {
    Serial.println("WiFi connected...yeey :)");
  }

  // Stored  for later usage, from v1.1.0, but clear first
  memset(&WM_config, 0, sizeof(WM_config));

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
    String tempSSID = ESPAsync_wifiManager.getSSID(i);
    String tempPW = ESPAsync_wifiManager.getPW(i);

    if (strlen(tempSSID.c_str()) <
        sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
      strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
    else
      strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(),
              sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

    if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
      strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
    else
      strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(),
              sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);

    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") &&
        (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE)) {
      LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid,
                F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
      wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid,
                      WM_config.WiFi_Creds[i].wifi_pw);
    }
  }

  int i = 0;
  config.cfgWifiSsid = WM_config.WiFi_Creds[i].wifi_ssid;
  config.cfgWifiPassword = WM_config.WiFi_Creds[i].wifi_pw;

  // Getting posted form values and overriding local variables parameters
  // Config file is written regardless the connection state
  config.cfgPinSda = atoi(p_pinSda.getValue());
  config.cfgPinScl = atoi(p_pinScl.getValue());

  if (strncmp(p_mdnsEnabled.getValue(), "T", 1) != 0) {
    Serial.println("mDNS Disabled");
    config.cfgMdnsEnabled = false;
  } else {
    config.cfgMdnsEnabled = true;

    snprintf(config.cfgMdnsName, sizeof(config.cfgMdnsName), "%s\n",
             p_mdnsName.getValue());

    Serial.println("mDNS Enabled");
  }

  if (!(ESParaSite::FileCore::saveConfig())) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config Saved");
    Serial.println();

    //}

    // Turn led off as we are not in configuration mode.

    digitalWrite(LED_BUILTIN,
                 LED_OFF);
    startedAt = millis();

    if (!initialConfig) {
      // Load stored data, the addAP ready for MultiWiFi reconnection
      ESParaSite::FileCore::loadConfig();

      for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") &&
            (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE)) {
          LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid,
                    F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
          wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid,
                          WM_config.WiFi_Creds[i].wifi_pw);
        }
      }

      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ConnectMultiWiFi in setup");

        connectMultiWiFi();
      }
    }

    Serial.print("After waiting ");
    Serial.print((float)(millis() - startedAt) / 1000L);
    Serial.print(" secs more in setup(), connection result is ");

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("connected. Local IP: ");
      Serial.println(WiFi.localIP());
    } else
      Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));


    Serial.println("Resetting ESParaSite");
    Serial.println();
    delay(5000);

    // We restart the ESP8266 to reload with changes.
    ESP.restart();
    check_status();
  }
}