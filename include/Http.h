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

#include <arduino.h>
#include <ArduinoJson.h>

class http_rest_server;

#ifndef INCLUDE_REST_H_
#define INCLUDE_REST_H_

namespace ESParaSite {
namespace HttpCore {

void configHttpServerRouting();
void startHttpServer();
void stopHttpServer();
void serveHttpClient();

}; // namespace HttpCore

namespace HttpFile {

String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
bool loadFromLittleFS(String path);
void handleGuiUpdate();

}; // namespace HttpFile

namespace HttpHandler {

void handleRoot();
void handleNotFound();
void handleWebRequests();
void getHtmlUpload();

void handleResetFep();
void handleResetLed();
void handleResetScreen();

void getResetFep();
void getResetLed();
void getResetScreen();

void handleSetClock();

void handleHistory();
void handleApiRequest();

void getGuiUpdate();

}; // namespace HttpHandler

namespace HttpHandleJson {

void serializeSendJson(const JsonDocument&);
void sendContentLengthUnknown();
void serializeSendJsonPartN(const JsonDocument &);

} // namespace HttpHandleJson
} // namespace ESParaSite

#endif // INCLUDE_REST_H_
