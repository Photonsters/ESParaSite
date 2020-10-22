// ConfigPortal.h

/* ESParaSite-ESP32 Data Logger
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

#ifndef INCLUDE_CONFIGP_H_
#define INCLUDE_CONFIGP_H_

#include <stdint.h>

namespace ESParaSite {
namespace ConfigPortal {

void doConfigPortal();

} // namespace ConfigPortal
} // namespace ESParaSite

uint8_t connectMultiWiFi(void);
void toggleLED();
void heartBeatPrint(void);
void check_WiFi(void);
void check_status(void);

#endif // INCLUDE_CONFIGP_H_