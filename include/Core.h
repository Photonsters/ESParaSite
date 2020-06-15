// Core.h

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

#ifndef INCLUDE_CORE_H_
#define INCLUDE_CORE_H_

#include <stdint.h>


namespace ESParaSite {
namespace Core {
        
uint16_t doReadSensors(uint16_t, uint16_t);
uint16_t doHandleEeprom(uint16_t, uint16_t);
uint16_t doReadDht(uint16_t, uint16_t);
uint16_t doHandleHistory(uint16_t, uint16_t);

} // namespace Core
} // namespace ESParaSite

void doCheckPrinting();
bool isPrinting();

#endif // INCLUDE_CORE_H_
