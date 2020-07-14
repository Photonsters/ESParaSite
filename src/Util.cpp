// Util.cpp

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

#include "ESParaSite.h"
#include "DebugUtils.h"
#include "Util.h"


using namespace ESParaSite;

#define countof(a) (sizeof(a) / sizeof(a[0]))

int16_t Util::convertCtoF(float temp_c) {
  int16_t temp_f = (static_cast<int>(round(1.8 * temp_c + 32)));
  return temp_f;
}

double Util::dewPoint(double celsius, double humidity) {
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO))) - 1);
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
  RHS += log10(1013.246);

  // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

  // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP / 0.61078);
  return (241.88 * T) / (17.558 - T);
}

void Util::printDateTime(const RtcDateTime &dt) {
  char datestring[20];

  snprintf_P(datestring, countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"), dt.Month(), dt.Day(),
             dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
  Serial.print(datestring);
}

uint64 Util::join64(uint32 first_part, uint32 second_part) {
  uint64 joined_word = (((u64)first_part) << 32 | second_part);
  return joined_word;
}

void Util::SerializeUint32(unsigned char (&buf)[4], uint32_t val) {
  buf[0] = val;
  buf[1] = val >> 8;
  buf[2] = val >> 16;
  buf[3] = val >> 24;
}

uint32_t Util::ParseUint32(const char (&buf)[4]) {
  // This prevents buf[i] from being promoted to a signed int.
  uint32_t u0 = buf[0], u1 = buf[1], u2 = buf[2], u3 = buf[3];
  uint32_t uval = u0 | (u1 << 8) | (u2 << 16) | (u3 << 24);
  return uval;
}

int16_t ESParaSite::Util::floatToInt(float floatValue){
  int16_t intValue;
  intValue = roundf(floatValue*100);
  return intValue;
}
