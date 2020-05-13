// ESParaSite_Sensors.h

/* ESParasite Data Logger v0.6
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
   source.

        All Original content is free and unencumbered software released into the
   public domain.
*/

#ifndef INCLUDE_ESPARASITE_SENSORSCORE_H_
#define INCLUDE_ESPARASITE_SENSORSCORE_H_

namespace ESParaSite {
namespace Sensors {
void init_dht_sensor();
void init_si_sensor();
void init_mlx_sensor();
void init_bme_sensor();
void init_rtc_clock();
void read_dht_sensor(bool);
void read_si_sensor();
void read_mlx_sensor();
void read_bme_sensor();
void read_rtc_data();
void check_rtc_status();
void read_rtc_temp();
time_t read_rtc_epoch();
void init_i2c_sensors();
int ping_sensor(uint16_t);
void dump_sensors(bool);
}; // namespace Sensors
}; // namespace ESParaSite

#endif // INCLUDE_ESPARASITE_SENSORSCORE_H_
