//ESParaSite_Core.cpp

/* ESParasite Data Logger v0.4
	Authors: Andy  (SolidSt8Dad)Eakin

	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory

	All Derived Content is subject to the most restrictive licence of it's source.

	All Original content is free and unencumbered software released into the public domain.
*/

#include <Arduino.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include "I2C_eeprom.h"

#include "ESParaSite_Core.h"
#include "ESParaSite_Eeprom.h"

/* 	The AT24C32 EEPROM is structured with 4096 x 8 bit words. Writing is performed on the page boundaries of 128 x 32 byte pages.
	The EEPROM is rated for 1 Million Write Cycles and we will write the values every 60 Seconds (1 Minute) of real time. 
	If we write this data 24/7/365 we would burn out the EEPROM in 694 days or just under 2 years.
	In order to extend the life of the EEPROM chip, we will further divide the pages into 16 x 8 page 'segments'. The first 'segment' will be used to hold any non-resettable counters and 
	will only be written at first boot (or if we add new counters in the future). 
	This will allow us to log 8 separate 32-bit values twice each minute for over 13 years.
	At each write, we will use the first 2 pages of each 'segment' for a 'write_timestamp'. Then, after writing the remining values, we will shift to the next segment,
	spreading the writes out evenly across the remaining 15 segments memory cells. 
	At boot time, we will read the 'write_timestamp' value from all 'segments' and compare to find the MRU. Then, we will read in the remaining pages from the MRU segment.	
	As a side effect, since the segments are only overwritten on the 9th cycle. If we encounter read errors, we will fall back to the segment immediately preceding the MRU. (not yet implemented)
*/

#define NUMBER_OF_EEPROM_SEGMENTS (16)
#define PAGES_PER_SEGMENT (8)
#define BYTES_PER_PAGE (4)
#define FIRST_SEGMENT_OFFSET (32)
#define MAX_EEPROM_FORMAT_VERSION (1)

//for decimal display uncomment below two lines
//#define DISPLAY_DECIMAL
//#define BLOCK_TO_LENGTH 10

//for hex display uncomment below two lines
#define DISPLAY_HEX
#define BLOCK_TO_LENGTH 16

#define MEMORY_SIZE 0x1000 //total bytes can be accessed 24LC64 = 0x2000 (maximum address = 0x1FFF)

/* 	These values control the I2C address of each sensor. Som chips may use different addresses and I recommend utilizing the I2C scanner sketch at:
 	https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
	to scan for your sensors if you are having any issues with communication.
*/

// DHT12 Sensor Address. Default (0x5C)
#define DHT_ADDR (0x5C)
// SI1145 Sensor Address. Default (0x60)
#define SI_ADDR (0x60)
// MLX90614 Sensor Address. Default (0x5A)
#define MLX_ADDR (0x5A)
// BME280 Sensor Address. Default (0x76) or (0x77)
#define BME_ADDR_A (0x76)
#define BME_ADDR_B (0x77)
// DS3231 Real Time Clock Address. Default (0x68)
#define RTC_ADDR (0x68)
// AT24C32 EEPROM Address. Default (0x50)
#define RTC_EEPROM_ADDR (0x57)

uint32_t start, diff, totals = 0;

I2C_eeprom rtc_eeprom(RTC_EEPROM_ADDR, MEMORY_SIZE);

void init_rtc_eeprom()
{
	rtc_eeprom.begin();

	Serial.println("Determining EEPROM size");
	int size = rtc_eeprom.determineSize();
	if (size > 0)
	{
		Serial.print("SIZE: ");
		Serial.print(size);
		Serial.println(" KB");
	}
	else if (size == 0)
	{
		Serial.println("WARNING: Can't determine eeprom size");
	}
	else
	{
		Serial.println("ERROR: Can't find eeprom\nstopped...");
		while (1)
			;
	}

	// Now we will check the first segment for proper format
	int current_seg_addr = 0;
	uint32_t read_words[3]{};

	for (int i = 0; i < 3; i++)
	{
		char page_data[BYTES_PER_PAGE];

		rtc_eeprom.readBlock(current_seg_addr, (uint8_t *)page_data, BYTES_PER_PAGE);
		read_words[i] = ParseUint32(page_data);
		current_seg_addr += (BYTES_PER_PAGE);
		if (i == 1)
		{
			current_seg_addr = (BYTES_PER_PAGE * 7);
		}
	}

	bool test_1 = false;
	bool test_2 = false;
	bool test_3 = false;
	uint32_t current_rtc_epoch = read_rtc_epoch();

	// First test - Data at index [0] should be '0' if date and time is currently prior to 03:14:07 on Tuesday, 19 January 2038.
	if (read_words[0] != 0)
	{
		if (current_rtc_epoch <= 2147483647)
		{
			test_1 = true;
		}
	}
	// Second test - Data at index [1] should be both less than the current epoch time and should never exceed '2147483647'
	if (read_words[1] >= current_rtc_epoch || read_words[1] > 2147483647)
	{
		test_2 = true;
	}

	//Third test - The last page of the reserved segment holds the EEPROM Format version number. The value should only be '0' if the rom is initialized
	// and should never be greater than 'MAX_EEPROM_FORMAT_VERSION'.
	if ((int)read_words[2] == 0 || (int)read_words[2] > MAX_EEPROM_FORMAT_VERSION)
	{
		test_3 = true;
	}

	if (test_3)
	{
		Serial.println("WARNING: The EEPROM Format is Invalid. All reserved values will be reset and all eeprom values will be set to '0'.");
		do_eeprom_format(1);
	}
	else if (test_2)
	{
		Serial.println("WARNING: The first_on_timestamp is Invalid. All reserved values will be reset.");
		do_eeprom_format(2);
	}
	else if (test_1)
	{
		Serial.println("WARNING: The first_on_timestamp is Invalid. This value will be reset to 0.");
		do_eeprom_format(3);
	}
	else
	{
		Serial.print("EEPROM Format Valid - Version:\t");
		Serial.println(read_words[2]);
	}
}

void dumpEEPROM(uint16_t memoryAddress, uint16_t length)
{
#ifdef DISPLAY_DECIMAL
	Serial.print("\t  ");
#endif
#ifdef DISPLAY_HEX
	Serial.print("\t ");
#endif
	for (int x = 0; x < BLOCK_TO_LENGTH; x++)
	{
		if (x != 0)
		{
#ifdef DISPLAY_DECIMAL
			Serial.print("    ");
#endif
#ifdef DISPLAY_HEX
			Serial.print("   ");
#endif
		}
#ifdef DISPLAY_DECIMAL
		Serial.print(x);
#endif
#ifdef DISPLAY_HEX
		Serial.print(x, HEX);
#endif
	}
	Serial.println();

	// block to defined length
	memoryAddress = memoryAddress / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;
	length = (length + BLOCK_TO_LENGTH - 1) / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;

	byte b = rtc_eeprom.readByte(memoryAddress);
	for (unsigned int i = 0; i < length; i++)
	{
		char buf[6];
		if (memoryAddress % BLOCK_TO_LENGTH == 0)
		{
			if (i != 0)
			{
				Serial.println();
			}
#ifdef DISPLAY_DECIMAL
			sprintf(buf, "%05d", memoryAddress);
#endif
#ifdef DISPLAY_HEX
			sprintf(buf, "%04X", memoryAddress);
#endif
			Serial.print(buf);
			Serial.print(":\t");
		}
#ifdef DISPLAY_DECIMAL
		sprintf(buf, "%03d", b);
#endif
#ifdef DISPLAY_HEX
		sprintf(buf, "%02X", b);
#endif
		Serial.print(buf);
		b = rtc_eeprom.readByte(++memoryAddress);
		Serial.print("  ");
	}
	Serial.println();
}

int do_eeprom_first_read()
{
	uint32_t first_word;
	uint32_t second_word;

	first_word = 0;
	second_word = 0;

	char page_1[BYTES_PER_PAGE];
	char page_2[BYTES_PER_PAGE];

	rtc_eeprom.readBlock(0, (uint8_t *)page_1, BYTES_PER_PAGE);
	rtc_eeprom.readBlock(BYTES_PER_PAGE, (uint8_t *)page_2, BYTES_PER_PAGE);

	first_word = ParseUint32(page_1);
	second_word = ParseUint32(page_2);

	rtc_eeprom_resource.first_on_timestamp = join_64(first_word, second_word);

	int seg_addr[NUMBER_OF_EEPROM_SEGMENTS];
	uint64_t timestamps[NUMBER_OF_EEPROM_SEGMENTS];

	Serial.print("Looking for MRU EEPROM segment");

	int current_seg_addr = FIRST_SEGMENT_OFFSET;

	for (int seg = 0; seg < (NUMBER_OF_EEPROM_SEGMENTS - 1); seg++)
	{
		first_word = 0;
		second_word = 0;

		rtc_eeprom.readBlock(current_seg_addr, (uint8_t *)page_1, BYTES_PER_PAGE);
		rtc_eeprom.readBlock((current_seg_addr + BYTES_PER_PAGE), (uint8_t *)page_2, BYTES_PER_PAGE);

		first_word = ParseUint32(page_1);
		second_word = ParseUint32(page_2);

		seg_addr[seg] = current_seg_addr;
		timestamps[seg] = join_64(first_word, second_word);

		current_seg_addr += (PAGES_PER_SEGMENT * BYTES_PER_PAGE);
		Serial.print(".");
	}

	Serial.println();

	int most_recent = std::max_element(timestamps, timestamps + NUMBER_OF_EEPROM_SEGMENTS) - timestamps;

	time_t mru_timestamp = timestamps[most_recent];
	if (mru_timestamp == -1 || mru_timestamp == 0 || most_recent == 0)
	{
		Serial.println("EEPROM Does not contain any valid data. Data logging will begin at first segment.");
		return (FIRST_SEGMENT_OFFSET);
	}
	else
	{
		Serial.print("The most recent write timestamp is:\t");
		Serial.println(mru_timestamp);
		return (seg_addr[most_recent]);
	}
}

int do_eeprom_read(int segment_addr)
{
	Serial.print("Reading EEPROM Values");

	uint32_t first_word;
	uint32_t second_word;

	for (int page = 0; page < PAGES_PER_SEGMENT; page++)
	{
		Serial.print(".");

		first_word = 0;
		second_word = 0;

		char page_1[BYTES_PER_PAGE];
		char page_2[BYTES_PER_PAGE];

		switch (page)
		{
		case 0:
			rtc_eeprom.readBlock(segment_addr, (uint8_t *)page_1, BYTES_PER_PAGE);
			rtc_eeprom.readBlock((segment_addr + BYTES_PER_PAGE), (uint8_t *)page_2, BYTES_PER_PAGE);

			first_word = ParseUint32(page_1);
			second_word = ParseUint32(page_2);

			rtc_eeprom_resource.last_write_timestamp = join_64(first_word, second_word);
			break;
		case 4:
			rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page), (uint8_t *)page_1, BYTES_PER_PAGE);
			rtc_eeprom_resource.fep_life_seconds = ParseUint32(page_1);
			break;
		case 5:
			rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page), (uint8_t *)page_1, BYTES_PER_PAGE);
			rtc_eeprom_resource.led_life_seconds = ParseUint32(page_1);
			break;
		case 6:
			rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page), (uint8_t *)page_1, BYTES_PER_PAGE);
			rtc_eeprom_resource.screen_life_seconds = ParseUint32(page_1);
			break;
		default:
			break;
		}
	}
	Serial.println();
	Serial.println("DONE!");
	Serial.println();
	return 0;
}

int do_eeprom_write()
{
	return 0;
}

void do_eeprom_format(int format_type)
{
	Serial.println("RTC EEPROM Zeroed");

	Serial.println("The current epoch is:");
	time_t current_epoch = read_rtc_epoch();
	Serial.println(current_epoch);

	uint32_t first_word = (uint32_t)((current_epoch & 0xFFFFFFFF00000000LL) >> 32);
	uint32_t second_word = (uint32_t)(current_epoch & 0xFFFFFFFFLL);

	Serial.println(first_word);
	Serial.println(second_word);

	unsigned char first_data[4];
	unsigned char second_data[4];
	unsigned char third_data[4];

	SerializeUint32(first_data, first_word);
	rtc_eeprom.writeBlock(0, (uint8_t *)first_data, 4);

	SerializeUint32(second_data, second_word);
	rtc_eeprom.writeBlock(4, (uint8_t *)second_data, 4);

	SerializeUint32(third_data, MAX_EEPROM_FORMAT_VERSION);
	rtc_eeprom.writeBlock((BYTES_PER_PAGE * 7), (uint8_t *)third_data, 4);
}

/*
int split_64(uint64 joined_word)
{
	uint32 first_word = (u32)((joined_word & 0xFFFFFFFF00000000LL) >> 32);
	uint32 second_word = (u32)(joined_word & 0xFFFFFFFFLL);
}
*/

uint64 join_64(uint32 first_word, uint32 second_word)
{
	uint64 joined_word = (((u64)first_word) << 32 | second_word);
	return joined_word;
}
/*
void test_eeprom_usage()
{
	Serial.println("We need to do reverse value packing to put a uint64_t into 2 x uint32_t for storage in eeprom");
	Serial.println("The current epoch is:");
	time_t current_epoch = read_rtc_epoch();
	Serial.println(current_epoch);

	uint32_t first_word = (uint32_t)((current_epoch & 0xFFFFFFFF00000000LL) >> 32);
	uint32_t second_word = (uint32_t)(current_epoch & 0xFFFFFFFFLL);

	Serial.println("We split the epoch into 2 words:");
	Serial.println(first_word);
	Serial.println(second_word);

	Serial.println("And we join it back together to get the original value");

	uint64_t joined_word = (((uint64_t)first_word) << 32 | second_word);
	time_t reassembled_epoch = (joined_word);

	Serial.println(reassembled_epoch);

	if (current_epoch == reassembled_epoch)
	{
		Serial.println("They match!");
	}
	else
	{
		Serial.println("They don't match!");
	}

	Serial.println("Now let us write to EEPROM");

	rtc_eeprom.setBlock(0, 0, 32);
	dumpEEPROM(0, 32);

	unsigned char first_data[4];
	unsigned char second_data[4];

	SerializeUint32(first_data, first_word);
	rtc_eeprom.writeBlock(0, (uint8_t *)first_data, 4);
	dumpEEPROM(0, 16);

	SerializeUint32(second_data, second_word);
	rtc_eeprom.writeBlock(4, (uint8_t *)second_data, 4);
	dumpEEPROM(0, 16);

	Serial.println("We are going to wait 3 seconds to visually verify the dump looks proper");
	delay(3000);

	Serial.println("Now we are going to try to read back the values and reassemble them");

	char page_1[4];
	char page_2[4];

	//memset(ar, 0, 100);
	rtc_eeprom.readBlock(0, (uint8_t *)page_1, 4);
	Serial.println(page_1);
	rtc_eeprom.readBlock(4, (uint8_t *)page_2, 4);
	Serial.println(page_2);

	//Let's parse those blocks we just read so we can convert them back
	first_word = ParseUint32(page_1);
	second_word = ParseUint32(page_2);

	int i;
	for (i = 0; i < 4; i++)
	{
		Serial.print(page_1[i]);
	}
	Serial.println();
	for (i = 0; i < 4; i++)
	{
		Serial.print(page_2[i]);
	}
	Serial.println();
	Serial.println();
	Serial.println(first_word);
	Serial.println(second_word);

	uint64_t saved_second_word = second_word;

	joined_word = (((uint64_t)first_word) << 32 | second_word);
	time_t read_reassembled_epoch = (joined_word);

	if (current_epoch == read_reassembled_epoch)
	{
		Serial.println("Epochs -- They match!");
	}
	else
	{
		Serial.println("Epochs -- They don't match!");
	}

	// OK, our next step is to test our wear levelling by writing the data to 8 different "Segments" across the eeprom and using the reassembled epoch value to determine the last segment written.
	// We are going to write to the first two pages all 8 segments and subtract or add arbitrary values from them to simulate the 1st, 4th, and 8th value being the largest
	// that should tell us if our code to detect the latest write is good.
	Serial.println("Wear Levelling Test - 4th value greatest");
	int b = 0;

	for (int seg = 0; seg < 8; seg++)
	{

		int seg_addr[8];
		uint32_t second_words[8];

		rtc_eeprom.setBlock(b, 0, 64);
		dumpEEPROM(b, 64);

		seg_addr[seg] = b;
		second_words[seg] = second_word;

		Serial.println();
		Serial.print("Iteration\t\t");
		Serial.println(seg);
		Serial.print("Segment Start Address\t\t");
		Serial.println(b);
		Serial.println(first_word);
		Serial.println(second_word);

		SerializeUint32(first_data, first_word);
		rtc_eeprom.writeBlock(b, (uint8_t *)first_data, 4);

		SerializeUint32(second_data, second_word);
		rtc_eeprom.writeBlock((b + 4), (uint8_t *)second_data, 4);
		dumpEEPROM(b, 16);

		b += 64;

		if (seg == 3)
		{
			second_word -= 10000;
		}
		else
		{
			second_word += 1000;
		}

		Serial.println();
		Serial.println();

		delay(1000);
	}

	delay(1000);

	int seg_addr[NUMBER_OF_EEPROM_SEGMENTS];
	uint64_t timestamps[NUMBER_OF_EEPROM_SEGMENTS];

	b = 0;

	for (int seg = 0; seg < 8; seg++)
	{
		first_word = 0;
		second_word = 0;

		Serial.println();
		Serial.print("Iteration\t\t");
		Serial.println(seg);
		Serial.print("Segment Start Address\t\t");
		Serial.println(b);

		rtc_eeprom.readBlock(b, (uint8_t *)page_1, 4);
		//Serial.println(page_1);
		rtc_eeprom.readBlock((b + 4), (uint8_t *)page_2, 4);
		//Serial.println(page_2);

		//Let's parse those blocks we just read so we can convert them back
		first_word = ParseUint32(page_1);
		second_word = ParseUint32(page_2);

		Serial.println();
		Serial.println();
		Serial.println(first_word);
		Serial.println(second_word);

		seg_addr[seg] = b;
		timestamps[seg] = join_64(first_word, second_word);

		reassembled_epoch = timestamps[seg];

		Serial.println(reassembled_epoch);

		b += 64;
	}
	int most_recent = std::max_element(timestamps, timestamps + NUMBER_OF_EEPROM_SEGMENTS) - timestamps;

	Serial.println("The most recent value is:");
	Serial.print("Array Index\t\t");
	Serial.println(most_recent);
	Serial.print("Offset\t\t");
	Serial.println((unsigned char)seg_addr[most_recent]);
	Serial.print("Data\t\t");
	Serial.println((unsigned char)timestamps[most_recent]);
	int proof = (timestamps[most_recent] - saved_second_word);

	Serial.println("The proof is:");
	Serial.println("If the below value is 3000, it worked!");
	Serial.println(proof);

	delay(300000);
}
*/
void SerializeUint32(unsigned char (&buf)[4], uint32_t val)
{
	buf[0] = val;
	buf[1] = val >> 8;
	buf[2] = val >> 16;
	buf[3] = val >> 24;
}

uint32_t ParseUint32(const char (&buf)[4])
{
	// This prevents buf[i] from being promoted to a signed int.
	uint32_t u0 = buf[0], u1 = buf[1], u2 = buf[2], u3 = buf[3];
	uint32_t uval = u0 | (u1 << 8) | (u2 << 16) | (u3 << 24);
	return uval;
}