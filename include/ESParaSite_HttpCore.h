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

//#include <ESPAsyncWebServer.h>

#ifndef INCLUDE_ESPARASITE_REST_H_
#define INCLUDE_ESPARASITE_REST_H_

namespace ESParaSite {
namespace HttpCore {

void configHttpServerRouting();
void startHttpServer();
void stopHttpServer();
void serveHttpClient();

// void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
//               AwsEventType type, void *arg, uint8_t *data, size_t len);
// void cleanup_http_client();

}; // namespace HttpCore
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_REST_H_
