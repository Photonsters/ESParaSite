// Util.h

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

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <RtcDS3231.h>

namespace ESParaSite {
namespace Util {
        
int16_t convertCtoF(float temp_c);
double dewPoint(double celsius, double humidity);
void printDateTime(const RtcDateTime &dt);
uint64_t join64(uint32_t first_word, uint32_t second_word);
void SerializeUint32(unsigned char (&buf)[4], uint32_t val);
uint32_t ParseUint32(const char (&buf)[4]);
int16_t floatToInt(float floatValue);
}; // namespace Util
}; // namespace ESParaSite

#endif // INCLUDE_UTIL_H_
