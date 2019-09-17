//ESParaSite_Core.cpp

/* ESParasite Data Logger v0.5
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Time.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <FS.h>
#include <ESP8266mDNS.h>

#include <WiFiManager.h>

#include <ArduinoJson.h>

#include <BlueDot_BME280.h>
#include <dht12.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>
#include <RtcDS3231.h>

#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"
#include "ESParaSite_Eeprom.h"
#include "ESParaSite_Sensors.h"
#include "ESParaSite_Util.h"
#include "ESParaSite_WiFi.h"
#include "ESParaSite_FileConfig.h"
#include "ESParaSite_PortalConfig.h"

//+++ Advanced Settings +++
//VISIBLE_THRESHOLD adjusts the sensitivity of the SI1145 Sensor to Ambient light when used to detect printer actively printing.  Default is (280).
#define VISIBLE_THRESHOLD (280.00)

//*** It IS STRONGLY RECOMMENDED THAT YOU DO NOT MODIFY THE BELOW VALUES ***
// POLLING_INTERVAL_SEC determines how often the sensor array should be polled for new data.  Since we cannot poll the DHT12 more often than every 2000ms, The minimum polling interval is 3s.
// Increasing the polling interval will reduce the performance requirement, however it will decrease the resolution of the lifetime counter.
#define POLLING_INTERVAL_SEC (3)
// EEPROM_WRITE_INTERVAL_SEC determines how often we update the EEPROM that contains the various lifetime counters. The value of 30s gives us over 13 years of EEPROM life.
// Reducing this value will burn out the EEPROM chip faster. Increasing it will increase the EEPROM life, but at the cost of a higher margin of error for the lifetime counters.
// The margin of error for the DEFAULT 30s Interval is ~2.5hours/year.
#define EEPROM_WRITE_INTERVAL_SEC (30)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

printchamber chamber_resource;
optics optics_resource;
ambient ambient_resource;
enclosure enclosure_resource;
status status_resource;
eeprom_data rtc_eeprom_resource;
config_data config_resource;

time_t last_poll_sec = 0;
int is_printing_counter = 0;
boolean is_new_boot = 0;

const int TRIGGER_PIN = 13; // D7 on NodeMCU and WeMos.
const int TRIGGER_PIN2 = 13;
void loop(void)
{
  if ((digitalRead(TRIGGER_PIN) == LOW) || (digitalRead(TRIGGER_PIN2) == LOW))
  {
    WiFi.disconnect(true);
    WiFi.status();
    Serial.println("");
    Serial.println("HALTING LOOP - Configuration Mode Triggered.");
    Serial.println("");
    delay(5000);
    do_config_portal();
  }
  MDNS.update();

  do_client();

  do_sensors();

  do_eeprom();
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.println("");
  Serial.println("ESParaSite Data Logging Server");
  Serial.println("https://github.com/Photonsters/ESParaSite_TemperatureLogger");
  Serial.print("Compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  Serial.println("");

  Serial.println("Mounting FS...");

  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  if (!loadConfig())
  {
    Serial.println("Failed to load config. Booting in AP Config Portal mode.");

    do_config_portal();
  }

  if (!WiFi.getAutoConnect() || init_wifi() != WL_CONNECTED)
  {
    Serial.println();
    Serial.println("Error connecting to WiFi Network - Starting Config Portal");

    do_config_portal();
  }
  else if (WiFi.getAutoConnect())
  {
    Serial.println();
    Serial.println("");
    Serial.print("Using Autoconnect");
    Serial.println("");
    Serial.println();
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
  }
  else
  {
    Serial.print("Using config.json");
    Serial.println("");
    Serial.print("Connected to: ");
    Serial.println(config_resource.cfg_wifi_ssid);
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.println();

  Serial.print("mDNS Enabled:\t");
  Serial.println(config_resource.cfg_mdns_enabled);
  Serial.print("mDNS URL: http://");
  Serial.print(config_resource.cfg_mdns_name);
  Serial.println(".local");

  if (config_resource.cfg_mdns_enabled == 1)
  {
    const char *mdns_n = config_resource.cfg_mdns_name;
    if (!MDNS.begin(mdns_n))
    { // Start the mDNS responder for esp8266.local
      Serial.println("Error setting up MDNS responder!");
    }
    else
    {
      Serial.println("mDNS responder started");
    }
  }

  //start http rest server
  start_http_server();
  config_rest_server_routing();
  Serial.println("HTTP REST server started");
  Serial.println();

  //initialize i2c bus, sensors, rtc and eeprom
  init_i2c_sensors();

  //dump all sensor data to serial console
  dump_sensors();

  //Find the most recent EEPROM segment and populate our eeprom_data struct.
  int mru_segment_addr = do_eeprom_first_read();

  Serial.print("The most recent write address:\t");
  Serial.println(mru_segment_addr);
  Serial.println();

  do_eeprom_read(mru_segment_addr);

  last_poll_sec = status_resource.rtc_current_second;
  is_new_boot = true;

  Serial.println("=========first_read");
  Serial.print("last_poll_sec \t");
  Serial.println((int)last_poll_sec);
  Serial.println();

  Serial.println("Startup Complete!");
  Serial.println();
}

int do_sensors()
{
  status_resource.rtc_current_second = read_rtc_epoch();

  if ((status_resource.rtc_current_second) >= ((last_poll_sec) + POLLING_INTERVAL_SEC))
  {
    enclosure_resource.life_sec = (status_resource.rtc_current_second - rtc_eeprom_resource.first_on_timestamp);
    Serial.print("This Printer has been on for:\t");
    Serial.print(enclosure_resource.life_sec);
    Serial.println("  seconds");
    read_dht_sensor();
    read_bme_sensor();
    read_mlx_sensor();
    read_rtc_temp();
    read_si_sensor();
    do_check_printing();

    last_poll_sec = status_resource.rtc_current_second;

    return 1;
  }
  else
  {
    return 0;
  }
}

void do_check_printing()
{
  if (optics_resource.si_visible >= VISIBLE_THRESHOLD)
  {
    is_printing_counter++;
  }
}

void do_eeprom()
{
  if ((status_resource.rtc_current_second) >= ((rtc_eeprom_resource.last_write_timestamp) + EEPROM_WRITE_INTERVAL_SEC) && !is_new_boot)
  {
    Serial.println();
    Serial.print("rtc_current_second\t");
    Serial.println(status_resource.rtc_current_second);
    Serial.print("last_write_timestamp\t");
    Serial.println(rtc_eeprom_resource.last_write_timestamp);

    if (is_printing_counter >= (int(EEPROM_WRITE_INTERVAL_SEC / 7))) //if 4 or more out of our last 10 poll intervals detect light.
    {
      status_resource.is_printing_flag = 1;

      Serial.println();
      Serial.print("Is Printing Flag\t");
      Serial.println(status_resource.is_printing_flag);

      is_printing_counter = 0;

      rtc_eeprom_resource.screen_life_seconds += EEPROM_WRITE_INTERVAL_SEC;
      rtc_eeprom_resource.led_life_seconds += EEPROM_WRITE_INTERVAL_SEC;
      rtc_eeprom_resource.fep_life_seconds += EEPROM_WRITE_INTERVAL_SEC;

      do_eeprom_write();
    }
    else
    {
      status_resource.is_printing_flag = 0;

      Serial.print("Is Printing Flag\t");
      Serial.println(status_resource.is_printing_flag);

      is_printing_counter = 0;

      do_eeprom_write();
    }
  }
  else if (is_new_boot)
  {
    is_new_boot = false;
    Serial.print("\nLet's wait to write until Next time\n");
  }
}