// HttpCore.h

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

#include <ESPAsyncWebServer.h>

#ifndef INCLUDE_REST_H_
#define INCLUDE_REST_H_

namespace ESParaSite {
namespace HttpCore {

void configHttpServerRouting();
void startHttpServer();

}; // namespace HttpCore

namespace HTTPHandler {

void handleAPI(AsyncWebServerRequest *);

void handleUpload(AsyncWebServerRequest *, String, size_t, uint8_t *, size_t,
                  bool);

}; // namespace HttpHandler

} // namespace ESParaSite

#endif // INCLUDE_REST_H_
