//ESParaSite_Core.h

/* ESParaSite Data Logger v0.3
	Authors: Andy  (SolidSt8Dad)Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

//+++ User Settings +++

//Put your WiFi network and WiFi password here:
//const char* wifi_ssid     = "yourwifinetwork";
//const char* wifi_password = "yourwifipassword";

const char* wifi_ssid     = "<wifi_ssid>";
const char* wifi_password = "<wifi_passwd>";

//Enter the port on which you want the HTTP server to run (Default is 80).  
//You will access the server at http://<ipaddress>:<port>
#define HTTP_REST_PORT 80

//+++ Advanced Settings +++
// For precise altitude measurements please put in the current pressure corrected for the sea level
// Otherwise leave the standard pressure as default (1013.25 hPa);
// Also put in the current average temperature outside (yes, really outside!)
// For slightly less precise altitude measurements, just leave the standard temperature as default (15°C and 59°F);
#define SEALEVELPRESSURE_HPA (1013.25)
#define CURRENTAVGTEMP_C (15)
#define CURRENTAVGTEMP_F (59)

//Set the I2C address of your BME280 breakout board
//int bme_i2c_address = 0x77;
int bme_i2c_address = 0x76;


//+++ DO NOT CHANGE ANYTHING BELOW THIS LINE +++

#define countof(a) (sizeof(a) / sizeof(a[0]))

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
unsigned long delayTime;
int bmeDetected = 0;

RtcDateTime now;
char timestamp[14];

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> Rtc(Wire);
EepromAt24c32<TwoWire> RtcEeprom(Wire);
BlueDot_BME280 bme;

void get_chamber ();
void get_optics ();
void get_ambient();
void get_enclosure();

void init_dht_sensor();
void init_bme_sensor();
void init_rtc_clock();

void read_rtc_data ();
void read_dht_sensor();
void read_si_sensor();
void read_mlx_sensor();
void read_bme_sensor();

int convertCtoF(int temp_c);
void printDateTime(const RtcDateTime & dt);
void create_timestamp(const RtcDateTime & dt);

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
};

