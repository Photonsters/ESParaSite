// ESParaSite_HttpFile.h

/* ESParasite Data Logger v0.6
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

#ifndef INCLUDE_ESPARASITE_HTTPFILE_H_
#define INCLUDE_ESPARASITE_HTTPFILE_H_

class http_rest_server;

namespace ESParaSite {
namespace HttpFile {

String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
bool loadFromLittleFS(String path);

}; // namespace HttpFile
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_HTTPFILE_H_
