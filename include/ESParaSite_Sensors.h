//ESParaSite_Sensors.h

/* ESParasite Data Logger v0.5
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

#ifndef _ESParaSite_Sensors_h
#define _ESParaSite_Sensors_h

class RtcDateTime;

class rtceeprom;

int ping_sensor(int address);

void init_dht_sensor();
void init_si_sensor();
void init_mlx_sensor();
void init_bme_sensor();
void init_rtc_clock();

void read_dht_sensor();
void read_si_sensor();
void read_mlx_sensor();
void read_bme_sensor();
void read_rtc_data();
void read_rtc_temp();
time_t read_rtc_epoch();

void init_i2c_sensors();
void dump_sensors();

void printDateTime(const RtcDateTime &dt);

extern void init_rtc_eeprom(int);

extern int convertCtoF(int temp_c);
extern double dewPoint(double celsius, double humidity);

extern printchamber chamber_resource;
extern optics optics_resource;
extern ambient ambient_resource;
extern enclosure enclosure_resource;
extern status status_resource;

#endif