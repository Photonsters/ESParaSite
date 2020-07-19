// DataDigest.cpp

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
#include <cppQueue.h>

#include "DataDigest.h"
#include "ESParaSite.h"
#include "Http.h"
#include "Json.h"
#include "Sensors.h"

typedef struct history history;

Queue fiveSecHistory(sizeof(history), 6, FIFO);
Queue thirtySecHistory(sizeof(history), 10, FIFO);
Queue fiveMinHistory(sizeof(history), 12, FIFO);
Queue oneHourHistory(sizeof(history), 12, FIFO);

extern ESParaSite::chamberData chamber;
extern ESParaSite::opticsData optics;
extern ESParaSite::ambientData ambient;
extern ESParaSite::enclosureData enclosure;
extern ESParaSite::statusData status;
extern ESParaSite::configData config;
extern ESParaSite::sensorExists exists;

int8_t fiveSecCycleCount = 0;
int8_t thirtySecCycleCount = 0;
int8_t fiveMinCycleCount = 0;
int8_t oneHourCycleCount = 0;

void ESParaSite::DataDigest::fillRow() {
  history fiveSecFill = {0};

  fiveSecFill.dataTimestamp = ESParaSite::Sensors::readRtcEpoch();
  fiveSecFill.ambientTempC = ambient.ambientTempC;
  fiveSecFill.ambientHumidity = ambient.ambientHumidity;
  fiveSecFill.chamberTempC = chamber.chamberTempC;
  fiveSecFill.ledTempC = optics.ledTempC;
  fiveSecFill.screenTempC = optics.screenTempC;
  fiveSecFill.ledOn = status.isPrintingFlag;

  if (!fiveSecHistory.isFull()) {
    fiveSecHistory.push(&fiveSecFill);
    fiveSecCycleCount = FIVESECMAXELEMENT;
    // ESParaSite::DataToJson::getJsonHistory();
    // ESParaSite::DataDigest::printRows();
  } else if (fiveSecHistory.isFull() && fiveSecCycleCount < FIVESECMAXELEMENT) {
    fiveSecHistory.drop();
    fiveSecHistory.push(&fiveSecFill);
    fiveSecCycleCount++;
    // ESParaSite::DataToJson::getJsonHistory();
    // ESParaSite::DataDigest::printRows();
  } else if (fiveSecHistory.isFull() &&
             fiveSecCycleCount == FIVESECMAXELEMENT) {
    fivesToThirty();
    fiveSecHistory.drop();
    fiveSecHistory.push(&fiveSecFill);
    // ESParaSite::DataToJson::getJsonHistory();
    // ESParaSite::DataDigest::printRows();
    fiveSecCycleCount = 0;
  }
}

void fivesToThirty() {
  const int8_t n = fiveSecHistory.getCount();
  int16_t tempAHArray[] = {0};
  int16_t tempATArray[] = {0};
  int16_t tempCHArray[] = {0};
  int16_t tempCTArray[] = {0};
  int16_t tempLTArray[] = {0};
  int16_t tempSTArray[] = {0};
  int8_t tempLOArray[] = {0};

  history thirtySecFill;
  // Serial.print("Size of Queue: ");
  // Serial.print(fiveSecHistory.getCount());
  // Now we crack open all of the structs in the queue and reassemble them
  // into arrays so we can do math against them.
  for (int8_t i = 0; i <= n; i++) {
    history tempStruct;
    fiveSecHistory.peekIdx(&tempStruct, i);

    // Every 30 seconds we want to reduce the "five second" data points to a
    // mean and place that into a 30 seconds data set. First thing we will do is
    // grab the first timestamp.
    if (i == 0) {
      thirtySecFill.dataTimestamp = tempStruct.dataTimestamp;
    }

    tempAHArray[i] = tempStruct.ambientHumidity;
    tempATArray[i] = tempStruct.ambientTempC;
    tempCHArray[i] = tempStruct.chamberHumidity;
    tempCTArray[i] = tempStruct.chamberTempC;
    tempLTArray[i] = tempStruct.ledTempC;
    tempSTArray[i] = tempStruct.screenTempC;
    tempLOArray[i] = tempStruct.ledOn;
  }

  // For most values, we will take the mathmatical mean.
  float sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempAHArray[i];

  thirtySecFill.ambientHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempATArray[i];

  thirtySecFill.ambientTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCHArray[i];

  thirtySecFill.chamberHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCTArray[i];

  thirtySecFill.chamberTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLTArray[i];

  thirtySecFill.ledTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempSTArray[i];

  thirtySecFill.screenTempC = roundf((sum / n) * 100) / 100;

  // For the LED On value, we will set it on if more than half of the
  // intervals are on.
  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLOArray[n];
  if (sum >= (n / 2)) {
    thirtySecFill.ledOn = 1;
  } else {
    thirtySecFill.ledOn = 0;
  }

  if (!thirtySecHistory.isFull()) {
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount = THIRTYSECMAXELEMENT;
  } else if (thirtySecHistory.isFull() &&
             thirtySecCycleCount < THIRTYSECMAXELEMENT) {
    thirtySecHistory.drop();
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount++;
  } else if (thirtySecHistory.isFull() &&
             thirtySecCycleCount == THIRTYSECMAXELEMENT) {
    thirtysTofiveMin();
    thirtySecHistory.drop();
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount = 0;
  }
}

void thirtysTofiveMin() {
  const int8_t n = thirtySecHistory.getCount();
  float tempAHArray[] = {0};
  float tempATArray[] = {0};
  float tempCHArray[] = {0};
  float tempCTArray[] = {0};
  float tempLTArray[] = {0};
  float tempSTArray[] = {0};
  int8_t tempLOArray[] = {0};

  history fiveMinFill;

  // Now we crack open all of the structs in the queue and reassemble them
  // into arrays so we can do math against them.
  for (int8_t i = 0; i <= n; i++) {
    history tempStruct;

    thirtySecHistory.peekIdx(&tempStruct, i);

    // Every 30 seconds we want to reduce the "five second" data points to a
    // mean and place that into a 30 seconds data set. First thing we will do is
    // grab the first timestamp.
    if (i == 0) {
      fiveMinFill.dataTimestamp = tempStruct.dataTimestamp;
    }
    tempAHArray[i] = tempStruct.ambientHumidity;
    tempATArray[i] = tempStruct.ambientTempC;
    tempCHArray[i] = tempStruct.chamberHumidity;
    tempCTArray[i] = tempStruct.chamberTempC;
    tempLTArray[i] = tempStruct.ledTempC;
    tempSTArray[i] = tempStruct.screenTempC;
    tempLOArray[i] = tempStruct.ledOn;
  }

  // For most values, we will take the mathmatical mean.
  float sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempAHArray[i];

  fiveMinFill.ambientHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempATArray[i];

  fiveMinFill.ambientTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCHArray[i];

  fiveMinFill.chamberHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCTArray[i];

  fiveMinFill.chamberTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLTArray[i];

  fiveMinFill.ledTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempSTArray[i];

  fiveMinFill.screenTempC = roundf((sum / n) * 100) / 100;

  // For the LED On value, we will set it on if more than half of the
  // intervals are on.
  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLOArray[i];
  if (sum >= (n / 2)) {
    fiveMinFill.ledOn = 1;
  } else {
    fiveMinFill.ledOn = 0;
  }

  if (!fiveMinHistory.isFull()) {
    fiveMinHistory.push(&fiveMinFill);
    fiveMinCycleCount = FIVEMINMAXELEMENT;
  } else if (fiveMinHistory.isFull() && fiveMinCycleCount < FIVEMINMAXELEMENT) {
    fiveMinHistory.drop();
    fiveMinHistory.push(&fiveMinFill);
    fiveMinCycleCount++;
  } else if (fiveMinHistory.isFull() &&
             fiveMinCycleCount == FIVEMINMAXELEMENT) {
    fiveMinToOneHour();
    fiveMinHistory.drop();
    fiveMinHistory.push(&fiveMinFill);
    fiveMinCycleCount = 0;
  }
}

void fiveMinToOneHour() {
  const int8_t n = fiveMinHistory.getCount();
  float tempAHArray[] = {0};
  float tempATArray[] = {0};
  float tempCHArray[] = {0};
  float tempCTArray[] = {0};
  float tempLTArray[] = {0};
  float tempSTArray[] = {0};
  int8_t tempLOArray[] = {0};

  history oneHourFill;

  // Now we crack open all of the structs in the queue and reassemble them
  // into arrays so we can do math against them.
  for (int8_t i = 0; i <= n; i++) {
    history tempStruct;

    fiveMinHistory.peekIdx(&tempStruct, i);

    // Every 30 seconds we want to reduce the "five second" data points to a
    // mean and place that into a 30 seconds data set. First thing we will do is
    // grab the first timestamp.
    if (i == 0) {
      oneHourFill.dataTimestamp = tempStruct.dataTimestamp;
    }
    tempAHArray[i] = tempStruct.ambientHumidity;
    tempATArray[i] = tempStruct.ambientTempC;
    tempCHArray[i] = tempStruct.chamberHumidity;
    tempCTArray[i] = tempStruct.chamberTempC;
    tempLTArray[i] = tempStruct.ledTempC;
    tempSTArray[i] = tempStruct.screenTempC;
    tempLOArray[i] = tempStruct.ledOn;
  }

  // For most values, we will take the mathmatical mean.
  float sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempAHArray[i];

  oneHourFill.ambientHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempATArray[i];

  oneHourFill.ambientTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCHArray[i];

  oneHourFill.chamberHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempCTArray[i];

  oneHourFill.chamberTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLTArray[i];

  oneHourFill.ledTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempSTArray[i];

  oneHourFill.screenTempC = roundf((sum / n) * 100) / 100;

  // For the LED On value, we will set it on if more than half of the
  // intervals are on.
  sum = 0;
  for (int8_t i = 0; i < n; i++)
    sum += tempLOArray[i];
  if (sum >= (n / 2)) {
    oneHourFill.ledOn = 1;
  } else {
    oneHourFill.ledOn = 0;
  }

  if (!oneHourHistory.isFull()) {
    oneHourHistory.push(&oneHourFill);
  } else {
    oneHourHistory.drop();
    oneHourHistory.push(&oneHourFill);
  }
}

void ESParaSite::DataDigest::printRows() {
  int8_t i;
  Serial.println(fiveSecCycleCount);
  Serial.println(F("5 Second Queue"));
  for (i = 0; i <= FIVESECMAXELEMENT; i++) {
    history myRec;

    fiveSecHistory.peekIdx(&myRec, i);
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(myRec.dataTimestamp);
    Serial.print(" : ");
    Serial.print(myRec.ambientTempC);
    Serial.print(" : ");
    Serial.print(myRec.ambientHumidity);
    Serial.print(" : ");
    Serial.print(myRec.chamberTempC);
    Serial.print(" : ");
    Serial.print(myRec.chamberHumidity);
    Serial.print(" : ");
    Serial.print(myRec.ledTempC);
    Serial.print(" : ");
    Serial.print(myRec.screenTempC);
    Serial.print(" : ");
    Serial.println(myRec.ledOn);
  }

  Serial.println(thirtySecCycleCount);
  Serial.println(F("30 Second Queue"));

  for (i = 0; i <= THIRTYSECMAXELEMENT; i++) {
    history myRec2 = {0};

    thirtySecHistory.peekIdx(&myRec2, i);
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(myRec2.dataTimestamp);
    Serial.print(" : ");
    Serial.print(myRec2.ambientTempC);
    Serial.print(" : ");
    Serial.print(myRec2.ambientHumidity);
    Serial.print(" : ");
    Serial.print(myRec2.chamberTempC);
    Serial.print(" : ");
    Serial.print(myRec2.chamberHumidity);
    Serial.print(" : ");
    Serial.print(myRec2.ledTempC);
    Serial.print(" : ");
    Serial.print(myRec2.screenTempC);
    Serial.print(" : ");
    Serial.println(myRec2.ledOn);
  }

  Serial.println(F("5 Minute Queue"));
  for (i = 0; i <= FIVEMINMAXELEMENT; i++) {
    history myRec3 = {0};

    fiveMinHistory.peekIdx(&myRec3, i);
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(myRec3.dataTimestamp);
    Serial.print(" : ");
    Serial.print(myRec3.ambientTempC);
    Serial.print(" : ");
    Serial.print(myRec3.ambientHumidity);
    Serial.print(" : ");
    Serial.print(myRec3.chamberTempC);
    Serial.print(" : ");
    Serial.print(myRec3.chamberHumidity);
    Serial.print(" : ");
    Serial.print(myRec3.ledTempC);
    Serial.print(" : ");
    Serial.print(myRec3.screenTempC);
    Serial.print(" : ");
    Serial.println(myRec3.ledOn);
  }

  Serial.println(F("1 Hour Queue"));
  for (i = 0; i <= FIVEMINMAXELEMENT; i++) {
    history myRec4 = {0};

    fiveMinHistory.peekIdx(&myRec4, i);
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(myRec4.dataTimestamp);
    Serial.print(" : ");
    Serial.print(myRec4.ambientTempC);
    Serial.print(" : ");
    Serial.print(myRec4.ambientHumidity);
    Serial.print(" : ");
    Serial.print(myRec4.chamberTempC);
    Serial.print(" : ");
    Serial.print(myRec4.chamberHumidity);
    Serial.print(" : ");
    Serial.print(myRec4.ledTempC);
    Serial.print(" : ");
    Serial.print(myRec4.screenTempC);
    Serial.print(" : ");
    Serial.println(myRec4.ledOn);
  }
}
