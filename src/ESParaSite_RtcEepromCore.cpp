// ESParaSite_RtcEepromCore.cpp

/* ESParasite Data Logger v0.9
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

#include "I2C_eeprom.h"
#include <Arduino.h>
#include <RtcDS3231.h>
#include <Wire.h>

#include "ESParaSite.h"
#include "ESParaSite_DebugUtils.h"
#include "ESParaSite_RtcEepromCore.h"
#include "ESParaSite_SensorsCore.h"
#include "ESParaSite_Util.h"


/*     The AT24C32 EEPROM is structured with 4096 x 8 bit words. Writing is
   performed on the page boundaries of 128 x 32 byte pages. The EEPROM is rated
   for 1 Million Write Cycles and we will write the values every 60 Seconds (1
   Minute) of real time. If we write this data 24/7/365 we would burn out the
   EEPROM in 694 days or just under 2 years. In order to extend the life of the
   EEPROM chip, we will further divide the pages into 16 x 8 page 'segments'.
   The first 'segment' will be used to hold any non-resettable counters and will
   only be written at first boot (or if we add new counters in the future). This
   will allow us to log 8 separate 32-bit values twice each minute for over 13
   years. At each write, we will use the first 2 pages of each 'segment' for a
   'write_timestamp'. Then, after writing the remining values, we will shift to
   the next segment, spreading the writes out evenly across the remaining 15
   segments memory cells. At boot time, we will read the 'write_timestamp' value
   from all 'segments' and compare to find the MRU. Then, we will read in the
   remaining pages from the MRU segment. As a side effect, since the segments
   are only overwritten on the 9th cycle. If we encounter read errors, we will
   fall back to the segment immediately preceding the MRU. (not yet implemented)
*/

#define NUMBER_OF_EEPROM_SEGMENTS (16)
#define PAGES_PER_SEGMENT (8)
#define BYTES_PER_PAGE (4)
#define FIRST_SEGMENT_OFFSET (32)
#define MAX_EEPROM_FORMAT_VERSION (1)

// for decimal display uncomment below two lines
// #define DISPLAY_DECIMAL
// #define BLOCK_TO_LENGTH 10

// for hex display uncomment below two lines
#define DISPLAY_HEX
#define BLOCK_TO_LENGTH 16

// total bytes can be accessed
// 24LC64 = 0x2000(maximum address = 0x1FFF)
// 24LC32 = 0x1000(maximum address = 0x0FFF)
#define MEMORY_SIZE 0x1000

// These values control the I2C address of each sensor. Some chips may use
// different addresses and I recommendutilizing the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

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

extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::rtcEepromData rtcEepromResource;
extern ESParaSite::statusData statusResource;

void ESParaSite::RtcEeprom::initRtcEeprom() {
  rtc_eeprom.begin();

  Serial.println();
  Serial.println("Determining EEPROM size");
  int size = rtc_eeprom.determineSize();
  if (size > 0) {
    Serial.print("SIZE: ");
    Serial.print(size);
    Serial.println(" kB");
  } else if (size == 0) {
    Serial.println("WARNING: Can't determine eeprom size");
  } else {
    Serial.println("ERROR: Can't find eeprom\nstopped...");
  }

  // Now we will check the first segment for proper format
  int current_seg_addr = 0;
  uint32_t read_words[8]{};

  for (int i = 0; i < 8; i++) {
    char page_data[BYTES_PER_PAGE];

    rtc_eeprom.readBlock(current_seg_addr,
                         reinterpret_cast<uint8_t *>(page_data),
                         BYTES_PER_PAGE);
    read_words[i] = ESParaSite::Util::ParseUint32(page_data);
    current_seg_addr += (BYTES_PER_PAGE);
  }

  bool test_1 = false;
  bool test_2 = false;
  bool test_3 = false;
  uint32_t current_rtc_epoch = ESParaSite::Sensors::readRtcEpoch();

  // First test - Data at index [0] should be '0' if date and time is
  // currently prior to 03:14:07 on Tuesday, 19 January 2038.
  if (read_words[0] != 0) {
    if (current_rtc_epoch <= 2147483647) {
      test_1 = true;
    }
  }

  // Second test - Data at index [1] should be both less than the current
  // epoch time and should never exceed '2147483647', nor should the value
  // be 0, -1, null or prior to Sun 01 Jan 2017 12:00:01 AM UTC.

  if (read_words[1] >= current_rtc_epoch || read_words[1] > 2147483647 ||
      read_words[1] == 0 || read_words[1] < 1483228801 || !read_words[1]) {
    test_2 = true;
  }

  // Third test - The last page of the reserved segment holds the EEPROM
  // Format version number. The value should only be '0' if the rom is
  // initialized and should never be greater than 'MAX_EEPROM_FORMAT_VERSION'.

  if (static_cast<int>(read_words[7]) == 0 ||
      static_cast<int>(read_words[2]) > MAX_EEPROM_FORMAT_VERSION) {
    test_3 = true;
  }

  if (test_3) {
    Serial.print("WARNING: The EEPROM Format is Invalid. All reserved"
                 "values will be reset and all eeprom ");
    Serial.println("values will be set to '0'.");
    do_eeprom_format(1);
  } else if (test_2 || test_1) {
    Serial.println("WARNING: The firstOnTimestamp is Invalid."
                   "This value will be reset to 0.");
    do_eeprom_format(2);
  } else {
    Serial.print("EEPROM Format Valid - Version:\t");
    Serial.println(read_words[7]);

    rtcEepromResource.firstOnTimestamp =
        Util::join_64(read_words[0], read_words[1]);
  }
}

int ESParaSite::RtcEeprom::doEepromFirstRead() {
  int current_seg_addr = FIRST_SEGMENT_OFFSET;

  char first_data[BYTES_PER_PAGE];
  char second_data[BYTES_PER_PAGE];

  time_t latest_timestamp = 0;
  uint64_t latest_timestamp_segment = 0;

  Serial.print("Looking for MRU EEPROM segment");

  for (int seg = 0; seg < (NUMBER_OF_EEPROM_SEGMENTS - 1); seg++) {
    time_t read_timestamp = 0;
    uint32_t first_part = 0;
    uint32_t second_part = 0;

    rtc_eeprom.readBlock(current_seg_addr,
                         reinterpret_cast<uint8_t *>(first_data),
                         BYTES_PER_PAGE);
    rtc_eeprom.readBlock((current_seg_addr + BYTES_PER_PAGE),
                         reinterpret_cast<uint8_t *>(second_data),
                         BYTES_PER_PAGE);

    first_part = ESParaSite::Util::ParseUint32(first_data);
    second_part = ESParaSite::Util::ParseUint32(second_data);

    read_timestamp = ESParaSite::Util::join_64(first_part, second_part);

    if (read_timestamp != 0 && read_timestamp >= latest_timestamp) {
      latest_timestamp_segment = (seg + 1);
      latest_timestamp = read_timestamp;
    }

    current_seg_addr += (PAGES_PER_SEGMENT * BYTES_PER_PAGE);

    Serial.print(".");
  }

  Serial.println();

  if (latest_timestamp == 0 || latest_timestamp_segment == 0) {
    Serial.println("EEPROM Does not contain any valid data. Data logging"
                   " will begin at first segment.");
    rtcEepromResource.lastWriteTimestamp =
        statusResource.rtcCurrentSecond;
    return (FIRST_SEGMENT_OFFSET);
  } else {
    Serial.print("The most recent write timestamp is:\t");
    Serial.println(latest_timestamp);
    rtcEepromResource.lastWriteTimestamp = latest_timestamp;
    return (latest_timestamp_segment * (PAGES_PER_SEGMENT * BYTES_PER_PAGE));
  }
}

uint8_t ESParaSite::RtcEeprom::doEepromRead(uint16_t segment_addr) {
  Serial.print("Reading EEPROM Values");

  for (int page = 0; page < PAGES_PER_SEGMENT; page++) {
    Serial.print(".");

    uint32_t first_part = 0;
    uint32_t second_part = 0;

    char page_1[BYTES_PER_PAGE];
    char page_2[BYTES_PER_PAGE];

    switch (page) {
    case 0:
      rtc_eeprom.readBlock(segment_addr, reinterpret_cast<uint8_t *>(page_1),
                           BYTES_PER_PAGE);
      rtc_eeprom.readBlock((segment_addr + BYTES_PER_PAGE),
                           reinterpret_cast<uint8_t *>(page_2), BYTES_PER_PAGE);

      first_part = ESParaSite::Util::ParseUint32(page_1);
      second_part = ESParaSite::Util::ParseUint32(page_2);

      rtcEepromResource.lastWriteTimestamp =
          ESParaSite::Util::join_64(first_part, second_part);
      break;
    case 4:
      rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page),
                           reinterpret_cast<uint8_t *>(page_1), BYTES_PER_PAGE);
      rtcEepromResource.eepromVatLifeSec =
          ESParaSite::Util::ParseUint32(page_1);
      break;
    case 5:
      rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page),
                           reinterpret_cast<uint8_t *>(page_1), BYTES_PER_PAGE);
      rtcEepromResource.eepromLedLifeSec =
          ESParaSite::Util::ParseUint32(page_1);
      break;
    case 6:
      rtc_eeprom.readBlock(segment_addr + (BYTES_PER_PAGE * page),
                           reinterpret_cast<uint8_t *>(page_1), BYTES_PER_PAGE);
      rtcEepromResource.eepromScreenLifeSec =
          ESParaSite::Util::ParseUint32(page_1);
      break;
    default:
      break;
    }
  }

  rtcEepromResource.lastSegmentAddress = segment_addr;

  Serial.println();
  Serial.println("DONE!");
  Serial.println();
  return 0;
}

uint8_t ESParaSite::RtcEeprom::doEepromWrite() {
  // ESParaSite::Sensors::dumpSensor(true);
  uint16_t segment_addr = rtcEepromResource.lastSegmentAddress +
                          (BYTES_PER_PAGE * PAGES_PER_SEGMENT);
  Serial.println("");
  
  if (segment_addr >= (static_cast<uint16_t>(MEMORY_SIZE) / 8)) {
    Serial.print(F("Rolling over to first segment:\t"));
    segment_addr = FIRST_SEGMENT_OFFSET;
  }

  Serial.print(F("Writing EEPROM Values to segment:\t"));
  Serial.println(segment_addr);

  for (uint8_t page = 0; page < PAGES_PER_SEGMENT; page++) {
    Serial.print(".");

    uint32_t first_part = 0;
    uint32_t second_part = 0;

    unsigned char page_1[BYTES_PER_PAGE];
    unsigned char page_2[BYTES_PER_PAGE];

    switch (page) {
    case 0:
      first_part = (uint32_t)(
          (statusResource.rtcCurrentSecond & 0xFFFFFFFF00000000LL) >> 32);
      second_part =
          (uint32_t)(statusResource.rtcCurrentSecond & 0xFFFFFFFFLL);

      ESParaSite::Util::SerializeUint32(page_1, first_part);
      rtc_eeprom.writeBlock(segment_addr, reinterpret_cast<uint8_t *>(page_1),
                            BYTES_PER_PAGE);

      ESParaSite::Util::SerializeUint32(page_2, second_part);
      rtc_eeprom.writeBlock((segment_addr + BYTES_PER_PAGE),
                            reinterpret_cast<uint8_t *>(page_2),
                            BYTES_PER_PAGE);

      break;
    case 4:

      ESParaSite::Util::SerializeUint32(page_1,
                                        rtcEepromResource.eepromVatLifeSec);
      rtc_eeprom.writeBlock(segment_addr + (BYTES_PER_PAGE * page),
                            reinterpret_cast<uint8_t *>(page_1),
                            BYTES_PER_PAGE);

      enclosureResource.vatLifeSec = rtcEepromResource.eepromVatLifeSec;
      break;
    case 5:

      ESParaSite::Util::SerializeUint32(page_1,
                                        rtcEepromResource.eepromLedLifeSec);
      rtc_eeprom.writeBlock(segment_addr + (BYTES_PER_PAGE * page),
                            reinterpret_cast<uint8_t *>(page_1),
                            BYTES_PER_PAGE);

      enclosureResource.ledLifeSec = rtcEepromResource.eepromLedLifeSec;
      break;
    case 6:

      ESParaSite::Util::SerializeUint32(
          page_1, rtcEepromResource.eepromScreenLifeSec);
      rtc_eeprom.writeBlock(segment_addr + (BYTES_PER_PAGE * page),
                            reinterpret_cast<uint8_t *>(page_1),
                            BYTES_PER_PAGE);

      enclosureResource.lcdLifeSec = rtcEepromResource.eepromScreenLifeSec;
      break;
    default:
      break;
    }
  }

  rtcEepromResource.lastSegmentAddress = segment_addr;
  rtcEepromResource.lastWriteTimestamp = statusResource.rtcCurrentSecond;

  Serial.println();
  Serial.println(F("DONE!"));
  Serial.println();
  
// "Magic Number Warning" - This logic may be flawed for chips other than the
// AT24C32 and AT24C64. It divides the reported ROM size by 8-bit pages, but
// other ROMs could be organized differently.
#ifdef DEBUG_L2
  RtcEeprom::dumpEEPROM(segment_addr, ((static_cast<uint16_t>(MEMORY_SIZE)) /
                                       (BYTES_PER_PAGE * PAGES_PER_SEGMENT)));
#endif

  return 0;
}

void do_eeprom_format(uint8_t format_type) {
  int set_first_on = 0;

  // Format Type 1 -
  if (format_type == 1) {
    // "Magic Number Warning - See Above."
    rtc_eeprom.setBlock(0, 0, ((static_cast<int>(MEMORY_SIZE)) / 8));

    Serial.println("RTC EEPROM Zeroed");

    // "Magic Number Warning - See Above."
    ESParaSite::RtcEeprom::dumpEEPROM(0, ((static_cast<int>(MEMORY_SIZE)) / 8));

    delay(500);

    unsigned char third_data[4];

    Serial.println("Writing EEPROM format version");
    ESParaSite::Util::SerializeUint32(third_data, MAX_EEPROM_FORMAT_VERSION);
    rtc_eeprom.writeBlock((BYTES_PER_PAGE * 7),
                          reinterpret_cast<uint8_t *>(third_data), 4);

    set_first_on = 1;
  } else if (format_type == 2) {
    set_first_on = 1;
  }

  if (set_first_on == 1) {
    Serial.println("Resetting first on value to current second");
    Serial.print("The current epoch is:\t");
    time_t current_epoch = ESParaSite::Sensors::readRtcEpoch();
    Serial.println(current_epoch);

    uint32_t first_part =
        (uint32_t)((current_epoch & 0xFFFFFFFF00000000LL) >> 32);
    uint32_t second_part = (uint32_t)(current_epoch & 0xFFFFFFFFLL);

    unsigned char first_data[4];
    unsigned char second_data[4];

    ESParaSite::Util::SerializeUint32(first_data, first_part);
    rtc_eeprom.writeBlock(0, reinterpret_cast<uint8_t *>(first_data), 4);

    ESParaSite::Util::SerializeUint32(second_data, second_part);
    rtc_eeprom.writeBlock(4, reinterpret_cast<uint8_t *>(second_data), 4);

    // "Magic Number Warning - See Above."
    ESParaSite::RtcEeprom::dumpEEPROM(0, (static_cast<int>(MEMORY_SIZE)) / 8);
  }
}

void ESParaSite::RtcEeprom::dumpEEPROM(uint16_t memoryAddress,
                                       uint16_t length) {
#ifdef DISPLAY_DECIMAL
  Serial.print("\t  ");
#endif
#ifdef DISPLAY_HEX
  Serial.print("\t ");
#endif
  for (int x = 0; x < BLOCK_TO_LENGTH; x++) {
    if (x != 0) {
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
  for (unsigned int i = 0; i < length; i++) {
    char buf[6];
    if (memoryAddress % BLOCK_TO_LENGTH == 0) {
      if (i != 0) {
        Serial.println();
      }
#ifdef DISPLAY_DECIMAL
      snprintf(buf, sizeof(buf), "%05d", memoryAddress);
#endif
#ifdef DISPLAY_HEX
      snprintf(buf, sizeof(buf), "%04X", memoryAddress);
#endif
      Serial.print(buf);
      Serial.print(":\t");
    }
#ifdef DISPLAY_DECIMAL
    snprintf(buf, sizeof(buf), "%03d", b);
#endif
#ifdef DISPLAY_HEX
    snprintf(buf, sizeof(buf), "%02X", b);
#endif
    Serial.print(buf);
    b = rtc_eeprom.readByte(++memoryAddress);
    Serial.print("  ");
  }
  Serial.println();
}
