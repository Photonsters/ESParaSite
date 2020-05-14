//ESParaSite_HttpHandler.h

/* ESParasite Data Logger v0.6
        Authors: Andy (DocMadmag) Eakin

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

#ifndef INCLUDE_ESPARASITE_HTTP_HANDLERS_H_
#define INCLUDE_ESPARASITE_HTTP_HANDLERS_H_

class http_rest_server;

namespace ESParaSite {
namespace HttpHandler {

void get_chamber();
void get_optics();
void get_ambient();
void get_enclosure();
void get_config();
void handleRoot();
void handleNotFound();
}; // namespace HttpHandler
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_HTTP_HANDLERS_H_