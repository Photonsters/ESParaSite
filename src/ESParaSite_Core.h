//ESParaSite_Core.h

/* ESParaSite Data Logger v0.3
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

#ifndef _ESParaSite_Core_h
#define _ESParaSite_Core_h

int ping_sensor(int address);

void init_dht_sensor();
void init_bme_sensor();
void init_rtc_clock();

void read_rtc_data();
void read_dht_sensor();
void read_si_sensor();
void read_mlx_sensor();
void read_bme_sensor();

int convertCtoF(int temp_c);



struct printchamber
{
  float dht_temp_c{};
  float dht_humidity{};
  float dht_dewpoint{};
};

struct optics
{
  float si_uvindex{};
  float si_visible{};
  float si_infrared{};
  float mlx_amb_temp_c{};
  float mlx_obj_temp_c{};
};

struct ambient
{
  float bme_temp_c{};
  float bme_humidity{};
  float bme_barometer{};
  float bme_altitude{};
};

struct enclosure
{
  float case_temp{};
  float total_sec{};
  float screen_sec{};
  float led_sec{};
  float fep_sec{};
};

extern void do_client();
extern void start_http_server();

#endif