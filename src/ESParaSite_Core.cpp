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
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <BlueDot_BME280.h>
#include <dht12.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>

#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

//Put your WiFi network and WiFi password here:

//const char* wifi_ssid     = "yourwifinetwork";
//const char* wifi_password = "yourwifipassword";

const char *wifi_ssid = "mxbnet";
const char *wifi_password = "Presto1234!";

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure corrected for the sea level
// Otherwise leave the standard pressure as default (1013.25 hPa);
// Also put in the current average temperature outside (yes, really outside!)
// For slightly less precise altitude measurements, just leave the standard temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

//Set the I2C address of your BME280 breakout board
//int bme_i2c_address = 0x77;

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

printchamber chamber_resource;
optics optics_resource;
ambient ambient_resource;
enclosure enclosure_resource;

unsigned long delayTime;
int bmeDetected = 0;
int bme_i2c_address;
time_t rtc_timestamp;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
BlueDot_BME280 bme;
DHT12 dht;

//Initialize Libraries

void loop(void)
{
  do_client();
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

void setup(void)
{
  Serial.begin(115200);
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

  Serial.println("");
  Serial.println("ESParaSite Data Logging Server");
  Serial.print("Compiled: ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
  Serial.println("");

  //start http rest server
  start_http_server();
  Serial.println("HTTP REST server started");
  config_rest_server_routing();
  Serial.println();

  // initialize I2C bus
  Serial.print("Init I2C bus...");
  Wire.begin(0, 4);
  Serial.println("\t\t\t\t\t\t\tOK!");
  Serial.println();

  // initialize DHT12 sensor
  Serial.print("Init DHT12 Sensor...");
  int i2c_addr = 0x5C;
  int error = ping_sensor(i2c_addr);
  if (error == 0)
  {
    init_dht_sensor();
  }
  Serial.println();

  // initialize SI1145 UV sensor
  Serial.print("Init SI1145 sensor...");
  i2c_addr = 0x60;
  error = ping_sensor(i2c_addr);
  if (error == 0)
  {
    if (!uv.begin())
    {
      Serial.println("SI1145 Initialization Failure!");
    }
    else
    {
      Serial.println("OK!");
    }
  }
  Serial.println();

  // initialize MLX90614 temperature sensor
  Serial.print("Init MLX90614 sensor...");
  i2c_addr = 0x5A;
  error = ping_sensor(i2c_addr);
  if (error == 0)
  {
    if (!mlx.begin())
    {
      Serial.println("MLX90614 Initialization Failure");
    }
    else
    {
      Serial.println("OK!");
    }
  }
  Serial.println();

  // initialize BME280 temperature sensor
  Serial.print("Init BME280 sensor...");
  i2c_addr = 0x76;
  error = ping_sensor(i2c_addr);
  if (error == 0)
  {
    bme_i2c_address = i2c_addr;
    init_bme_sensor();
  }
  else
  {
    i2c_addr = 0x77;
    error = ping_sensor(i2c_addr);
    if (error == 0)
    {
      bme_i2c_address = i2c_addr;
      init_bme_sensor();
    }
  }
  Serial.println();

  // initialize DS3231 RTC
  Serial.print("Init DS3231 RTC...");
  i2c_addr = 0x68;
  error = ping_sensor(i2c_addr);
  if (error == 0)
  {
    Serial.println("OK!");
    Serial.print("Init AT24C32 EEPROM...");
    i2c_addr = 0x57;
    error = ping_sensor(i2c_addr);
    if (error == 0)
    {
      Serial.println("OK!");
      init_rtc_clock();
    }
  }
  Serial.println();

  //Dump all Sensor data to Serial
  Serial.println();
  Serial.println("Current Sensor Readings");
  Serial.println("============================================================");
  Serial.println();
  Serial.println("DS3231 Real-Time Clock Timestamp and Temperature:");
  read_rtc_data();
  Serial.println();

  Serial.println("dht12 Print Chamber Environmental Data:");
  read_dht_sensor();
  Serial.println();

  Serial.println("SI1145 UV and Light Sensor Data:");
  read_si_sensor();
  Serial.println();

  Serial.println("MLX90614 LED Temp Sensor Data:");
  read_mlx_sensor();
  Serial.println();

  Serial.println("BME280 Ambient Temp Sensor Data:");
  read_bme_sensor();
  Serial.println();
  Serial.println("ESParaSite Ready!");
  
  loop();
}

int ping_sensor(int address)
{
  byte error;
  Wire.beginTransmission(address);
  error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.print("I2C device found at address 0x");
    if (address < 16)
      Serial.print("0");
    Serial.print(address, HEX);
    Serial.print("\t\t");
  }
  else if (error == 2)
  {
    Serial.print("No device (Ping recieved NACK) at address 0x");
    if (address < 16)
      Serial.print("0");
    Serial.print(address, HEX);
    Serial.print("\t\t");
  }
  else if (error == 4)
  {
    Serial.print("Unknown error at address 0x");
    if (address < 16)
      Serial.print("0");
    Serial.print(address, HEX);
    Serial.print("\t\t");
  }
  return error;
}

void init_dht_sensor()
{
  // initialize dht12 temperature sensor
  int status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    Serial.println("OK!\t");
    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.println("Checksum error,\t");
    break;
  case DHT12_ERROR_CONNECT:
    Serial.println("Connect error,\t");
    break;
  case DHT12_MISSING_BYTES:
    Serial.println("Missing bytes,\t");
    break;
  default:
    Serial.println("Unknown error,\t");
    break;
  }
}

void init_bme_sensor()
{
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
    Serial.println(F("OK!"));
    bmeDetected = 1;
  }
}

void init_rtc_clock()
{
}

void read_rtc_data()
{
}

void read_dht_sensor()
{
  Serial.println("===================");

  //First dht measurement is stale, so we measure, wait ~2 seconds, then measure again.

  int status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHT12_ERROR_CONNECT:
    Serial.print("Connect error,\t");
    break;
  case DHT12_MISSING_BYTES:
    Serial.print("Missing bytes,\t");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }

  delay(2500);

  status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    Serial.print(F("Temperature (°C): "));
    chamber_resource.dht_temp_c = dht.temperature;
    Serial.println(chamber_resource.dht_temp_c, 1);

    Serial.print(F("Humidity: "));
    chamber_resource.dht_humidity = dht.humidity;
    Serial.print(chamber_resource.dht_humidity, 1);
    Serial.println("%");

    /*Serial.print(F("Dew Point (°C): "));
  chamber_resource.dht_dewpoint = ((float)dht.dewPoint());
  Serial.println(((int)chamber_resource.dht_dewpoint));
*/
    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHT12_ERROR_CONNECT:
    Serial.print("Connect error,\t");
    break;
  case DHT12_MISSING_BYTES:
    Serial.print("Missing bytes,\t");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }
}

void read_si_sensor()
{
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

void read_mlx_sensor()
{
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

void read_bme_sensor()
{
  Serial.println("===================");

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

  Serial.println();
  Serial.println();

  delay(1000);
}

int convertCtoF(int temp_c)
{
  int temp_f;
  temp_f = ((int)round(1.8 * temp_c + 32));
  return temp_f;
}
