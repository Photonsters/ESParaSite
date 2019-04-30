//ESParaSite_Core.cpp

/* ESParasite Data Logger v0.3
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>
#include <RtcDateTime.h>
#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <EepromAT24C32.h>
#include <BlueDot_BME280.h>
#include <DHT12.h>
#include "ESParaSite_Core.h"

//Initialize Libraries
ESP8266WebServer http_rest_server(HTTP_REST_PORT);

printchamber chamber_resource;
optics optics_resource;
ambient ambient_resource;
enclosure enclosure_resource;

void loop(void) {
  http_rest_server.handleClient();
}

int init_wifi() {
  // Connect to WiFi network
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("\n\r \n\rConnecting to Wifi");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  return WiFi.status(); // return the WiFi connection status
}

void config_rest_server_routing() {
  http_rest_server.on("/", HTTP_GET, []() {
    http_rest_server.send(200, "text/html",
                          "Welcome to the ESParasite REST Web Server");
  });
  http_rest_server.on("/printchamber", HTTP_GET, get_chamber);
  http_rest_server.on("/optics", HTTP_GET, get_optics);
  http_rest_server.on("/ambient", HTTP_GET, get_ambient);
  http_rest_server.on("/enclosure", HTTP_GET, get_enclosure);
  // http_rest_server.on("/enclosure", HTTP_POST, post_enclosure); //Not yet implemented
  // http_rest_server.on("/enclosure", HTTP_PUT, post_enclosure);  //Not yet implemented
}

void get_chamber () {

  read_dht_sensor();
  read_rtc_data();
  create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = timestamp;
  doc["seconds_t"] = chamber_resource.dht_temp_c;
  doc["seconds_s"] = chamber_resource.dht_humidity;
  doc["seconds_l"] = chamber_resource.dht_dewpoint;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_optics () {

  read_si_sensor();
  read_mlx_sensor();
  read_rtc_data();
  create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = timestamp;
  doc["uvindex"] = optics_resource.si_uvindex;
  doc["visible"] = optics_resource.si_visible;
  doc["infrared"] = optics_resource.si_infrared;
  doc["led_temp_c"] = optics_resource.mlx_amb_temp_c;
  doc["screen_temp_c"] = optics_resource.mlx_obj_temp_c;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_ambient() {

  read_bme_sensor();
  read_rtc_data();
  create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "ambient";
  doc["timestamp"] = timestamp;
  doc["amb_temp_c"] = ambient_resource.bme_temp_c;
  doc["amb_humidity"] = ambient_resource.bme_humidity;
  doc["amb_pressure"] = ambient_resource.bme_barometer;
  doc["altitude"] = ambient_resource.bme_altitude;

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void get_enclosure() {

  read_rtc_data();
  //  read_at24_data();   //Placeholder - Not yet Implemented
  create_timestamp(now);

  StaticJsonDocument<256> doc;

  doc["class"] = "enclosure";
  doc["timestamp"] = timestamp;
  doc["case_temp"] = enclosure_resource.case_temp;
  doc["seconds_t"] = enclosure_resource.total_sec;  //Placeholder - Not yet Implemented
  doc["seconds_s"] = enclosure_resource.screen_sec; //Placeholder - Not yet Implemented
  doc["seconds_l"] = enclosure_resource.led_sec;    //Placeholder - Not yet Implemented

  serializeJson(doc, Serial);
  Serial.println();

  String output = "JSON = ";
  serializeJsonPretty(doc, output);
  http_rest_server.send(200, "application/json", output);

  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void setup(void) {
  Serial.begin(9600);
  Serial.println("");

  if (init_wifi() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(wifi_ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.print("Error connecting to: ");
    Serial.println(wifi_ssid);
  }

  config_rest_server_routing();

  Serial.println("");
  Serial.println("ESParaSite Data Logging Server");
  Serial.print("Compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  Serial.println("");

  //start http rest server
  http_rest_server.begin();
  Serial.println("HTTP REST server started");
  Serial.println();

  // initialize I2C bus
  Serial.println("Initialize I2C bus");
  Wire.begin(0, 2);
  Serial.println("OK!");
  Serial.println();

  // initialize DHT12 sensor
  Serial.println("Initialize DHT12 Sensor");
  init_dht_sensor();
  Serial.println();

  // initialize SI1145 UV sensor
  Serial.println("Read SI1145 sensor");
  if (! uv.begin()) {
    Serial.println("SI1145 Initialization Failure");
  }
  Serial.println("OK!");
  Serial.println();

  // initialize MLX90614 temperature sensor
  Serial.println("Read MLX90614 sensor");
  if (! mlx.begin()) {
    Serial.println("MLX90614 Initialization Failure");
  }
  Serial.println("OK!");
  Serial.println();

  // initialize bme80 temperature sensor
  Serial.println("Read BME280 sensor");
  init_bme_sensor();
  Serial.println();

  // initialize DS3231 RTC
  Serial.println("Read DS3231 RTC sensor");
  init_rtc_clock();
  Serial.println();

  //Dump all Sensor data to Serial
  Serial.println();
  Serial.println("Current Sensor Readings");
  Serial.println("============================================================");
  Serial.println();
  Serial.println("DS3231 Real-Time Clock Timestamp and Temperature:");
  read_rtc_data();
  Serial.println();

  Serial.println("DHT12 Print Chamber Environmental Data:");
  read_dht_sensor();
  Serial.println();

  Serial.println("SI1145 UV and Light Sensor Data:");
  read_si_sensor();
  Serial.println();

  Serial.println("MLX90614 Temp Sensor Data:");
  read_mlx_sensor();
  Serial.println();

  Serial.println("BME280 Temp Sensor Data:");
  read_bme_sensor();
  Serial.println();
  Serial.println("ESParasite Ready!");
}


void  init_dht_sensor() {
  // initialize DHT12 temperature sensor
}

void init_bme_sensor() {
  // initialize BME280 temperature sensor

  bme.parameter.communication = 0;
  bme.parameter.I2CAddress = bme_i2c_address;
  bme.parameter.sensorMode = 0b11;
  bme.parameter.IIRfilter = 0b100;
  bme.parameter.humidOversampling = 0b101;
  bme.parameter.tempOversampling = 0b101;
  bme.parameter.pressOversampling = 0b101;
  bme.parameter.pressureSeaLevel = SEALEVELPRESSURE_HPA;
  bme.parameter.tempOutsideCelsius = CURRENTAVGTEMP_C;
  bme.parameter.tempOutsideFahrenheit = CURRENTAVGTEMP_F;

  if (bme.init() != 0x60)
  {
    Serial.println(F("BME280 Sensor not found!"));
    bmeDetected = 0;
  }
  else
  {
    Serial.println(F("BME280 Sensor detected!"));
    bmeDetected = 1;
  }
  Serial.println();
}

void init_rtc_clock() {
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

}

void read_rtc_data () {
  Serial.println("===================");
  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }

  now = Rtc.GetDateTime();
  printDateTime(now);
  Serial.println();

  RtcTemperature temp = Rtc.GetTemperature();
  enclosure_resource.case_temp = (temp.AsFloatDegC());
  Serial.print(enclosure_resource.case_temp);
  Serial.println("°C");

  delay(500);
}

void read_dht_sensor() {
  Serial.println("===================");

  //First DHT measurement is stale, so we measure, wait ~2 seconds, then measure again.
  delay(2500);
/*
  Serial.print(F("Temperature (°C): "));
  chamber_resource.dht_temp_c = ((float)DHT.getTemperature() / (float)10);
  Serial.println(((int)chamber_resource.dht_temp_c));

  Serial.print(F("Humidity: "));
  chamber_resource.dht_humidity = ((float)DHT.getHumidity() / (float)10);
  Serial.print(((int)chamber_resource.dht_humidity));
  Serial.println("%");

  Serial.print(F("Dew Point (°C): "));
  chamber_resource.dht_dewpoint = ((float)DHT.dewPoint());
  Serial.println(((int)chamber_resource.dht_dewpoint));

  */
}

void read_si_sensor() {
  Serial.println("===================");

  optics_resource.si_uvindex = uv.readUV();
  optics_resource.si_uvindex /= 100.0;
  Serial.print("UV Index: ");
  Serial.println((int)optics_resource.si_uvindex);

  optics_resource.si_visible = uv.readVisible();
  Serial.print("Vis: ");
  Serial.println(optics_resource.si_visible);

  optics_resource.si_infrared = uv.readIR();
  Serial.print("IR: ");
  Serial.println(optics_resource.si_infrared);

  delay(1000);
}

void read_mlx_sensor() {
  Serial.println("===================");

  optics_resource.mlx_amb_temp_c = mlx.readAmbientTempC();
  Serial.print("Ambient = ");
  Serial.print(optics_resource.mlx_amb_temp_c);
  Serial.print("°C\t");
  Serial.print(mlx.readAmbientTempF());
  Serial.println("°F");

  optics_resource.mlx_obj_temp_c = mlx.readObjectTempC();
  Serial.print("Object = ");
  Serial.print(optics_resource.mlx_obj_temp_c);
  Serial.print("°C\t");
  Serial.print(mlx.readObjectTempF());
  Serial.println("°F");
  Serial.println();

  delay(1000);
}

void read_bme_sensor() {
  Serial.println("===================");
  //  if (bmeDetected)
  //  {
  ambient_resource.bme_temp_c = bme.readTempC();
  Serial.print(F("Temperature Sensor:\t\t"));
  Serial.print(ambient_resource.bme_temp_c);
  Serial.print("°C\t");
  Serial.print(bme.readTempF());
  Serial.println("°F");

  ambient_resource.bme_humidity = bme.readHumidity();
  Serial.print(F("Humidity Sensor:\t\t"));
  Serial.print(ambient_resource.bme_humidity);
  Serial.println("%");

  ambient_resource.bme_barometer = bme.readPressure();
  Serial.print(F("Pressure Sensor [hPa]:\t"));
  Serial.print(ambient_resource.bme_barometer);
  Serial.println(" hPa");

  ambient_resource.bme_altitude = bme.readAltitudeMeter();
  Serial.print(F("Altitude Sensor:\t\t"));
  Serial.print(ambient_resource.bme_altitude);
  Serial.print("m\t");
  Serial.print(bme.readAltitudeFeet());
  Serial.println("ft");
  //  }

  /*  else
    {
      Serial.print(F("Temperature Sensor [°C]:\t\t"));
      Serial.println(F("Null"));
      Serial.print(F("Temperature Sensor [°F]:\t\t"));
      Serial.println(F("Null"));
      Serial.print(F("Humidity Sensor [%]:\t\t\t"));
      Serial.println(F("Null"));
      Serial.print(F("Pressure Sensor [hPa]:\t\t"));
      Serial.println(F("Null"));
      Serial.print(F("Altitude Sensor [m]:\t\t\t"));
      Serial.println(F("Null"));
      Serial.print(F("Altitude Sensor [ft]:\t\t\t"));
      Serial.println(F("Null"));
    }
  */
  Serial.println();
  Serial.println();

  delay(1000);

}

int convertCtoF(int temp_c)  {
  int temp_f;
  temp_f = ((int)round(1.8 * temp_c + 32));
  return temp_f;
}

void printDateTime(const RtcDateTime & dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}

void create_timestamp(const RtcDateTime & dt)
{
  snprintf_P(timestamp,
             countof(timestamp),
             PSTR("%04u%02u%02u%02u%02u%02u"),
             dt.Year(),
             dt.Month(),
             dt.Day(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(timestamp);
}
