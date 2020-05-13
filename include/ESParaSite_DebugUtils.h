
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

// DEBUG_L1 dumps verbose data about what the program is doing
//#define DEBUG_L1

// DEBUG_L2 dumps sensor data on every read as well as EEPROM values and some
// variables.  Additionally several 500ms Delays are added which will slow down the web server response.
//#define DEBUG_L2

// DEBUG_L3 dumps time data on every loop this can cause some terminals to crash
// and should not be used with VSCode as your terminal.
//#define DEBUG_L3

#endif
