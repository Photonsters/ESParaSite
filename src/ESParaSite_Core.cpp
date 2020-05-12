// ESParaSite_Core.cpp

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

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "ESParaSite.h"
#include "ESParaSite_ConfigPortal.h"
#include "ESParaSite_Core.h"
#include "ESParaSite_FileCore.h"
#include "ESParaSite_HttpCore.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"
#include "ESParaSite_WiFiCore.h"

// +++ Advanced Settings +++
// VISIBLE_THRESHOLD adjusts the sensitivity of the SI1145 Sensor to Ambient
// light when used to detect print activity.
// Default is (280).
#define VISIBLE_THRESHOLD (280.00)

// *** It IS STRONGLY RECOMMENDED THAT YOU DO NOT MODIFY THE BELOW VALUES ***

// ALL_SENSOR_POLLING_SEC determines how often the sensor array should be
// polled for new data.

// Increasing the polling interval will reduce the performance requirement,
// however it will decrease the resolution of the lifetime counter.
#define ALL_SENSOR_POLLING_SEC (1)

// Since the DHT12 has a 0.5Hz Sample Rate, the minimum DHT_SENSOR_POLLING_SEC
// is (2) by default we use (3).
#define DHT_SENSOR_POLLING_SEC (3)

// EEPROM_WRITE_INTERVAL_SEC determines how often we update the EEPROM that
// contains the various lifetime counters.
// The value of 30s gives us over 13 years of EEPROM life.

// Reducing this value will burn out the EEPROM chip faster. Increasing it
// will increase the EEPROM life, but at the cost of a higher margin of
// error for the lifetime counters.

// The margin of error for the DEFAULT 30s Interval is ~2.5hours/year or
// roughly 0.02%.
#define EEPROM_WRITE_INTERVAL_SEC (30)

// *** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

// Trigger for inititating config mode is Pin D3 and also flash button on
// NodeMCU.  Flash button is convenient to use but if it is pressed it will
// hang the serial port device driver until the computer is rebooted on
// Windows machines.

// D3 on NodeMCU and WeMos.
const int TRIGGER_PIN = 0;

// Alternate button, if an external button is desired.
// D0 on NodeMCU and WeMos.
const int TRIGGER_PIN2 = 16;

uint8_t is_printing_counter = 0;

const uint16_t sensors_read_msec = (ALL_SENSOR_POLLING_SEC * 1000);
const uint16_t dht_read_msec = (DHT_SENSOR_POLLING_SEC * 1000);
const uint16_t eeprom_write_msec = (EEPROM_WRITE_INTERVAL_SEC * 1000);

extern ESParaSite::enclosure enclosure_resource;
extern ESParaSite::optics optics_resource;
extern ESParaSite::rtc_eeprom_data rtc_eeprom_resource;
extern ESParaSite::status_data status_resource;

WiFiClient Wifi;

uint16_t ESParaSite::Core::do_read_sensors(uint16_t cur_loop_msec,
                                           uint16_t prev_sensor_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_sensor_msec) >=
      sensors_read_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Reading the sensors"));
    Serial.println();
#endif

    status_resource.rtc_current_second = Sensors::read_rtc_epoch();
    enclosure_resource.life_sec = (status_resource.rtc_current_second -
                                   rtc_eeprom_resource.first_on_timestamp);

    Serial.print(F("This Printer has been on for:\t"));
    Serial.print(enclosure_resource.life_sec);
    Serial.println(F("  seconds"));
    Serial.println();

    ESParaSite::Sensors::read_bme_sensor();
    ESParaSite::Sensors::read_mlx_sensor();
    ESParaSite::Sensors::read_rtc_temp();
    ESParaSite::Sensors::read_si_sensor();

    do_check_printing();

    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT read the sensors"));
    Serial.println();
#endif

    return (prev_sensor_msec);
  }
}

uint16_t ESParaSite::Core::do_read_dht(uint16_t cur_loop_msec,
                                       uint16_t prev_dht_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_dht_msec) >= dht_read_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Reading the DHT sensor"));
    Serial.println();
#endif

    ESParaSite::Sensors::read_dht_sensor(true);
    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT read the DHT sensor"));
    Serial.println();
#endif

    return (prev_dht_msec);
  }
}

uint16_t ESParaSite::Core::do_handle_eeprom(uint16_t cur_loop_msec,
                                            uint16_t prev_eeprom_msec) {
  if (static_cast<uint16_t>(cur_loop_msec - prev_eeprom_msec) >=
      eeprom_write_msec) {

#ifdef DEBUG_L1
    Serial.println(F("Checking printing status before writing EEPROM"));
    Serial.println();
#endif

    status_resource.is_printing_flag = static_cast<uint8_t>(is_printing());

#ifdef DEBUG_L1
    Serial.print(F("Printing Status:\t"));
    Serial.println(status_resource.is_printing_flag);
    Serial.println();
    Serial.println(F("Writing the EEPROM"));
    Serial.println();
#endif

    ESParaSite::RtcEeprom::do_eeprom_write();

    return millis();
  } else {

#ifdef DEBUG_L1
    Serial.println(F("We did NOT write to the EEPROM"));
    Serial.println();
#endif

    return (prev_eeprom_msec);
  }
}

void do_check_printing() {
  if (optics_resource.si_visible >= VISIBLE_THRESHOLD) {

#ifdef DEBUG_L1
    Serial.println(F("Incrementing is_printing_counter"));
    Serial.println();
#endif

    is_printing_counter++;
  } else{

#ifdef DEBUG_L1
    Serial.println(F("Not incrementing is_printing_counter"));
    Serial.println();
#endif

  }
}

bool is_printing() {
  // if 50% or more of our poll intervals detect light we will set our
  // flag for the full write Interval.
  if (is_printing_counter >=
      (static_cast<uint8_t>((EEPROM_WRITE_INTERVAL_SEC / ALL_SENSOR_POLLING_SEC) /
                        2))) {
    is_printing_counter = 0;
    rtc_eeprom_resource.screen_life_seconds += EEPROM_WRITE_INTERVAL_SEC;
    rtc_eeprom_resource.led_life_seconds += EEPROM_WRITE_INTERVAL_SEC;
    rtc_eeprom_resource.fep_life_seconds += EEPROM_WRITE_INTERVAL_SEC;

#ifdef DEBUG_L1
    Serial.println(F("Currently printing during this interval"));
    Serial.println();
#endif

    return 1;
  } else {
    is_printing_counter = 0;

#ifdef DEBUG_L1
    Serial.println(F("Not printing during this interval"));
    Serial.println();
#endif

    return 0;
  }
}
