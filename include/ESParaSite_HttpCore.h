// ESParaSite_Rest.h

/* ESParasite Data Logger v0.5
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#ifndef INCLUDE_ESPARASITE_REST_H_
#define INCLUDE_ESPARASITE_REST_H_

class http_rest_server;

namespace ESParaSite {
namespace HttpCore {
void config_rest_server_routing();
void start_http_server();
void serve_http_client();
bool do_web_gui(String path);
}; // namespace HttpCore
}; // namespace ESParaSite

void get_chamber();
void get_optics();
void get_ambient();
void get_enclosure();
void get_config();
void handleRoot();
void handleNotFound();

String getContentType(String filename);
void handleFileUpload();

#endif // INCLUDE_ESPARASITE_REST_H_
