//ESParaSite_Rest.h

/* ESParaSite Data Logger v0.3
	Authors: Andy  (SolidSt8Dad)Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

#ifndef _ESParaSite_Rest_h
#define _ESParaSite_Rest_h

#include <ESP8266WebServer.h>
#include "ESParaSite_Settings.hxx"
extern ESP8266WebServer http_rest_server(HTTP_REST_PORT);

extern time_t timestamp;

extern void get_chamber ();
extern void get_optics ();
extern void get_ambient();
extern void get_enclosure();

extern void read_dht_sensor();
extern void read_rtc_data();

extern printchamber chamber_resource;
extern optics optics_resource;
extern ambient ambient_resource;
extern enclosure enclosure_resource;

#endif