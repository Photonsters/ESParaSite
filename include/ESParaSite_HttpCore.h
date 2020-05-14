// ESParaSite_HttpCore.h

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

#ifndef INCLUDE_ESPARASITE_REST_H_
#define INCLUDE_ESPARASITE_REST_H_

class http_rest_server;

namespace ESParaSite {
namespace HttpCore {
void config_rest_server_routing();
void start_http_server();
void stop_http_server();
void serve_http_client();

String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
}; // namespace HttpCore
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_REST_H_
