//ESParaSite_Eeprom.h

/* ESParasite Data Logger v0.4
	Authors: Andy (DocMadmag) Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#ifndef _ESParaSite_Eeprom_h
#define _ESParaSite_Eeprom_h

void init_rtc_eeprom(int);

void dumpEEPROM(uint16_t memoryAddress, uint16_t length);

int do_eeprom_first_read();
void do_eeprom_read();
int do_eeprom_write();
void do_eeprom_format(int format_type);

uint64_t join_64(uint32_t first_word, uint32_t second_word);
void SerializeUint32(unsigned char (&buf)[4], uint32_t val);
uint32_t ParseUint32(const char (&buf)[4]);

extern int ping_sensor();

extern enclosure enclosure_resource;
extern timestamp rtc_timestamp;
extern eeprom_data rtc_eeprom_resource;

//void test_eeprom_usage();

#endif