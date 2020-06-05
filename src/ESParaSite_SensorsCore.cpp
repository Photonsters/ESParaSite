// ESParaSite_SensorsCore.cpp

/* ESParasite Data Logger v0.9
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

extern ESParaSite::chamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::sensorExists existsResource;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> rtc(Wire);
BlueDot_BME280 bme;
DHT12 dht;

// Since we use libraries by different authors and not all libraries talk to
// sensors in the same way, initI2cSensors and pingSensor give us a cleaner,
// abstracted format to check for the sensors before calling the
// init_xyz_sensor() methods which are unique to the sensor and library chosen.
// This will allow us to extend sensor support over time in a more elegant
// fashion.

void ESParaSite::Sensors::initI2cSensors() {
  // initialize I2C bus
  Serial.print(F("Init I2C bus..."));
  Wire.begin(configResource.cfgPinSda, configResource.cfgPinScl);
  Serial.println(F("\t\t\t\t\tOK!"));
  Serial.println();

  // initialize Print Chamber sensor
  Serial.println(F("Init Print Chamber Sensor..."));
  int error = pingSensor(DHT_ADDR);
  if (error == 0) {
    Sensors::initDhtSensor();
  } else {
    existsResource.dhtDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize UV Light sensor
  Serial.println(F("Init UV Light sensor..."));
  error = pingSensor(SI_ADDR);
  if (error == 0) {
    Sensors::initSiSensor();
  } else {
    existsResource.siDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Non-Contact temperature sensor
  Serial.println(F("Init Non-Contact temperature sensor..."));
  error = pingSensor(MLX_ADDR);
  if (error == 0) {
    Sensors::initMlxSensor();
  } else {
    existsResource.mlxDetected = 0;
  }

  Serial.println();
  Serial.println();

  // initialize Ambient temperature sensor
  Serial.println(F("Init BME280 sensor..."));
  error = pingSensor(BME_ADDR_A);
  if (error == 0) {
    bme_i2c_address = (BME_ADDR_A);
    Sensors::initBmeSensor();
  } else {
    error = pingSensor(BME_ADDR_B);
    if (error == 0) {
      bme_i2c_address = (BME_ADDR_B);
      Sensors::initBmeSensor();
    } else {
      existsResource.bmeDetected = 0;
    }
  }

  Serial.println();
  Serial.println();

  // initialize DS3231 RTC
  Serial.println(F("Init DS3231 RTC..."));
  error = pingSensor(RTC_ADDR);
  if (error == 0) {
    Serial.println(F("OK!"));
    initRtcClock();
  } else {
    Serial.println(F("No RTC Found. ESParaSite requires an RTC with EEPROM. "
                     "Launching config portal"));
    ESParaSite::ConfigPortal::doConfigPortal();
  }

  Serial.println();
  Serial.println();

  // initialize AR24C32 EEPROM
  Serial.println(F("Init AT24C32 EEPROM..."));

  for (eeprom_i2c_address = RTC_EEPROM_BASE_ADDR;
       eeprom_i2c_address <= RTC_EEPROM_MAX_ADDR; eeprom_i2c_address++) {
    error = pingSensor(eeprom_i2c_address);
    if (error == 0) {
      Serial.println(F("OK!"));
      Serial.println();
      RtcEeprom::initRtcEeprom();
      return;
    }
    Serial.println();
  }
  Serial.println(F("NO EEPROM FOUND!"));
  Serial.println();
}

// This gives us a nicely formatted dump of all sensor data to Serial console.
void ESParaSite::Sensors::dumpSensor(bool in_seq_read) {
  dump_sensor_data = 1;
  Serial.println();
  Serial.println(F("Current Sensor Readings"));
  Serial.println(
      F("============================================================"));
  Serial.println();

  Serial.println(F("DS3231 Real-Time Clock Timestamp and Temperature:"));
  readRtcData();
  Serial.println();

  Serial.println(F("DHT12 Print Chamber Environmental Data:"));
  if (in_seq_read == true) {
    readDhtSensor(true);
  } else {
    readDhtSensor(false);
  }
  Serial.println();

  Serial.println(F("SI1145 UV and Light Sensor Data:"));
  readSiSensor();
  Serial.println();

  Serial.println(F("MLX90614 LED Temp Sensor Data:"));
  readMlxSensor();
  Serial.println();

  Serial.println(F("BME280 Ambient Temp Sensor Data:"));
  readBmeSensor();
  Serial.println();

  Serial.print(F("This Printer has been on for:\t"));
  Serial.print(enclosureResource.printerLifeSec);
  Serial.println(F("  seconds"));
  Serial.println();

#ifndef DEBUG_L2
  dump_sensor_data = 0;
#endif
}

void ESParaSite::Sensors::initDhtSensor() {
  int status = dht.read();
  switch (status) {
  case DHT12_OK:
    Serial.print(F("OK!\t"));
    existsResource.dhtDetected = 1;
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

void ESParaSite::Sensors::initSiSensor() {
  if (!uv.begin()) {
    Serial.print(F("SI1145 Initialization Failure!"));
  } else {
    Serial.print(F("OK!"));
    existsResource.siDetected = 1;
  }
}

void ESParaSite::Sensors::initMlxSensor() {
  if (!mlx.begin()) {
    Serial.print(F("MLX90614 Initialization Failure"));
  } else {
    Serial.print(F("OK!"));
    existsResource.mlxDetected = 1;
  }
}

void ESParaSite::Sensors::initBmeSensor() {
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
    existsResource.bmeDetected = 1;
  }
}

void ESParaSite::Sensors::initRtcClock() {
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

  statusResource.rtcCurrentSecond = (rtc.GetDateTime() + 946684800);

  Serial.println();
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void ESParaSite::Sensors::readDhtSensor(bool in_seq_read) {
  // First dht measurement is stale, so if in_seq_read is 'false' (when read is
  // out-of-sequence or, in other words, not called by a timer in the loop() ),
  // we read, wait 2 seconds, then read again. Normal in sequence reads do not
  // need this, so we send a 'true' to skip the additional delay.
  if (existsResource.dhtDetected == 1) {
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
      chamberResource.chamberTempC = dht.temperature;
      chamberResource.chamberHumidity = dht.humidity;
      chamberResource.chamberDewPoint = (Util::dewPoint(
          chamberResource.chamberTempC, chamberResource.chamberHumidity));

      if (dump_sensor_data == 1) {
        Serial.println(F("==========Print Chamber=========="));
        Serial.print(F("Temperature:\t\t\t"));
        Serial.print(chamberResource.chamberTempC, 1);
        Serial.print("°C / ");
        Serial.print(Util::convertCtoF(chamberResource.chamberTempC));
        Serial.println("°F");
        Serial.print(F("Humidity:\t\t\t"));
        Serial.print(chamberResource.chamberHumidity, 1);
        Serial.println("%");
        Serial.print(F("Dew Point:\t\t\t"));
        Serial.print(static_cast<int>(chamberResource.chamberDewPoint));
        Serial.print("°C / ");
        Serial.print(Util::convertCtoF(chamberResource.chamberDewPoint));
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
    chamberResource.chamberTempC = 0;
    chamberResource.chamberHumidity = 0;
    chamberResource.chamberDewPoint = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("DHT12 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::readSiSensor() {
  if (existsResource.siDetected == 1) {
    opticsResource.ledUVIndex = uv.readUV();
    opticsResource.ledUVIndex /= 100.0;
    opticsResource.ledVisible = uv.readVisible();
    opticsResource.ledInfrared = uv.readIR();

    if (dump_sensor_data == 1) {
      Serial.println(F("==========LED Light Sensor=========="));
      Serial.print(F("UV Index:\t\t\t"));
      Serial.println(static_cast<int>(opticsResource.ledUVIndex));
      Serial.print(F("Visible:\t\t\t"));
      Serial.println(opticsResource.ledVisible);
      Serial.print(F("Infrared:\t\t\t"));
      Serial.println(opticsResource.ledInfrared);
    }

  } else {
    opticsResource.ledUVIndex = 0;
    opticsResource.ledVisible = 0;
    opticsResource.ledInfrared = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("SI1145 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::readMlxSensor() {
  if (existsResource.mlxDetected == 1) {
    opticsResource.ledTempC = roundf(mlx.readAmbientTempC() *100) / 100;
    opticsResource.screenTempC = roundf(mlx.readObjectTempC() * 100) / 100;

    if (dump_sensor_data == 1) {
      Serial.println("==========LCD Temp Sensor==========");
      Serial.print("Ambient:\t\t\t");
      Serial.print(opticsResource.ledTempC);
      Serial.print("°C / ");
      Serial.print(mlx.readAmbientTempF());
      Serial.println("°F");
      Serial.print("LCD Panel:\t\t\t");
      Serial.print(opticsResource.screenTempC);
      Serial.print("°C / ");
      Serial.print(mlx.readObjectTempF());
      Serial.println("°F");
    }

  } else {
    opticsResource.ledTempC = 0;
    opticsResource.screenTempC = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("MLX90614 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::readBmeSensor() {
  if (existsResource.bmeDetected == 1) {
    ambientResource.ambientTempC = roundf(bme.readTempC() * 100) / 100;
    ambientResource.ambientHumidity = roundf(bme.readHumidity() * 100) / 100;
    ambientResource.ambientBarometer = bme.readPressure();
    ambientResource.ambientAltitude = bme.readAltitudeMeter();

    if (dump_sensor_data == 1) {
      Serial.println("==========Ambient Conditions==========");
      Serial.print(F("Temperature:\t\t\t"));
      Serial.print(ambientResource.ambientTempC);
      Serial.print("°C / ");
      Serial.print(bme.readTempF());
      Serial.println("°F");
      Serial.print(F("Relative Humidity:\t\t"));
      Serial.print(ambientResource.ambientHumidity);
      Serial.println("%");
      Serial.print(F("Barometric Pressure:\t\t"));
      Serial.print(ambientResource.ambientBarometer);
      Serial.println(" hPa");
      Serial.print(F("Altitude:\t\t\t"));
      Serial.print(ambientResource.ambientAltitude);
      Serial.print("m / ");
      Serial.print(bme.readAltitudeFeet());
      Serial.println("ft");
    }

  } else {
    ambientResource.ambientTempC = 0;
    ambientResource.ambientHumidity = 0;
    ambientResource.ambientBarometer = 0;
    ambientResource.ambientAltitude = 0;

    if (dump_sensor_data == 1) {
      Serial.print(F("BME280 Sensor not found"));
    }
  }
}

void ESParaSite::Sensors::readRtcData() {
  ESParaSite::Sensors::checkRtcStatus();
  RtcDateTime now = rtc.GetDateTime();
  // Epoch64 Conversion
  statusResource.rtcCurrentSecond = (rtc.GetDateTime() + 946684800);

  if (dump_sensor_data == 1) {
    Serial.println("==========Real Time Clock==========");
    Util::printDateTime(now);
    Serial.println();
    Serial.print("Epoch64:\t\t\t");
    Serial.println(statusResource.rtcCurrentSecond);
  }
  ESParaSite::Sensors::readRtcTemp();
}

void ESParaSite::Sensors::checkRtcStatus() {
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

void ESParaSite::Sensors::readRtcTemp() {
  RtcTemperature temp = (rtc.GetTemperature());
  enclosureResource.caseTempC = (temp.AsFloatDegC());

  if (dump_sensor_data == 1) {
    Serial.println("==========Case Temperature=========");
    Serial.print(F("Case Temperature:\t\t"));
    temp.Print(Serial);
    Serial.print("°C / ");
    Serial.print(Util::convertCtoF(enclosureResource.caseTempC));
    Serial.println("°F");
  }
}

time_t ESParaSite::Sensors::readRtcEpoch() {
  ESParaSite::Sensors::checkRtcStatus();
  time_t rtc_return = (rtc.GetDateTime() + 946684800);
  return rtc_return;
}

int ESParaSite::Sensors::pingSensor(uint16_t address) {
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