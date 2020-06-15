// DataDigest.h

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
#include <time.h>

#ifndef INCLUDE_DIGEST_H_
#define INCLUDE_DIGEST_H_

#define FIVESECMAXELEMENT (5)
#define THIRTYSECMAXELEMENT (9)
#define FIVEMINMAXELEMENT (11)
#define ONEHOURMAXELEMENT (11)

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
void thirtysTofiveMin();
void fiveMinToOneHour();

namespace ESParaSite {
namespace DataDigest {

void fillRow();
void printRows();

} // namespace DataDigest
} // namespace ESParaSite

#endif // INCLUDE_DIGEST_H_