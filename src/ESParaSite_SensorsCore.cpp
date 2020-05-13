// ESParaSite_Sensors.cpp

/* ESParasite Data Logger v0.6
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#include <Adafruit_MLX90614.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <BlueDot_BME280.h>
#include <DHT12.h>
#include <EepromAt24C32.h>
#include <RtcDS3231.h>
#include <Time.h>
#include <Wire.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"


#define countof(a) (sizeof(a) / sizeof(a[0]))

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure
// corrected for the sea level Otherwise leave the standard pressure as default
// (1013.25 hPa)

// Also put in the current average temperature outside (yes, really outside!)
// For slightly less precise altitude measurements, just leave the standard
// temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

// These values control the I2C address of each sensor. Some chips may use
// different addresses and it is recommend to utilize the I2C scanner sketch at:
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

#ifndef DEBUG_L2
bool dump_sensor_data = 0;
#endif
#ifdef DEBUG_L2
bool dump_sensor_data = 1;
#endif

int bme_i2c_address;
int eeprom_i2c_address;




extern ESParaSite::printchamber chamber_resource;
extern ESParaSite::optics optics_resource;
extern ESParaSite::ambient ambient_resource;
extern ESParaSite::enclosure enclosure_resource;
extern ESParaSite::status_data status_resource;
extern ESParaSite::config_data config_resource;
extern ESParaSite::sensor_exists exists_resource;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> rtc(Wire);
BlueDot_BME280 bme;
DHT12 dht;



// Since we use libraries by different authors and not all libraries talk to
// sensors in the same way, init_i2c_sensors and ping_sensor give us a cleaner,
// abstracted format to check for the sensors before calling the
// init_xyz_sensor() methods which are unique to the sensor and library chosen.
// This will allow us to extend sensor support over time in a more elegant
// fashion.

void ESParaSite::Sensors::init_i2c_sensors() {
  // initialize I2C bus
  Serial.print(F("Init I2C bus..."));
  Wire.begin(config_resource.cfg_pin_sda, config_resource.cfg_pin_scl);
  Serial.println(F("\t\t\t\t\tOK!"));
  Serial.println();

  // initialize Print Chamber sensor
  Serial.println(F("Init Print Chamber Sensor..."));
  int error = ping_sensor(DHT_ADDR);
  if (error == 0) {
    Sensors::init_dht_sensor();
  } else {
    exists_resource.dhtDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize UV Light sensor
  Serial.println(F("Init UV Light sensor..."));
  error = ping_sensor(SI_ADDR);
  if (error == 0) {
    Sensors::init_si_sensor();
  } else {
    exists_resource.siDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Non-Contact temperature sensor
  Serial.println(F("Init Non-Contact temperature sensor..."));
  error = ping_sensor(MLX_ADDR);
  if (error == 0) {
    Sensors::init_mlx_sensor();
  } else {
    exists_resource.mlxDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Ambient temperature sensor
  Serial.println(F("Init BME280 sensor..."));
  error = ping_sensor(BME_ADDR_A);
  if (error == 0) {
    bme_i2c_address = (BME_ADDR_A);
    Sensors::init_bme_sensor();
  } else {
    error = ping_sensor(BME_ADDR_B);
    if (error == 0) {
      bme_i2c_address = (BME_ADDR_B);
      Sensors::init_bme_sensor();
    } else {
      exists_resource.bmeDetected = 0;
    }
  }

  Serial.println();
  Serial.println();

  // initialize DS3231 RTC
  Serial.println(F("Init DS3231 RTC..."));
  error = ping_sensor(RTC_ADDR);
  if (error == 0) {
    Serial.println(F("OK!"));
    init_rtc_clock();
  } else {
    Serial.println(F("No RTC Found. ESParaSite requires an RTC with EEPROM. "
                     "Launching config portal"));
    ESParaSite::ConfigPortal::do_config_portal();
  }

  Serial.println();
  Serial.println();

  // initialize AR24C32 EEPROM
  Serial.println(F("Init AT24C32 EEPROM..."));

  for (eeprom_i2c_address = RTC_EEPROM_BASE_ADDR;
       eeprom_i2c_address <= RTC_EEPROM_MAX_ADDR; eeprom_i2c_address++) {
    error = ping_sensor(eeprom_i2c_address);
    if (error == 0) {
      Serial.println(F("OK!"));
      Serial.println();
      RtcEeprom::init_rtc_eeprom();
      return;
    }
    Serial.println();
  }
  Serial.println(F("NO EEPROM FOUND!"));
  Serial.println();
}

// This gives us a nicely formatted dump of all sensor data to Serial console.
void ESParaSite::Sensors::dump_sensors(bool in_seq_read) {
  dump_sensor_data = 1;
  Serial.println();
  Serial.println(F("Current Sensor Readings"));
  Serial.println(
      F("============================================================"));
  Serial.println();

  Serial.println(F("DS3231 Real-Time Clock Timestamp and Temperature:"));
  read_rtc_data();
  Serial.println();

  Serial.println(F("DHT12 Print Chamber Environmental Data:"));
  if (in_seq_read == true){
  read_dht_sensor(true);
  } else {
    read_dht_sensor(false);
  }
  Serial.println();

  Serial.println(F("SI1145 UV and Light Sensor Data:"));
  read_si_sensor();
  Serial.println();

  Serial.println(F("MLX90614 LED Temp Sensor Data:"));
  read_mlx_sensor();
  Serial.println();

  Serial.println(F("BME280 Ambient Temp Sensor Data:"));
  read_bme_sensor();
  Serial.println();

  Serial.print(F("This Printer has been on for:\t"));
  Serial.print(enclosure_resource.life_sec);
  Serial.println(F("  seconds"));
  Serial.println();

#ifndef DEBUG_L2
  dump_sensor_data = 0;
#endif
}

void ESParaSite::Sensors::init_dht_sensor() {
  int status = dht.read();
  switch (status) {
  case DHT12_OK:
    Serial.print(F("OK!\t"));
    exists_resource.dhtDetected = 1;
    break;
  case DHT12_ERROR_CHECKSUM:
    Serial.print(F("DHT12 Checksum error,\t"));
    break;
  case DHT12_ERROR_CONNECT:
    Serial.print(F("DHT12 Connect error,\t"));
    break;
  case DHT12_MISSING_BYTES:
    Serial.print(F("DHT12 Missing bytes,\t"));
    break;
  default:
    Serial.print(F("DHT12 Unknown error,\t"));
    break;
  }
}

void ESParaSite::Sensors::init_si_sensor() {
  if (!uv.begin()) {
    Serial.print(F("SI1145 Initialization Failure!"));
  } else {
    Serial.print(F("OK!"));
    exists_resource.siDetected = 1;
  }
}

void ESParaSite::Sensors::init_mlx_sensor() {
  if (!mlx.begin()) {
    Serial.print(F("MLX90614 Initialization Failure"));
  } else {
    Serial.print(F("OK!"));
    exists_resource.mlxDetected = 1;
  }
}

void ESParaSite::Sensors::init_bme_sensor() {
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

  if (bme.init() != 0x60) {
    Serial.print(F("BME280 Sensor not found!"));
  } else {
    Serial.print(F("OK!"));
    exists_resource.bmeDetected = 1;
  }
}

void ESParaSite::Sensors::init_rtc_clock() {
  rtc.Begin();

  Serial.println();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Util::printDateTime(compiled);
  Serial.println();

  if (!rtc.IsDateTimeValid()) {
    if (rtc.LastError() != 0) {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print(F("RTC communications error = "));
      Serial.println(rtc.LastError());
    } else {
      // Common Cuases:
      //  1) first time you ran and the device wasn't running yet
      //  2) the battery on the device is low or even missing

      Serial.println(F("RTC lost confidence in the DateTime!"));

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      rtc.SetDateTime(compiled);
    }
  }

  if (!rtc.GetIsRunning()) {
    Serial.println(F("RTC was not actively running, starting now"));
    rtc.SetIsRunning(true);
  }

  RtcDateTime now = rtc.GetDateTime();
  if (now < compiled) {
    Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
    rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println(F("RTC is newer than compile time. (this is expected)"));
  } else if (now == compiled) {
    Serial.println(
        F("RTC is the same as compile time! (not expected but all is fine)"));
  }

  Serial.println();

  status_resource.rtc_current_second = (rtc.GetDateTime() + 946684800);

  Serial.println();
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void ESParaSite::Sensors::read_dht_sensor(bool in_seq_read) {
  // First dht measurement is stale, so if in_seq_read is 'false' (when read is
  // out-of-sequence or, in other words, not called by a timer in the loop() ),
  // we read, wait 2 seconds, then read again. Normal in sequence reads do not
  // need this, so we send a 'true' to skip the additional delay.
  if (exists_resource.dhtDetected == 1) {
    int status = dht.read();

    if (!in_seq_read) {
      switch (status) {
      case DHT12_OK:
        break;
      case DHT12_ERROR_CHECKSUM:
        Serial.print(F("Checksum error,\t"));
        break;
      case DHT12_ERROR_CONNECT:
        Serial.print(F("Connect error,\t"));
        break;
      case DHT12_MISSING_BYTES:
        Serial.print(F("Missing bytes,\t"));
        break;
      default:
        Serial.print(F("Unknown error,\t"));
        break;
      }
      delay(2000);
      status = dht.read();
    }

    switch (status) {
    case DHT12_OK:
      chamber_resource.dht_temp_c = dht.temperature;
      chamber_resource.dht_humidity = dht.humidity;
      chamber_resource.dht_dewpoint = (Util::dewPoint(
          chamber_resource.dht_temp_c, chamber_resource.dht_humidity));

      if (dump_sensor_data == 1) {
        Serial.println(F("==========Print Chamber=========="));
        Serial.print(F("Temperature:\t\t\t"));
        Serial.print(chamber_resource.dht_temp_c, 1);
        Serial.print("°C / ");
        Serial.print(Util::convertCtoF(chamber_resource.dht_temp_c));
        Serial.println("°F");
        Serial.print(F("Humidity:\t\t\t"));
        Serial.print(chamber_resource.dht_humidity, 1);
        Serial.println("%");
        Serial.print(F("Dew Point:\t\t\t"));
        Serial.print(static_cast<int>(chamber_resource.dht_dewpoint));
        Serial.print("°C / ");
        Serial.print(Util::convertCtoF(chamber_resource.dht_dewpoint));
        Serial.println("°F");
      }
      break;
    case DHT12_ERROR_CHECKSUM:
      Serial.print(F("DHT12 Checksum error,\t"));
      break;
    case DHT12_ERROR_CONNECT:
      Serial.print(F("DHT12 Connect error,\t"));
      break;
    case DHT12_MISSING_BYTES:
      Serial.print(F("DHT12 Missing bytes,\t"));
      break;
    default:
      Serial.print(F("DHT12 Unknown error,\t"));
      break;
    }

  } else {
    chamber_resource.dht_temp_c = 0;
    chamber_resource.dht_humidity = 0;
    chamber_resource.dht_dewpoint = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("DHT12 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::read_si_sensor() {
  if (exists_resource.siDetected == 1) {
    optics_resource.si_uvindex = uv.readUV();
    optics_resource.si_uvindex /= 100.0;
    optics_resource.si_visible = uv.readVisible();
    optics_resource.si_infrared = uv.readIR();

    if (dump_sensor_data == 1) {
      Serial.println(F("==========LED Light Sensor=========="));
      Serial.print(F("UV Index:\t\t\t"));
      Serial.println(static_cast<int>(optics_resource.si_uvindex));
      Serial.print(F("Visible:\t\t\t"));
      Serial.println(optics_resource.si_visible);
      Serial.print(F("Infrared:\t\t\t"));
      Serial.println(optics_resource.si_infrared);
    }

  } else {
    optics_resource.si_uvindex = 0;
    optics_resource.si_visible = 0;
    optics_resource.si_infrared = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("SI1145 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::read_mlx_sensor() {
  if (exists_resource.mlxDetected == 1) {
    optics_resource.mlx_amb_temp_c = mlx.readAmbientTempC();
    optics_resource.mlx_obj_temp_c = mlx.readObjectTempC();

    if (dump_sensor_data == 1) {
      Serial.println("==========LCD Temp Sensor==========");
      Serial.print("Ambient:\t\t\t");
      Serial.print(optics_resource.mlx_amb_temp_c);
      Serial.print("°C / ");
      Serial.print(mlx.readAmbientTempF());
      Serial.println("°F");
      Serial.print("LCD Panel:\t\t\t");
      Serial.print(optics_resource.mlx_obj_temp_c);
      Serial.print("°C / ");
      Serial.print(mlx.readObjectTempF());
      Serial.println("°F");
    }

  } else {
    optics_resource.mlx_amb_temp_c = 0;
    optics_resource.mlx_obj_temp_c = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("MLX90614 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::read_bme_sensor() {
  if (exists_resource.bmeDetected == 1) {
    ambient_resource.bme_temp_c = bme.readTempC();
    ambient_resource.bme_humidity = bme.readHumidity();
    ambient_resource.bme_barometer = bme.readPressure();
    ambient_resource.bme_altitude = bme.readAltitudeMeter();

    if (dump_sensor_data == 1) {
      Serial.println("==========Ambient Conditions==========");
      Serial.print(F("Temperature:\t\t\t"));
      Serial.print(ambient_resource.bme_temp_c);
      Serial.print("°C / ");
      Serial.print(bme.readTempF());
      Serial.println("°F");
      Serial.print(F("Relative Humidity:\t\t"));
      Serial.print(ambient_resource.bme_humidity);
      Serial.println("%");
      Serial.print(F("Barometric Pressure:\t\t"));
      Serial.print(ambient_resource.bme_barometer);
      Serial.println(" hPa");
      Serial.print(F("Altitude:\t\t\t"));
      Serial.print(ambient_resource.bme_altitude);
      Serial.print("m / ");
      Serial.print(bme.readAltitudeFeet());
      Serial.println("ft");
    }

  } else {
    ambient_resource.bme_temp_c = 0;
    ambient_resource.bme_humidity = 0;
    ambient_resource.bme_barometer = 0;
    ambient_resource.bme_altitude = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("BME280 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::read_rtc_data() {
  ESParaSite::Sensors::check_rtc_status();
  RtcDateTime now = rtc.GetDateTime();
  // Epoch64 Conversion
  status_resource.rtc_current_second = (rtc.GetDateTime() + 946684800);

  if (dump_sensor_data == 1) {
    Serial.println("==========Real Time Clock==========");
    Util::printDateTime(now);
    Serial.println();
    Serial.print("Epoch64:\t\t\t");
    Serial.println(status_resource.rtc_current_second);
  }
  ESParaSite::Sensors::read_rtc_temp();
}

void ESParaSite::Sensors::check_rtc_status() {
  if (!rtc.IsDateTimeValid()) {
    if (rtc.LastError() != 0) {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print(F("RTC communications error = "));
      Serial.println(rtc.LastError());
      Serial.println(F("RTC not responding...Restarting"));
      ESP.restart();
    } else {
      // Common Cuases:
      //  1) the battery on the device is low or even missing and the power
      //  line was disconnected
      Serial.println(F("RTC lost confidence in the DateTime!"));
    }
  }
}

  void ESParaSite::Sensors::read_rtc_temp() {
    RtcTemperature temp = (rtc.GetTemperature());
    enclosure_resource.case_temp = (temp.AsFloatDegC());

    if (dump_sensor_data == 1) {
      Serial.println("==========Case Temperature=========");
      Serial.print(F("Case Temperature:\t\t"));
      temp.Print(Serial);
      Serial.print("°C / ");
      Serial.print(Util::convertCtoF(enclosure_resource.case_temp));
      Serial.println("°F");
    }
  }

  time_t ESParaSite::Sensors::read_rtc_epoch() {
    ESParaSite::Sensors::check_rtc_status();
    time_t rtc_return = (rtc.GetDateTime() + 946684800);
    return rtc_return;
  }


  int ESParaSite::Sensors::ping_sensor(uint16_t address) {
    // This function provides us a handy way to ping I2C addresses.
    byte error;
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.print(F("\t\t"));
    } else if (error == 2) {
      Serial.print(F("No device (Ping recieved NACK) at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.print(F("\t\t"));
    } else if (error == 4) {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.print(F("\t\t"));
    }
    return error;
  }