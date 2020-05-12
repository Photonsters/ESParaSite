// ESParaSite_Core.h

/* ESParasite Data Logger v0.6
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#ifndef INCLUDE_ESPARASITE_CORE_H_
#define INCLUDE_ESPARASITE_CORE_H_

#include <stdint.h>


namespace ESParaSite {
namespace Core {
uint16_t do_read_sensors(uint16_t, uint16_t);
uint16_t do_handle_eeprom(uint16_t, uint16_t);
uint16_t do_read_dht(uint16_t, uint16_t);
} // namespace Core
} // namespace ESParaSite

void do_check_printing();
bool is_printing();

#endif // INCLUDE_ESPARASITE_CORE_H_
