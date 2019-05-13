// ESParaSite Settings

/* ESParaSite Data Logger v0.3
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#pragma once

#ifndef _ESParaSite_Settings_h
#define _ESParaSite_Settings_h

//+++ User Settings +++

//Put your WiFi network and WiFi password here:

//const char* wifi_ssid     = "yourwifinetwork";
//const char* wifi_password = "yourwifipassword";

const char* wifi_ssid     = "mxbnet";
const char* wifi_password = "Presto1234!";

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


//*** DO NOT MODIFY ANYTHING BELOW THIS LINE ***

#endif