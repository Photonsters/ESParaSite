//ESParaSite_Core.h

/* ESParasite Data Logger v0.4
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

int init_wifi();
int do_sensors();
void do_check_printing();
void do_eeprom();

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
  uint32_t life_sec{};
  uint32_t lcd_sec{};
  uint32_t led_sec{};
  uint32_t fep_sec{};
};

struct status
{
  time_t rtc_current_second{};
  int is_printing_flag{};
};

struct eeprom_data
{
  time_t first_on_timestamp{};
  time_t last_write_timestamp{};
  int last_segment_address{};
  uint32_t screen_life_seconds{};
  uint32_t led_life_seconds{};
  uint32_t fep_life_seconds{};
};

#ifndef _ESParaSite_Util_h

int convertCtoF(int temp_c);
double dewPoint(double celsius, double humidity);

#endif

extern int convertCtoF(int temp_c);
extern double dewPoint(double celsius, double humidity);

#ifndef _ESParaSite_Sensors_h

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
time_t read_rtc_epoch();

void dump_sensors();
void init_i2c_sensors();

#endif

extern int bme_i2c_address;

extern int ping_sensor(int address);

extern void init_dht_sensor();
extern void init_si_sensor();
extern void init_mlx_sensor();
extern void init_bme_sensor();
extern void init_rtc_clock();

extern void read_dht_sensor();
extern void read_si_sensor();
extern void read_mlx_sensor();
extern void read_bme_sensor();
extern void read_rtc_data();
extern time_t read_rtc_epoch();

extern void dump_sensors();
extern void init_i2c_sensors();

#ifndef _ESParaSite_Eeprom_h

void init_rtc_eeprom();
void dumpEEPROM(uint16_t memoryAddress, uint16_t length);
int do_eeprom_first_read();
int do_eeprom_read(int);
int do_eeprom_write(int);

#endif

extern void init_rtc_eeprom();
extern void dumpEEPROM(uint16_t memoryAddress, uint16_t length);
extern int do_eeprom_first_read();
extern int do_eeprom_read(int);
extern int do_eeprom_write(int);

#ifndef _ESParaSite_Rest_h

void do_client();
void start_http_server();

#endif

extern void do_client();
extern void start_http_server();

#endif