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

void config_rest_server_routing();

void init_dht_sensor();
void init_bme_sensor();
void init_rtc_clock();

void read_rtc_data ();
void read_dht_sensor();
void read_si_sensor();
void read_mlx_sensor();
void read_bme_sensor();

int convertCtoF(int temp_c);

struct printchamber {
  float dht_temp_c{ 0 };
  float dht_humidity{ 0 };
  float dht_dewpoint{ 0 };
};

struct optics {
  float si_uvindex{ 0 };
  float si_visible{ 0 };
  float si_infrared{ 0 };
  float mlx_amb_temp_c{ 0 };
  float mlx_obj_temp_c{ 0 };
};

struct ambient {
  float bme_temp_c{ 0 };
  float bme_humidity{ 0 };
  float bme_barometer{ 0 };
  float bme_altitude{ 0 };
};

struct enclosure {
  float case_temp{ 0 };
  float total_sec{ 0 };
  float screen_sec{ 0 };
  float led_sec{ 0 };
  float fep_sec{0};
};

#endif