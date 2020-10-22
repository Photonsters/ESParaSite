// APIHandlerData.cpp

/* ESParasite Data Logger
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
#include <ArduinoJson.h>

#include "ESParaSite.h"
#include "DataDigest.h"
#include "HTTP.h"
#include "API.h"


#ifdef ESP32

#include <WiFi.h>

#else

#include <ESP8266WiFi.h>

#endif

extern ESParaSite::chamberData chamber;
extern ESParaSite::opticsData optics;
extern ESParaSite::ambientData ambient;
extern ESParaSite::enclosureData enclosure;
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::eepromData eeprom;
extern ESParaSite::machineData machine;

DynamicJsonDocument ESParaSite::APIHandler::getJsonAmbient() {
  DynamicJsonDocument doc(256);
 
  doc["class"] = "ambient";
  doc["timestamp"] = status.rtcCurrentSecond;
  doc["ambTempC"] = ambient.ambientTempC;
  doc["ambHumid"] = ambient.ambientHumidity;
  doc["ambPres"] = ambient.ambientBarometer;
  doc["ambAlt"] = ambient.ambientAltitude;
  doc["ambDewPt"] = ambient.ambientDewPoint;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonChamber() {
  DynamicJsonDocument doc(256);

  doc["class"] = "chamber";
  doc["timestamp"] = status.rtcCurrentSecond;
  doc["cmbTempC"] = chamber.chamberTempC;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonCurrent() {
  DynamicJsonDocument doc(256);

  doc["ts"] = status.rtcCurrentSecond;
  doc["at"] = ambient.ambientTempC;
  doc["ah"] = ambient.ambientHumidity;
  doc["ct"] = chamber.chamberTempC;
  doc["lt"] = optics.ledTempC;
  doc["st"] = optics.screenTempC;
  doc["lo"] = status.isPrintingFlag;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonEeprom() {
  DynamicJsonDocument doc(256);

  doc["class"] = "eeprom";
  doc["timestamp"] = status.rtcCurrentSecond;
  doc["lastWrtTS"] = eeprom.lastWriteTimestamp;
  doc["firstOnTS"] = eeprom.firstOnTimestamp;
  doc["eprmLedLS"] = eeprom.eepromLedLifeSec;
  doc["eprmScrLS"] = eeprom.eepromScreenLifeSec;
  doc["eprmVatLS"] = eeprom.eepromVatLifeSec;
  doc["lastSegAddr"] = eeprom.lastSegmentAddress;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonEnclosure() {
  DynamicJsonDocument doc(256);

  doc["class"] = "enclosure";
  doc["timestamp"] = status.rtcCurrentSecond;
  doc["caseTempC"] = enclosure.caseTempC;
  doc["printerLS"] = enclosure.printerLifeSec;
  doc["curScrLS"] = enclosure.lcdLifeSec;
  doc["curLedLS"] = enclosure.ledLifeSec;
  doc["curVatLS"] = enclosure.vatLifeSec;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonI2C() {
  DynamicJsonDocument doc(256);

  doc["class"] = "i2c";
  doc["sdaPin"] = config.cfgPinSda;
  doc["sclPin"] = config.cfgPinScl;
  doc["dhtExist"] = machine.dhtDetected;
  doc["bmeExist"] = machine.bmeDetected;
  doc["mlxExist"] = machine.mlxDetected;
  doc["siExist"] = machine.siDetected;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonNetwork() {
  DynamicJsonDocument doc(256);
  String tempArray[5];
  tempArray[0] = WiFi.SSID();
  tempArray[1] = WiFi.RSSI();
  tempArray[2] = WiFi.localIP().toString();
  tempArray[3] = config.cfgMdnsEnabled;
  tempArray[4] = config.cfgMdnsName;

  doc["class"] = "network";
  doc["ssid"] = tempArray[0];
  doc["rssi"] = tempArray[1];
  doc["ipAddr"] = tempArray[2];
  doc["mdnsS"] = tempArray[3];
  doc["mdnsN"] = tempArray[4];

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonOptics() {
  DynamicJsonDocument doc(256);

  doc["class"] = "optics";
  doc["timestamp"] = status.rtcCurrentSecond;
  doc["uvIdx"] = optics.ledUVIndex;
  doc["visLux"] = optics.ledVisible;
  doc["irLux"] = optics.ledInfrared;
  doc["ledTempC"] = optics.ledTempC;
  doc["scrTempC"] = optics.screenTempC;

  return doc;
}

DynamicJsonDocument ESParaSite::APIHandler::getJsonStatus() {
  DynamicJsonDocument doc(256);
  String tempArray[6];
  tempArray[0] = eeprom.lastWriteTimestamp;
  tempArray[1] = enclosure.printerLifeSec;
  tempArray[2] = enclosure.lcdLifeSec;
  tempArray[3] = enclosure.vatLifeSec;
  tempArray[4] = enclosure.ledLifeSec;
  tempArray[5] = enclosure.caseTempC;

  doc["lwts"] = tempArray[0];
  doc["prls"] = tempArray[1];
  doc["scrls"] = tempArray[2];
  doc["vatls"] = tempArray[3];
  doc["ledls"] = tempArray[4];
  doc["castc"] = tempArray[5];

  return doc;
}