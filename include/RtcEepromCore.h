// RtcEepromCore.h

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

#ifndef INCLUDE_EEPROM_H_
#define INCLUDE_EEPROM_H_

namespace ESParaSite {
namespace RtcEeprom {
void initRtcEeprom();
void dumpEEPROM(uint16_t, uint16_t);
int doEepromFirstRead();
uint8_t doEepromRead(uint16_t);
uint8_t doEepromWrite();

}; // namespace RtcEeprom
}; // namespace ESParaSite

void do_eeprom_format(uint8_t);

#endif // INCLUDE_EEPROM_H_
