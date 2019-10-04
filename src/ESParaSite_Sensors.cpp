// ESParaSite_Sensors.cpp

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
#include <BlueDot_BME280.h>
#include <dht12.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>
#include <RtcDS3231.h>
#include <EepromAt24C32.h>

#include "ESParaSite_Core.h"
#include "ESParaSite_Rest.h"
#include "ESParaSite_Eeprom.h"
#include "ESParaSite_Sensors.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure corrected for the sea level
// Otherwise leave the standard pressure as default (1013.25 hPa)

// Also put in the current average temperature outside (yes, really outside!)
// For slightly less precise altitude measurements, just leave the standard temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

// These values control the I2C address of each sensor. Some chips may use different addresses and it is recommend
// to utilize the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// DHT12 Sensor Address. Default (0x5C)
#define DHT_ADDR (0x5C)
// SI1145 Sensor Address. Default (0x60)
#define SI_ADDR (0x60)
// MLX90614 Sensor Address. Default (0x5A)
#define MLX_ADDR (0x5A)
// BME280 Sensor Address. Default (0x76) or (0x77)
#define BME_ADDR_A (0x76)
#define BME_ADDR_B (0x77)
// DS3231 Real Time Clock Address. Default (0x68)
#define RTC_ADDR (0x68)
// AT24C32 EEPROM Address. Default (0x50 to 0x57)
#define RTC_EEPROM_BASE_ADDR (0x50)
#define RTC_EEPROM_MAX_ADDR (0x57)

//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

int bme_i2c_address;
int eeprom_i2c_address;
int bmeDetected = 0;
int dhtDetected = 0;
int mlxDetected = 0;
int rtcDetected = 0;
int siDetected = 0;

extern config_data config_resource;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> rtc(Wire);
BlueDot_BME280 bme;
DHT12 dht;

// Since we use libraries by different authors, not all libraries talk to sensors in the same way.
// init_i2c_sensors and ping_sensor give us a cleaner, abstracted format to check fo rthe sensors before calling the
// init_xyz_sensor() methods which are unique to the sensor and library chosen.
// This will allow us to extend sensor support over time in a more elegant fashion.

void init_i2c_sensors()
{
  // initialize I2C bus
  Serial.print("Init I2C bus...");
  Wire.begin(config_resource.cfg_pin_sda, config_resource.cfg_pin_scl);
  Serial.println("\t\t\t\t\t\t\tOK!");
  Serial.println();

  // initialize Print Chamber sensor
  Serial.print("Init Print Chamber Sensor...");
  int error = ping_sensor(DHT_ADDR);
  if (error == 0)
  {
    init_dht_sensor();
  }
  Serial.println();

  // initialize UV Light sensor
  Serial.print("Init UV Light sensor...");
  error = ping_sensor(SI_ADDR);
  if (error == 0)
  {
    init_si_sensor();
  }
  Serial.println();

  // initialize Non-Contact temperature sensor
  Serial.print("Init Non-Contact temperature sensor...");
  error = ping_sensor(MLX_ADDR);
  if (error == 0)
  {
    init_mlx_sensor();
  }
  Serial.println();

  // initialize BME280 temperature sensor
  Serial.print("Init BME280 sensor...");
  error = ping_sensor(BME_ADDR_A);
  if (error == 0)
  {
    bme_i2c_address = (BME_ADDR_A);
    init_bme_sensor();
  }
  else
  {
    error = ping_sensor(BME_ADDR_B);
    if (error == 0)
    {
      bme_i2c_address = (BME_ADDR_B);
      init_bme_sensor();
    }
  }
  Serial.println();

  // initialize DS3231 RTC
  Serial.print("Init DS3231 RTC...");
  error = ping_sensor(RTC_ADDR);
  if (error == 0)
  {
    Serial.println("OK!");
    init_rtc_clock();
  }

  // initialize AR24C32 EEPROM
  Serial.println("Init AT24C32 EEPROM...");

  for (eeprom_i2c_address = RTC_EEPROM_BASE_ADDR; eeprom_i2c_address <= RTC_EEPROM_MAX_ADDR; eeprom_i2c_address++)
  {
    error = ping_sensor(eeprom_i2c_address);
    if (error == 0)
    {
      Serial.println("OK!");
      Serial.println();
      init_rtc_eeprom();
      return;
    }
    Serial.println();
  }
  Serial.println("NO EEPROM FOUND!");
  Serial.println();
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

// This gives us a nicely formatted dump of all sensor data to Serial console.
void dump_sensors()
{
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

  // Comment this line out for Production
}

void init_dht_sensor()
{
  int status = dht.read();
  switch (status)
  {
  case DHT12_OK:
    Serial.print("OK!\t");
    dhtDetected = 1;
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

void init_si_sensor()
{
  if (!uv.begin())
  {
    Serial.print("SI1145 Initialization Failure!");
  }
  else
  {
    siDetected = 1;
    Serial.print("OK!");
  }
}

void init_mlx_sensor()
{
  if (!mlx.begin())
  {
    Serial.print("MLX90614 Initialization Failure");
  }
  else
  {
    mlxDetected = 1;
    Serial.print("OK!");
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
    Serial.print(F("BME280 Sensor not found!"));
  }
  else
  {
    Serial.print(F("OK!"));
    bmeDetected = 1;
  }
}

void init_rtc_clock()
{
  rtc.Begin();

  Serial.println("");
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println("");

  if (!rtc.IsDateTimeValid())
  {
    if (rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(rtc.LastError());
    }
    else
    {
      // Common Cuases:
      //  1) first time you ran and the device wasn't running yet
      //  2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      rtc.SetDateTime(compiled);
    }
  }

  if (!rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    rtc.SetIsRunning(true);
  }

  RtcDateTime now = rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  Serial.println("");

  status_resource.rtc_current_second = (rtc.GetDateTime() + 946684800);

  Serial.println("");
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void read_dht_sensor()
{
  Serial.println("==========Print Chamber==========");

  // First dht measurement is stale, so we measure, wait ~2 seconds, then measure again.
  if (dhtDetected == 1)
  {
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

    delay(2000);

    status = dht.read();
    switch (status)
    {
    case DHT12_OK:
      Serial.print(F("Temperature:\t\t\t"));
      chamber_resource.dht_temp_c = dht.temperature;
      Serial.print(chamber_resource.dht_temp_c, 1);
      Serial.print("°C / ");
      Serial.print(convertCtoF(chamber_resource.dht_temp_c));
      Serial.println("°F");

      Serial.print(F("Humidity:\t\t\t"));
      chamber_resource.dht_humidity = dht.humidity;
      Serial.print(chamber_resource.dht_humidity, 1);
      Serial.println("%");

      Serial.print(F("Dew Point:\t\t\t"));
      chamber_resource.dht_dewpoint = (dewPoint(chamber_resource.dht_temp_c, chamber_resource.dht_humidity));
      Serial.print(static_cast<int>(chamber_resource.dht_dewpoint));
      Serial.print("°C / ");
      Serial.print(convertCtoF(chamber_resource.dht_dewpoint));
      Serial.println("°F");

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
  else
  {
    Serial.print("Error: DHT12 Sensor not found");
    chamber_resource.dht_temp_c = 0;
    chamber_resource.dht_humidity = 0;
    chamber_resource.dht_dewpoint = 0;
  }
}

void read_si_sensor()
{
  Serial.println("==========LED Light Sensor==========");

  optics_resource.si_uvindex = uv.readUV();
  optics_resource.si_uvindex /= 100.0;
  Serial.print("UV Index:\t\t\t");
  Serial.println(static_cast<int>(optics_resource.si_uvindex));

  optics_resource.si_visible = uv.readVisible();
  Serial.print("Visible:\t\t\t");
  Serial.println(optics_resource.si_visible);

  optics_resource.si_infrared = uv.readIR();
  Serial.print("Infrared:\t\t\t");
  Serial.println(optics_resource.si_infrared);
}

void read_mlx_sensor()
{
  Serial.println("==========LCD Temp Sensor==========");

  optics_resource.mlx_amb_temp_c = mlx.readAmbientTempC();
  Serial.print("Ambient:\t\t\t");
  Serial.print(optics_resource.mlx_amb_temp_c);
  Serial.print("°C / ");
  Serial.print(mlx.readAmbientTempF());
  Serial.println("°F");

  optics_resource.mlx_obj_temp_c = mlx.readObjectTempC();
  Serial.print("LCD Panel:\t\t\t");
  Serial.print(optics_resource.mlx_obj_temp_c);
  Serial.print("°C / ");
  Serial.print(mlx.readObjectTempF());
  Serial.println("°F");
}

void read_bme_sensor()
{
  Serial.println("==========Ambient Conditions==========");

  if (siDetected == 1)
  {
    ambient_resource.bme_temp_c = bme.readTempC();
    Serial.print(F("Temperature:\t\t\t"));
    Serial.print(ambient_resource.bme_temp_c);
    Serial.print("°C / ");
    Serial.print(bme.readTempF());
    Serial.println("°F");

    ambient_resource.bme_humidity = bme.readHumidity();
    Serial.print(F("Relative Humidity:\t\t"));
    Serial.print(ambient_resource.bme_humidity);
    Serial.println("%");

    ambient_resource.bme_barometer = bme.readPressure();
    Serial.print(F("Barometric Pressure:\t\t"));
    Serial.print(ambient_resource.bme_barometer);
    Serial.println(" hPa");

    ambient_resource.bme_altitude = bme.readAltitudeMeter();
    Serial.print(F("Altitude:\t\t\t"));
    Serial.print(ambient_resource.bme_altitude);
    Serial.print("m / ");
    Serial.print(bme.readAltitudeFeet());
    Serial.println("ft");
  }
  else
  {
    ambient_resource.bme_temp_c = 0;
    ambient_resource.bme_humidity = 0;
    ambient_resource.bme_barometer = 0;
    ambient_resource.bme_altitude = 0;
  }
}

void read_rtc_data()
{
  Serial.println("==========Real Time Clock==========");
  if (!rtc.IsDateTimeValid())
  {
    if (rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(rtc.LastError());
    }
    else
    {
      // Common Cuases:
      //  1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }

  RtcDateTime now = rtc.GetDateTime();
  printDateTime(now);
  Serial.println();

  // Epoch64 Conversion
  status_resource.rtc_current_second = (rtc.GetDateTime() + 946684800);
  Serial.print("Epoch64:\t\t\t");
  Serial.println(status_resource.rtc_current_second);

  //
  Serial.print(F("Case Temperature:\t\t"));
  RtcTemperature temp = (rtc.GetTemperature());
  enclosure_resource.case_temp = (temp.AsFloatDegC());
  temp.Print(Serial);
  Serial.print("°C / ");
  Serial.print(convertCtoF(enclosure_resource.case_temp));
  Serial.println("°F");
}

void read_rtc_temp()
{
  Serial.println("==========Case Temperature=========");
  if (rtc.LastError() != 0)
  {
    Serial.print("RTC communications error = ");
    Serial.println(rtc.LastError());
    enclosure_resource.case_temp = (0);
  }
  else
  {
    RtcTemperature temp = (rtc.GetTemperature());
    Serial.print(F("Temperature:\t\t\t"));
    enclosure_resource.case_temp = (temp.AsFloatDegC());
    temp.Print(Serial);
    Serial.print("°C / ");
    Serial.print(convertCtoF(enclosure_resource.case_temp));
    Serial.println("°F");
  }
}

time_t read_rtc_epoch()
{
  if (!rtc.IsDateTimeValid())
  {
    if (rtc.LastError() != 0)
    {
      Serial.print("RTC communications error = ");
      Serial.println(rtc.LastError());
    }
    else
    {
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }

  time_t rtc_return = (rtc.GetDateTime() + 946684800);

  return rtc_return;
}

void printDateTime(const RtcDateTime &dt)
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
}
