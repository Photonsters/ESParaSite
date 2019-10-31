// ESParaSite_Eeprom.h

/* ESParasite Data Logger v0.5
        Authors: Andy (DocMadmag) Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#ifndef INCLUDE_ESPARASITE_EEPROM_H_
#define INCLUDE_ESPARASITE_EEPROM_H_

namespace ESParaSite {
namespace RtcEeprom {
void init_rtc_eeprom();
void dumpEEPROM(uint16_t, uint16_t);
int do_eeprom_first_read();
uint8_t do_eeprom_read(uint16_t);
uint8_t do_eeprom_write();

}; // namespace RtcEeprom
}; // namespace ESParaSite

void do_eeprom_format(uint8_t);

#endif // INCLUDE_ESPARASITE_EEPROM_H_
