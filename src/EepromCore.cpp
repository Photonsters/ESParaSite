// RtcEepromCore.cpp

/* ESParasite Data Logger
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

#include <Arduino.h>
#include <I2C_eeprom.h>
#include <RtcDS3231.h>
#include <Wire.h>

#include "DebugUtils.h"
#include "ESParaSite.h"
#include "Eeprom.h"
#include "Sensors.h"
#include "Util.h"

/*     The AT24C32 EEPROM is structured with 4096 x 8 bit words. Writing is
   performed on the page boundaries of 128 x 32 byte pages. We will further
   divide each page into 8 'segments' of 4 bytes.
   The EEPROM is rated for 1 Million Write Cycles and we will write the
   values every 30 seconds of real time. If we write this data to the same
   page 24/7/365 we would burn out the EEPROM in 347 days or just under 1
   year.
   In order to extend the life of the EEPROM chip, we will Round Robin across
   the pages. The first page (Page 0)will be used to hold any "non-resettable"
   counters and will only be written at first boot or when we set the
   'first_on_timestamp' via the esps-uploader.py (or if we add new counters
   in the future).
   This will allow us to log 8 separate 32-bit values twice each minute for
   over 104 years (for a 32kbit EEPROM). At each write, we will use the first
   2 segments of each page for a 'write_timestamp'. Then, after writing the
   remining values, we will shift to the next page, spreading the writes out
   evenly across the remaining page memory cells.
   At boot time, we will read the 'write_timestamp' value from all pages and
   compare to find the MRU. Then, we will read in the remaining values from
   the MRU segment.
   (***not yet implemented) As a side effect, since the segments are only
   overwritten on the nth cycle, if we encounter read or write errors, we will
   fall back to the segment immediately preceding the MRU. We will also keep a
   bitmap of bad pages and skip writing and reading from those pages.

   https://github.com/Photonsters/ESParaSite/wiki/ESParaSite--EEPROM-Format
*/

#define NUMBER_OF_EEPROM_SEGMENTS (128)
#define SEGMENTS_PER_PAGE (8)
#define BYTES_PER_SEGMENT (4)
#define SECOND_PAGE_OFFSET (32)
#define MAX_EEPROM_FORMAT_VERSION (1)

#define DISPLAY_HEX
#define BLOCK_TO_LENGTH 16

// These values control the I2C address of each sensor. Some chips may use
// different addresses and I recommendutilizing the I2C scanner sketch at:
// https://gist.github.com/AustinSaintAubin/dc8abb2d168f5f7c27d65bb4829ca870
// to scan for your sensors if you are having any issues with communication.

// EEPROM Memory Sizes
// This should be set to 0x1000 due to how the library handles calculating
// PAGE_SIZE
// 24C256 = 0x8000(maximum address = 0x7FFF) common on EEPROM Modules
// 24C128 = 0x4000(maximum address = 0x3FFF)
// 24LC64 = 0x2000(maximum address = 0x1FFF)
// 24LC32 = 0x1000(maximum address = 0x0FFF) common on RTC Modules
#define MEMORY_SIZE 0x1000

// AT24Cxx EEPROM Address Range. Default (0x50 to 0x57)
#define RTC_EEPROM_BASE_ADDR (0x50)
#define RTC_EEPROM_MAX_ADDR (0x57)

uint32_t start, diff, totals = 0;

extern ESParaSite::enclosureData enclosure;
extern ESParaSite::eepromData eeprom;
extern ESParaSite::statusData status;
extern ESParaSite::machineData machine;

bool ESParaSite::Eeprom::initI2cEeprom() {
  // initialize AT24Cxx EEPROM
  Serial.println(F("Init AT24Cxx EEPROM..."));

  // There are 8 possible I2C addresses for the AT24xx eeprom. We will ping
  // each address and use the last one to respond.
  for (machine.eepromI2cAddress = RTC_EEPROM_BASE_ADDR;
       machine.eepromI2cAddress <= RTC_EEPROM_MAX_ADDR;
       machine.eepromI2cAddress++) {
    int8_t error = ESParaSite::Sensors::pingSensor(machine.eepromI2cAddress);
    if (error == 0) {
      Serial.println(F("OK!"));
      Serial.println();

      return true;
    }
    Serial.println();
  }
  Serial.println(F("NO EEPROM FOUND!"));
  Serial.println();
  return false;
}

void ESParaSite::Eeprom::initEeprom() {
  // Determine the size of the EEPROM and store that value
  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);
  dev_eeprom.begin();

  Serial.println();
  Serial.println("Determining EEPROM size");
  int16_t size = dev_eeprom.determineSize();
  if (size > 0) {
    Serial.print("SIZE: ");
    Serial.print(size);
    Serial.println(" kB");
    machine.eepromSizeBytes = (size * 1024);
  } else if (size == 0) {
    Serial.println("WARNING: Can't determine eeprom size");
  } else {
    Serial.println("ERROR: Can't find eeprom\nstopped...");
  }

  // Check the first page for proper format
  int8_t curPageAddr = 0;
  uint32_t readWords[8]{};

  for (int8_t i = 0; i < 8; i++) {
    char pageData[BYTES_PER_SEGMENT];

    dev_eeprom.readBlock(curPageAddr, reinterpret_cast<uint8_t *>(pageData),
                         BYTES_PER_SEGMENT);
    readWords[i] = ESParaSite::Util::ParseUint32(pageData);
    curPageAddr += (BYTES_PER_SEGMENT);
  }

  bool test1 = false;
  bool test2 = false;
  bool test3 = false;

  uint32_t curRtcEpoch = ESParaSite::Sensors::readRtcEpoch();

  // First test - Data at index [0] should be '0' if date and time is
  // currently prior to 03:14:07 on Tuesday, 19 January 2038.
  if (readWords[0] != 0) {
    if (curRtcEpoch <= 2147483647) {
      test1 = true;
    }
  }

  // Second test - Data at index [1] should be both less than the current
  // epoch time and should never exceed '2147483647', nor should the value
  // be 0, -1, null or prior to Sun 01 Jan 2017 12:00:01 AM UTC.

  if (readWords[1] >= curRtcEpoch || readWords[1] > 2147483647 ||
      readWords[1] == 0 || readWords[1] < 1483228801 || !readWords[1]) {
    test2 = true;
  }

  // Third test - The last page of the reserved segment holds the EEPROM
  // Format version number. The value should only be '0' if the rom is
  // initialized and should never be greater than 'MAX_EEPROM_FORMAT_VERSION'.

  if (static_cast<int>(readWords[7]) == 0 ||
      static_cast<int>(readWords[2]) > MAX_EEPROM_FORMAT_VERSION) {
    test3 = true;
  }

  if (test3) {
    Serial.print("WARNING: The EEPROM Format is Invalid. All reserved"
                 "values will be reset and all eeprom ");
    Serial.println("values will be set to '0'.");
    doEepromFormat(1);
  } else if (test2 || test1) {
    Serial.println("WARNING: The firstOnTimestamp is Invalid. "
                   "This value will be reset.");
    doEepromFormat(2);
  } else {
    Serial.print("EEPROM Format Valid - Version:\t");
    Serial.println(readWords[7]);
    Serial.println();
    eeprom.firstOnTimestamp = Util::join64(readWords[0], readWords[1]);
  }
#ifdef DEBUG_L3
  ESParaSite::Eeprom::dumpEEPROM(0, 32);
  delay(500);
#endif
}

void ESParaSite::Eeprom::doEepromFormat(uint8_t format_type) {
  // Routines to format the EEPROM device
  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);
  int8_t setFirstOn = 0;

  // Format Type 1 - Full erase We will zero the EEPROM and set the
  // FORMAT_VERSION field
  if (format_type == 1) {
    // "Magic Number Warning - See Above."
    dev_eeprom.setBlock(0, 0, machine.eepromSizeBytes);

    Serial.println("RTC EEPROM Zeroed");

#ifdef DEBUG_L3
    ESParaSite::Eeprom::dumpEEPROM(0, 4096);
    delay(500);
#endif

    unsigned char third_data[4];

    Serial.println("Writing EEPROM format version");
    ESParaSite::Util::SerializeUint32(third_data, MAX_EEPROM_FORMAT_VERSION);
    dev_eeprom.writeBlock((BYTES_PER_SEGMENT * 7),
                          reinterpret_cast<uint8_t *>(third_data), 4);

    setFirstOn = 1;
  } else if (format_type == 2) {
    // Format Type 2 - Do not zero the eeprom, only set the FIRST_ON value
    // to compile time
    setFirstOn = 1;
  }

  if (setFirstOn == 1) {
    Serial.println("Resetting First On Value To Compile Time");
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    time_t compile_time = (compiled + 946684800);

    uint32_t firstWord =
        (uint32_t)((compile_time & 0xFFFFFFFF00000000LL) >> 32);
    uint32_t secondWord = (uint32_t)(compile_time & 0xFFFFFFFFLL);

    unsigned char firstPart[4];
    unsigned char secondPart[4];

    ESParaSite::Util::SerializeUint32(firstPart, firstWord);
    dev_eeprom.writeBlock(0, reinterpret_cast<uint8_t *>(firstPart), 4);

    ESParaSite::Util::SerializeUint32(secondPart, secondWord);
    dev_eeprom.writeBlock(4, reinterpret_cast<uint8_t *>(secondPart), 4);

#ifdef DEBUG_L3
    ESParaSite::Eeprom::dumpEEPROM(0, 32);
    delay(500);
#endif
  }
}

int16_t ESParaSite::Eeprom::doEepromFirstRead() {
  // Read the EEPROM device and load values into variables
  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);
  int16_t curPageAddr = SECOND_PAGE_OFFSET;

  char firstPart[BYTES_PER_SEGMENT];
  char secondPart[BYTES_PER_SEGMENT];

  time_t latestTimestamp = 0;
  uint64_t latestTimestampSeg = 0;

  // Find Most Recently used EEPROM segment and latest write timestamp
  Serial.println("Looking for MRU EEPROM segment");

  for (int16_t seg = 1; seg < (machine.eepromSizeBytes / 32); seg++) {
    time_t readTimestamp = 0;
    uint32_t firstWord = 0;
    uint32_t secondWord = 0;

    dev_eeprom.readBlock(curPageAddr, reinterpret_cast<uint8_t *>(firstPart),
                         BYTES_PER_SEGMENT);
    dev_eeprom.readBlock((curPageAddr + BYTES_PER_SEGMENT),
                         reinterpret_cast<uint8_t *>(secondPart),
                         BYTES_PER_SEGMENT);

    firstWord = ESParaSite::Util::ParseUint32(firstPart);
    secondWord = ESParaSite::Util::ParseUint32(secondPart);

    readTimestamp = ESParaSite::Util::join64(firstWord, secondWord);

    if (readTimestamp != 0 && readTimestamp >= latestTimestamp) {
      latestTimestampSeg = (seg * 32);
      latestTimestamp = readTimestamp;
    }

    curPageAddr += (SEGMENTS_PER_PAGE * BYTES_PER_SEGMENT);

    Serial.print(".");
  }

  Serial.println();

  if (latestTimestamp == 0 || latestTimestampSeg == 0) {
    Serial.println("EEPROM Does not contain any valid data. Data logging"
                   " will begin at first segment.");
    eeprom.lastWriteTimestamp = status.rtcCurrentSecond;
    return (SECOND_PAGE_OFFSET);
  } else {

#ifdef DEBUG_L1
    Serial.print("The most recent write timestamp is:\t");
    Serial.println(latestTimestamp);
#endif

    eeprom.lastWriteTimestamp = latestTimestamp;
    return (latestTimestampSeg);
  }
}

uint8_t ESParaSite::Eeprom::doEepromRead(uint16_t pageAddr) {
  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);
  Serial.print("Reading EEPROM Values");

  for (int8_t page = 0; page < SEGMENTS_PER_PAGE; page++) {
    Serial.print(".");

    uint32_t firstWord = 0;
    uint32_t secondWord = 0;

    char firstPart[BYTES_PER_SEGMENT];
    char secondPart[BYTES_PER_SEGMENT];

    switch (page) {
    case 0:
      dev_eeprom.readBlock(pageAddr, reinterpret_cast<uint8_t *>(firstPart),
                           BYTES_PER_SEGMENT);
      dev_eeprom.readBlock((pageAddr + BYTES_PER_SEGMENT),
                           reinterpret_cast<uint8_t *>(secondPart),
                           BYTES_PER_SEGMENT);

      firstWord = ESParaSite::Util::ParseUint32(firstPart);
      secondWord = ESParaSite::Util::ParseUint32(secondPart);

      eeprom.lastWriteTimestamp =
          ESParaSite::Util::join64(firstWord, secondWord);
      break;
    case 4:
      dev_eeprom.readBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                           reinterpret_cast<uint8_t *>(firstPart),
                           BYTES_PER_SEGMENT);
      eeprom.eepromVatLifeSec = ESParaSite::Util::ParseUint32(firstPart);
      break;
    case 5:
      dev_eeprom.readBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                           reinterpret_cast<uint8_t *>(firstPart),
                           BYTES_PER_SEGMENT);
      eeprom.eepromLedLifeSec = ESParaSite::Util::ParseUint32(firstPart);
      break;
    case 6:
      dev_eeprom.readBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                           reinterpret_cast<uint8_t *>(firstPart),
                           BYTES_PER_SEGMENT);
      eeprom.eepromScreenLifeSec = ESParaSite::Util::ParseUint32(firstPart);
      break;
    default:
      break;
    }
  }

  eeprom.lastSegmentAddress = pageAddr;

  Serial.println();
  Serial.println("DONE!");
  Serial.println();

#ifdef DEBUG_L2
  // Print EEPROM Values to Serial Console
  Serial.print("eeprom.lastWriteTimestamp =  ");
  Serial.println(eeprom.lastWriteTimestamp);
  Serial.print("eeprom.eepromVatLifeSec =    ");
  Serial.println(eeprom.eepromVatLifeSec);
  Serial.print("eeprom.eepromLedLifeSec =    ");
  Serial.println(eeprom.eepromLedLifeSec);
  Serial.print("eeprom.eepromScreenLifeSec = ");
  Serial.println(eeprom.eepromScreenLifeSec);
  Serial.println();
#endif

  return 0;
}

uint8_t ESParaSite::Eeprom::doEepromWrite() {
  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);
  // ESParaSite::Sensors::dumpSensor(true);
  uint16_t pageAddr =
      eeprom.lastSegmentAddress + (BYTES_PER_SEGMENT * SEGMENTS_PER_PAGE);

  if (pageAddr >= machine.eepromSizeBytes) {
    Serial.print(F("Rolling over to first segment:\t"));
    pageAddr = SECOND_PAGE_OFFSET;
  }

  Serial.print(F("Writing EEPROM Values to segment at address:\t"));
  Serial.print(pageAddr);
  Serial.print(" (0x");
  Serial.print(pageAddr, HEX);
  Serial.println(")");

  for (uint8_t page = 0; page < SEGMENTS_PER_PAGE; page++) {
    Serial.print(".");

    uint32_t firstWord = 0;
    uint32_t secondWord = 0;

    unsigned char firstPart[BYTES_PER_SEGMENT];
    unsigned char secondPart[BYTES_PER_SEGMENT];

    switch (page) {
    case 0:
      firstWord =
          (uint32_t)((status.rtcCurrentSecond & 0xFFFFFFFF00000000LL) >> 32);
      secondWord = (uint32_t)(status.rtcCurrentSecond & 0xFFFFFFFFLL);

      ESParaSite::Util::SerializeUint32(firstPart, firstWord);
      dev_eeprom.writeBlock(pageAddr, reinterpret_cast<uint8_t *>(firstPart),
                            BYTES_PER_SEGMENT);

      ESParaSite::Util::SerializeUint32(secondPart, secondWord);
      dev_eeprom.writeBlock((pageAddr + BYTES_PER_SEGMENT),
                            reinterpret_cast<uint8_t *>(secondPart),
                            BYTES_PER_SEGMENT);

      break;
    case 2:
      ESParaSite::Util::SerializeUint32(firstPart, 0);
      dev_eeprom.writeBlock((pageAddr + BYTES_PER_SEGMENT),
                            reinterpret_cast<uint8_t *>(secondPart),
                            BYTES_PER_SEGMENT);
      break;
    case 3:
      ESParaSite::Util::SerializeUint32(firstPart, 0);
      dev_eeprom.writeBlock((pageAddr + BYTES_PER_SEGMENT),
                            reinterpret_cast<uint8_t *>(secondPart),
                            BYTES_PER_SEGMENT);
      break;
    case 4:

      ESParaSite::Util::SerializeUint32(firstPart, eeprom.eepromVatLifeSec);
      dev_eeprom.writeBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                            reinterpret_cast<uint8_t *>(firstPart),
                            BYTES_PER_SEGMENT);

      enclosure.vatLifeSec = eeprom.eepromVatLifeSec;
      break;
    case 5:

      ESParaSite::Util::SerializeUint32(firstPart, eeprom.eepromLedLifeSec);
      dev_eeprom.writeBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                            reinterpret_cast<uint8_t *>(firstPart),
                            BYTES_PER_SEGMENT);

      enclosure.ledLifeSec = eeprom.eepromLedLifeSec;
      break;
    case 6:

      ESParaSite::Util::SerializeUint32(firstPart, eeprom.eepromScreenLifeSec);
      dev_eeprom.writeBlock(pageAddr + (BYTES_PER_SEGMENT * page),
                            reinterpret_cast<uint8_t *>(firstPart),
                            BYTES_PER_SEGMENT);

      enclosure.lcdLifeSec = eeprom.eepromScreenLifeSec;
      break;
    case 7:
      ESParaSite::Util::SerializeUint32(firstPart, 0);
      dev_eeprom.writeBlock((pageAddr + BYTES_PER_SEGMENT),
                            reinterpret_cast<uint8_t *>(secondPart),
                            BYTES_PER_SEGMENT);
      break;
    default:
      break;
    }
  }

  eeprom.lastSegmentAddress = pageAddr;
  eeprom.lastWriteTimestamp = status.rtcCurrentSecond;

  Serial.println();
  Serial.println("DONE!");
  Serial.println();

#ifdef DEBUG_L3
  delay(10);
  Eeprom::dumpEEPROM(pageAddr, 64);
#endif

  return 0;
}

void ESParaSite::Eeprom::dumpEEPROM(uint16_t memoryAddress, uint16_t length) {

  I2C_eeprom dev_eeprom(machine.eepromI2cAddress, MEMORY_SIZE);

  Serial.print("\t ");
  for (int8_t x = 0; x < BLOCK_TO_LENGTH; x++) {
    if (x != 0) {
      Serial.print("   ");
    }
    Serial.print(x, HEX);
  }
  Serial.println();

  // block to defined length
  memoryAddress = memoryAddress / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;
  length = (length + BLOCK_TO_LENGTH - 1) / BLOCK_TO_LENGTH * BLOCK_TO_LENGTH;

  byte b = dev_eeprom.readByte(memoryAddress);
  for (uint8_t i = 0; i < length; i++) {
    char buf[6];
    if (memoryAddress % BLOCK_TO_LENGTH == 0) {
      if (i != 0) {
        Serial.println();
      }
      snprintf(buf, sizeof(buf), "%04X", memoryAddress);
      Serial.print(buf);
      Serial.print(":\t");
    }

    snprintf(buf, sizeof(buf), "%02X", b);

    Serial.print(buf);
    b = dev_eeprom.readByte(++memoryAddress);
    Serial.print("  ");
  }
  Serial.println();
  Serial.println();
}
