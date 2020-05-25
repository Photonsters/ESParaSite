// ESParaSite_DatatoJson.h

/* ESParasite Data Logger v0.9
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

#ifndef INCLUDE_ESPARASITE_DATATOJSON_H_
#define INCLUDE_ESPARASITE_DATATOJSON_H_

namespace ESParaSite {
namespace DataToJson {

void historyToJson();
void statusToJson();

} // namespace DataToJson
} // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_DATATOJSON_H_