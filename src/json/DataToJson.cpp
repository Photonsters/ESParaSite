// DataToJson.cpp

/* ESParasite Data Logger
        Authors: Andy (DocMadmag) Eakin

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
  doc["timestmp"] = statusResource.rtcCurrentSecond;
  doc["ambTempC"] = ambientResource.ambientTempC;
  doc["ambHumidity"] = ambientResource.ambientHumidity;
  doc["ambPressure"] = ambientResource.ambientBarometer;
  doc["ambAltitude"] = ambientResource.ambientAltitude;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonChamber() {
  StaticJsonDocument<256> doc;

  doc["class"] = "chamber";
  doc["timestmp"] = statusResource.rtcCurrentSecond;
  doc["cmbTempC"] = chamberResource.chamberTempC;
  doc["cmbHumidity"] = chamberResource.chamberHumidity;
  doc["cmbDewpoint"] = chamberResource.chamberDewPoint;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonEeprom() {
  StaticJsonDocument<256> doc;
  String tempArray[6];
  tempArray[0] = rtcEepromResource.lastWriteTimestamp;
  tempArray[1] = rtcEepromResource.firstOnTimestamp;
  tempArray[2] = rtcEepromResource.eepromLedLifeSec;
  tempArray[3] = rtcEepromResource.eepromScreenLifeSec;
  tempArray[4] = rtcEepromResource.eepromVatLifeSec;
  tempArray[5] = rtcEepromResource.lastSegmentAddress;

  doc["class"] = "eeprom";
  doc["timestmp"] = statusResource.rtcCurrentSecond;
  doc["lstwrts"] = tempArray[0];
  doc["frstonts"] = tempArray[1];
  doc["eledls"] = tempArray[2];
  doc["escrls"] = tempArray[3];
  doc["evatls"] = tempArray[4];
  doc["lsegaddr"] = tempArray[5];

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonEnclosure() {
  StaticJsonDocument<256> doc;

  doc["class"] = "enclosure";
  doc["timestmp"] = statusResource.rtcCurrentSecond;
  doc["caseTempC"] = enclosureResource.caseTempC;
  doc["lifetimeSec"] = enclosureResource.printerLifeSec;
  doc["scrnLifeSec"] = enclosureResource.lcdLifeSec;
  doc["ledLifeSec"] = enclosureResource.ledLifeSec;
  doc["vatLifeSec"] = enclosureResource.vatLifeSec;

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonI2C() {
  StaticJsonDocument<256> doc;
  String tempArray[6];
  tempArray[0] = configResource.cfgPinSda;
  tempArray[1] = configResource.cfgPinScl;
  tempArray[2] = existsResource.dhtDetected;
  tempArray[3] = existsResource.bmeDetected;
  tempArray[4] = existsResource.mlxDetected;
  tempArray[5] = existsResource.siDetected;

  doc["class"] = "i2c";
  doc["sdaPin"] = tempArray[0];
  doc["sclPin"] = tempArray[1];
  doc["dhtExist"] = tempArray[2];
  doc["bmeExist"] = tempArray[3];
  doc["mlxExist"] = tempArray[4];
  doc["siExist"] = tempArray[5];

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

  doc["ssid"] = tempArray[0];
  doc["rssi"] = tempArray[1];
  doc["ipaddr"] = tempArray[2];
  doc["mdnsS"] = tempArray[3];
  doc["mdnsN"] = tempArray[4];

  ESParaSite::HttpHandleJson::serializeSendJson(doc);
}

void ESParaSite::DataToJson::getJsonOptics() {
  StaticJsonDocument<256> doc;

  doc["class"] = "optics";
  doc["timestamp"] = statusResource.rtcCurrentSecond;
  doc["uvIndex"] = opticsResource.ledUVIndex;
  doc["visible"] = opticsResource.ledVisible;
  doc["infrared"] = opticsResource.ledInfrared;
  doc["ledTempC"] = opticsResource.ledTempC;
  doc["scrnTempC"] = opticsResource.screenTempC;

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
