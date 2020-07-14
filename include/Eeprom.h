// Eeprom.h

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

#ifndef INCLUDE_EEPROM_H_
#define INCLUDE_EEPROM_H_
#include <arduino.h>

namespace ESParaSite {
namespace RtcEeprom {
void initRtcEeprom();
void dumpEEPROM(uint16_t, uint16_t);
int8_t doEepromFirstRead();
uint8_t doEepromRead(uint16_t);
uint8_t doEepromWrite();
void doEepromFormat(uint8_t);

}; // namespace RtcEeprom
}; // namespace ESParaSite

#endif // INCLUDE_EEPROM_H_
