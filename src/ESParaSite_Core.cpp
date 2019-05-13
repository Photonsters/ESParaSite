//ESParaSite_Core.cpp

/* ESParasite Data Logger v0.3
	Authors: Andy  (SolidSt8Dad)Eakin

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
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>
//#include <RtcDateTime.h>
//#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <EepromAT24C32.h>
#include <BlueDot_BME280.h>
#include <dht12.h>
#include <Time.h>
#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"
#include "ESParaSite_Settings.hxx"

#define countof(a) (sizeof(a) / sizeof(a[0]))

unsigned long delayTime;
int bmeDetected = 0;

time_t timestamp;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
BlueDot_BME280 bme;
DHT12 dht;
//RtcDS3231<TwoWire> Rtc(Wire);
//EepromAt24c32<TwoWire> RtcEeprom(Wire);

//Initialize Libraries

printchamber chamber_resource;
optics optics_resource;
ambient ambient_resource;
enclosure enclosure_resource;

void loop(void)
{
  http_rest_server.handleClient();
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
  Serial.begin(9600);
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

  // initialize dht12 sensor
  Serial.println("Initialize dht12 Sensor");
  init_dht_sensor();
  Serial.println();

  // initialize SI1145 UV sensor
  Serial.println("Read SI1145 sensor");
  if (!uv.begin())
  {
    Serial.println("SI1145 Initialization Failure");
  }
  Serial.println("OK!");
  Serial.println();

  // initialize MLX90614 temperature sensor
  Serial.println("Read MLX90614 sensor");
  if (!mlx.begin())
  {
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

  Serial.println("dht12 Print Chamber Environmental Data:");
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

void init_dht_sensor()
{
  // initialize dht12 temperature sensor
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("DHT12 LIBRARY VERSION: ");
  Serial.println(DHT12_VERSION);
  Serial.println();
  read_bme_sensor();
  Serial.println("Type,\tStatus,\tHumidity (%),\tTemperature (C)");
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
    Serial.println(F("BME280 Sensor detected!"));
    bmeDetected = 1;
  }
  Serial.println();
}

void init_rtc_clock()
{
 /* Rtc.Begin();

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
*/}

void read_rtc_data()
{
  /*Serial.println("===================");
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
  printDateTime(rtcnow);
  Serial.println();

  RtcTemperature temp = Rtc.GetTemperature();
  enclosure_resource.case_temp = (temp.AsFloatDegC());
  Serial.print(enclosure_resource.case_temp);
  Serial.println("°C");

  delay(500);*/
}

void read_dht_sensor()
{
  Serial.println("===================");

  //First dht measurement is stale, so we measure, wait ~2 seconds, then measure again.

  int status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    Serial.print("OK,\t");
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

  Serial.print("dht12, \t");
  status = dht.read();
   switch (status)
  {
  case DHT12_OK:
    Serial.print("OK,\t");
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

  delay(2000);
  /*
  Serial.print(F("Temperature (°C): "));
  chamber_resource.dht_temp_c = ((float)dht.getTemperature() / (float)10);
  Serial.println(((int)chamber_resource.dht_temp_c));

  Serial.print(F("Humidity: "));
  chamber_resource.dht_humidity = ((float)dht.getHumidity() / (float)10);
  Serial.print(((int)chamber_resource.dht_humidity));
  Serial.println("%");

  Serial.print(F("Dew Point (°C): "));
  chamber_resource.dht_dewpoint = ((float)dht.dewPoint());
  Serial.println(((int)chamber_resource.dht_dewpoint));

  */
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

int convertCtoF(int temp_c)
{
  int temp_f;
  temp_f = ((int)round(1.8 * temp_c + 32));
  return temp_f;
}

/*void printDateTime(const RtcDateTime &dt)
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
             dt.Second());
  Serial.print(datestring);
}*/

/*void create_timestamp(const RtcDateTime &dt)
{
  snprintf_P(timestamp,
             countof(timestamp),
             PSTR("%04u%02u%02u%02u%02u%02u"),
             dt.Year(),
             dt.Month(),
             dt.Day(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(timestamp);
}*/
