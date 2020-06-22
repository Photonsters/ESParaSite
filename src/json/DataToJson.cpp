// DataToJson.cpp

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
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <cppQueue.h>

#include "ESParaSite.h"
#include "DataDigest.h"
#include "Http.h"
#include "Json.h"

extern ESParaSite::chamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::rtcEepromData rtcEepromResource;
extern ESParaSite::sensorExists existsResource;

extern ESP8266WebServer server;

void ESParaSite::DataToJson::getJsonAmbient() {
  StaticJsonDocument<256> doc;

  doc["class"] = "ambient";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["ambTempC"] = ambientResource.ambientTempC;
  doc["ambHumid"] = ambientResource.ambientHumidity;
  doc["ambPres"] = ambientResource.ambientBarometer;
  doc["ambAlt"] = ambientResource.ambientAltitude;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonChamber() {
  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["cmbTempC"] = chamberResource.chamberTempC;
  doc["cmbHumid"] = chamberResource.chamberHumidity;
  doc["cmbDewPt"] = chamberResource.chamberDewPoint;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonCurrent() {
  StaticJsonDocument<256> doc;

  doc["ts"] = statusResource.rtcCurrentSecond;
  doc["at"] = ambientResource.ambientTempC;
  doc["ah"] = ambientResource.ambientHumidity;
  doc["ct"] = chamberResource.chamberTempC;
  doc["ch"] = chamberResource.chamberHumidity;
  doc["lt"] = opticsResource.ledTempC;
  doc["st"] = opticsResource.screenTempC;
  doc["lo"] = statusResource.isPrintingFlag;
  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonEeprom() {
  StaticJsonDocument<256> doc;

  doc["class"] = "eeprom";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["lastWrtTS"] = rtcEepromResource.lastWriteTimestamp;
  doc["firstOnTS"] = rtcEepromResource.firstOnTimestamp;
  doc["eprmLedLS"] = rtcEepromResource.eepromLedLifeSec;
  doc["eprmScrLS"] = rtcEepromResource.eepromScreenLifeSec;
  doc["eprmVatLS"] = rtcEepromResource.eepromVatLifeSec;
  doc["lastSegAddr"] = rtcEepromResource.lastSegmentAddress;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonEnclosure() {
  StaticJsonDocument<256> doc;

  doc["class"] = "enclosure";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["caseTempC"] = enclosureResource.caseTempC;
  doc["printerLS"] = enclosureResource.printerLifeSec;
  doc["curScrLS"] = enclosureResource.lcdLifeSec;
  doc["curLedLS"] = enclosureResource.ledLifeSec;
  doc["curVatLS"] = enclosureResource.vatLifeSec;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonI2C() {
  StaticJsonDocument<256> doc;

  doc["class"] = "i2c";
  doc["sdaPin"] = configResource.cfgPinSda;
  doc["sclPin"] = configResource.cfgPinScl;
  doc["dhtExist"] = existsResource.dhtDetected;
  doc["bmeExist"] = existsResource.bmeDetected;
  doc["mlxExist"] = existsResource.mlxDetected;
  doc["siExist"] = existsResource.siDetected;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonNetwork() {
  StaticJsonDocument<256> doc;
  String tempArray[5];
  tempArray[0] = WiFi.SSID();
  tempArray[1] = WiFi.RSSI();
  tempArray[2] = WiFi.localIP().toString();
  tempArray[3] = MDNS.isRunning();
  tempArray[4] = configResource.cfgMdnsName;

  doc["class"] = "network";
  doc["ssid"] = tempArray[0];
  doc["rssi"] = tempArray[1];
  doc["ipAddr"] = tempArray[2];
  doc["mdnsS"] = tempArray[3];
  doc["mdnsN"] = tempArray[4];

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonOptics() {
  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["uvIdx"] = opticsResource.ledUVIndex;
  doc["visLux"] = opticsResource.ledVisible;
  doc["irLux"] = opticsResource.ledInfrared;
  doc["ledTempC"] = opticsResource.ledTempC;
  doc["scrTempC"] = opticsResource.screenTempC;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonStatus(){
  StaticJsonDocument<256> doc;
  String tempArray[6];
  tempArray[0] = rtcEepromResource.lastWriteTimestamp;
  tempArray[1] = enclosureResource.printerLifeSec;
  tempArray[2] = enclosureResource.lcdLifeSec;
  tempArray[3] = enclosureResource.vatLifeSec;
  tempArray[4] = enclosureResource.ledLifeSec;
  tempArray[5] = enclosureResource.caseTempC;

  doc["lwts"] = tempArray[0];
  doc["prls"] = tempArray[1];
  doc["scrls"] = tempArray[2];
  doc["vatls"] = tempArray[3];
  doc["ledls"] = tempArray[4];
  doc["castc"] = tempArray[5];

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}
