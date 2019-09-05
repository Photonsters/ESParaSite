//ESParaSite_Core.cpp

/* ESParasite Data Logger v0.4
	Authors: Andy  (SolidSt8Dad)Eakin

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

//Put your WiFi network and WiFi password here:

//const char* wifi_ssid     = "yourwifinetwork";
//const char* wifi_password = "yourwifipassword";

#include "wifi.ini" //Delete or comment this line if you have input your wifi information above.

//+++ Advanced Settings +++
//VISIBLE_THRESHOLD adjusts the sensitivity of the SI1145 Sensor to Ambient light when used to detect printer actively printing.  Default is (280).
#define VISIBLE_THRESHOLD (280)

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
timestamp timestamp_resource;
eeprom_data rtc_eeprom_resource;

time_t rtc_cur_epoch;

void loop(void)
{
  do_client();

  do_sensors();

  do_eeprom();
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  Serial.println("");
  Serial.println("ESParaSite Data Logging Server");
  Serial.println("https://github.com/Photonsters/ESParaSite_TemperatureLogger");
  Serial.print("Compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  Serial.println("");

  if (init_wifi() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(wifi_ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.print("Error connecting to: ");
    Serial.println(wifi_ssid);
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

  Serial.println("Startup Complete!");
  Serial.println();
}

int init_wifi()
{
  // Connect to WiFi network
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("\n\r \n\rConnecting to Wifi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  return WiFi.status(); // return the WiFi connection status
}

void do_sensors()
{
  rtc_cur_epoch = read_rtc_epoch();

  if ((rtc_cur_epoch) >= ((timestamp_resource.current_second) + POLLING_INTERVAL_SEC))
  {
    timestamp_resource.current_second = (rtc_cur_epoch);
    enclosure_resource.life_sec = rtc_cur_epoch - rtc_eeprom_resource.first_on_timestamp;
    read_dht_sensor();
    read_si_sensor();
    read_bme_sensor();
    read_mlx_sensor();
    read_rtc_temp();
    Serial.println();
  }
}

void do_check_printing()
{
}
void do_eeprom()
{
  // We need some code here to detect if printer is on  and update the values before we write to eeprom

  if ((rtc_cur_epoch) >= ((rtc_eeprom_resource.last_write_timestamp) + EEPROM_WRITE_INTERVAL_SEC))
  {
    do_eeprom_write();
  }
}