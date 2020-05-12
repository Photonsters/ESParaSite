// ESParaSite.h

/* ESParasite Data Logger v0.6
        Authors: Andy (DocMadmag) Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/
#include <stdbool.h>

#ifndef INCLUDE_ESPARASITE_H_
#define INCLUDE_ESPARASITE_H_

namespace ESParaSite {

struct printchamber {
  float dht_temp_c{};
  float dht_humidity{};
  float dht_dewpoint{};
};

struct optics {
  float si_uvindex{};
  float si_visible{};
  float si_infrared{};
  float mlx_amb_temp_c{};
  float mlx_obj_temp_c{};
};

struct ambient {
  float bme_temp_c{};
  float bme_humidity{};
  float bme_barometer{};
  float bme_altitude{};
};

struct enclosure {
  uint32_t life_sec{};
  uint32_t lcd_sec{};
  uint32_t led_sec{};
  uint32_t fep_sec{};
  float case_temp{};
};

struct status_data {
  time_t rtc_current_second{};
  int is_printing_flag{};
};

struct rtc_eeprom_data {
  time_t first_on_timestamp{};
  time_t last_write_timestamp{};
  uint32_t screen_life_seconds{};
  uint32_t led_life_seconds{};
  uint32_t fep_life_seconds{};
  uint16_t last_segment_address{};
};

struct config_data {
  char cfg_mdns_name[32];
  const char *cfg_wifi_ssid{};
  const char *cfg_wifi_password{};
  int cfg_pin_sda{};
  int cfg_pin_scl{};
  bool cfg_mdns_enabled{};

};

struct sensor_exists {
  bool bmeDetected{};
  bool dhtDetected{};
  bool mlxDetected{};
  bool rtcDetected{};
  bool siDetected{};
};

}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_H_