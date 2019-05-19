//ESParaSite_Rest.h

/* ESParaSite Data Logger v0.3
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

#ifndef _ESParaSite_Rest_h
#define _ESParaSite_Rest_h

class http_rest_server;

void config_rest_server_routing();

void do_client();
void start_http_server();

void get_chamber();
void get_optics();
void get_ambient();
void get_enclosure();

extern void read_dht_sensor();
extern void read_rtc_data();

extern void init_dht_sensor();
extern void init_bme_sensor();
extern void init_rtc_clock();

extern void read_rtc_data();
extern void read_dht_sensor();
extern void read_si_sensor();
extern void read_mlx_sensor();
extern void read_bme_sensor();

extern int convertCtoF(int temp_c);
extern time_t rtc_timestamp;

extern printchamber chamber_resource;
extern optics optics_resource;
extern ambient ambient_resource;
extern enclosure enclosure_resource;

#endif