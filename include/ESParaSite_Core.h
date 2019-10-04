// ESParaSite_Core.h

/* ESParasite Data Logger v0.5
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#ifndef INCLUDE_ESPARASITE_CORE_H_
#define INCLUDE_ESPARASITE_CORE_H_

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

struct config_data
{
  const char *cfg_wifi_ssid{};
  const char *cfg_wifi_password{};
  int cfg_pin_sda{};
  int cfg_pin_scl{};
  bool cfg_mdns_enabled{};
  char cfg_mdns_name[32];
};

extern void init_rtc_eeprom();
extern void dumpEEPROM(uint16_t memoryAddress, uint16_t length);
extern int do_eeprom_first_read();
extern int do_eeprom_read(int);
extern int do_eeprom_write(int);

extern time_t read_rtc_epoch();

extern bool loadConfig();
extern bool saveConfig();

extern int init_wifi();
extern void do_config_portal();

#endif // INCLUDE_ESPARASITE_CORE_H_
