
/*
DebugUtils.h - Simple debugging utilities.
Copyright (C) 2011 Fabio Varesano <fabio at varesano dot net>

Ideas taken from:
http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1271517197

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

// ***FOR DEVELOPMENT ONLY***
// WARNING: Uncommenting these #define directives should only be done as part 
// of development and test work. Running ESParaSite with these enabled can 
// lead to instability or improper and unexpected operation.

// DEBUG_L1 dumps verbose data about what the program is doing.
#define DEBUG_L1

// DEBUG_L2 dumps sensor data on every read as well as EEPROM values and some
// variables. Additionally several delays are added which will slow
// down the web server response.
// WARNING: This wil print your WiFi Password in plain text!
//#define DEBUG_L2

// DEBUG_L3 dumps EEPROM values to serial console

// #define DEBUG_L3

// RESET_LIFE_COUNTERS resets the lifetime counters
// #define RESET_LIFE_COUNTERS

// FOR DEVELOPMENT USE ONLY!!!
// The below debug options are for use during development work only and
// should not be enabled in your code. Uncommenting these debug options will
// cause unexpected behavior and can crash the Serial Monitor or render your 
// ESP32 in a boot loop.

// PRINT_HISTORY dumps the History data to the serial console. This option 
// will overload platform.io's serial monitor and cause VSCode to hang.
// #define PRINT_HISTORY

// DEBUG_L4 dumps timing data on every loop and enumerates every step of the 
// loop.This can cause some terminals to crash and should not be used with 
// VSCode as your terminal.
//#define DEBUG_L4

// FORMAT_SPIFFS formats the SPIFFS filesystem this will erase all stored 
// website files and configuration data. This will force the ESParaSite into 
// the captive configuration portal.  This setting will cause a boot loop, 
// so it must be run once then disabled.
//#define FORMAT_SPIFFS

// FORMAT_EEPROM formats the immutable section of the EEPROM. This will write
// a new "First on Timestamp" of "0". This settiung will cause a boot loop,
// so it must be run once then disabled.
//#define FORMAT_EEPROM

#endif
