// ESP8266.h

/* ESParaSite-ESP32 Data Logger
        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /ATTRIB for full credits and OSS License Info
        Please see /LIBRARIES for necessary libraries
        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
        source.

        All Original content is free and unencumbered software released into the
        public domain.

        The Author(s) are extremely grateful for the amazing open source
        communities that work to support all of the sensors, microcontrollers,
        web standards, etc.
*/

#define LED_ON LOW
#define LED_OFF HIGH

#define LED_BUILTIN                                                            \
  2 // Pin D4 mapped to pin GPIO2/TXD1 of ESP8266, NodeMCU and WeMoS, control
    // on-board LED
#define PIN_LED                                                                \
  2 // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
#define LED_PIN                                                                \
  16 // Pin D0 mapped to pin GPIO16 of ESP8266. This pin is also used for
     // Onboard-Blue LED. PIN_D0 = 0 => LED ON
// PIN_D0 can't be used for PWM/I2C

#define PIN_D0                                                                 \
  16 // Pin D0 mapped to pin GPIO16/USER/WAKE of ESP8266. This pin is also used
     // for Onboard-Blue LED. PIN_D0 = 0 => LED ON
#define PIN_D1 5 // Pin D1 mapped to pin GPIO5/SCL of ESP8266
#define PIN_D2 4 // Pin D2 mapped to pin GPIO4/SDA of ESP8266
#define PIN_D3 0 // Pin D3 mapped to pin GPIO0/FLASH of ESP8266
#define PIN_D4 2 // Pin D4 mapped to pin GPIO2/TXD1 of ESP8266

#define PIN_D5 14 // Pin D5 mapped to pin GPIO14/HSCLK of ESP8266
#define PIN_D6 12 // Pin D6 mapped to pin GPIO12/HMISO of ESP8266
#define PIN_D7 13 // Pin D7 mapped to pin GPIO13/RXD2/HMOSI of ESP8266
#define PIN_D8 15 // Pin D8 mapped to pin GPIO15/TXD2/HCS of ESP8266

#define PIN_RX 3 // Pin RX mapped to pin GPIO3/RXD0 of ESP8266
#define PIN_TX 1 // Pin RX mapped to pin GPIO1/TXD0 of ESP8266

// Don't use pins GPIO6 to GPIO11 as already connected to flash, etc. Use them
// can crash the program GPIO9(D11/SD2) and GPIO11 can be used only if flash in
// DIO mode ( not the default QIO mode)

#define PIN_SD2 9  // Pin SD2 mapped to pin GPIO9/SDD2 of ESP8266
#define PIN_SD3 10 // Pin SD3 mapped to pin GPIO10/SDD3 of ESP8266

#define PIN_SCL 5 // Pin SCL mapped to pin GPIO5/SCL of ESP8266
#define PIN_SDA 4 // Pin SDA mapped to pin GPIO4/SDA of ESP8266
