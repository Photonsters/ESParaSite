// ESParaSite_DataDigest.h

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
#include <time.h>

#ifndef INCLUDE_ESPARASITE_DIGEST_H_
#define INCLUDE_ESPARASITE_DIGEST_H_

#define FIVESECMAXELEMENT (5)
#define THIRTYSECMAXELEMENT (9)
#define THREEHSECMAXELEMENT (11)

struct history {
  time_t dataTimestamp;
  float chamberTempC;
  float chamberHumidity;
  float ambientTempC;
  float ambientHumidity;
  float screenTempC;
  float ledTempC;
  bool ledOn;
};

void fivesToThirty();
void thirtysToThreeH();

#define FIVESECMAXELEMENT (5)

namespace ESParaSite {
namespace DataDigest {

void fillRow();
void printRows();

} // namespace DataDigest
} // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_DIGEST_H_