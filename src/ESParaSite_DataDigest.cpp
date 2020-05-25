// ESParaSite_DataDigest.cpp

/* ESParasite Data Logger v0.9
        Authors: Andy (DocMadmag) Eakin

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

#include "ESParaSite.h"
#include "ESParaSite_DataDigest.h"
#include "ESParaSite_DataToJson.h"
#include "ESParaSite_SensorsCore.h"

typedef struct history history;

Queue fiveSecHistory(sizeof(history), 6, FIFO);
Queue thirtySecHistory(sizeof(history), 10, FIFO);
Queue threeHSecHistory(sizeof(history), 12, FIFO);

extern ESParaSite::printchamber chamberResource;
extern ESParaSite::optics opticsResource;
extern ESParaSite::ambient ambientResource;
extern ESParaSite::enclosure enclosureResource;
extern ESParaSite::statusData statusResource;
extern ESParaSite::configData configResource;
extern ESParaSite::sensorExists existsResource;

int fiveSecCycleCount = 0;
int thirtySecCycleCount = 0;
int threeHSecCycleCount = 0;

void ESParaSite::DataDigest::fillRow() {
  history fiveSecFill = {0};

  fiveSecFill.dataTimestamp = ESParaSite::Sensors::readRtcEpoch();
  fiveSecFill.ambientTempC = ambientResource.ambientTempC;
  fiveSecFill.ambientHumidity = ambientResource.ambientHumidity;
  fiveSecFill.chamberTempC = chamberResource.chamberTempC;
  fiveSecFill.chamberHumidity = chamberResource.chamberHumidity;
  fiveSecFill.ledTempC = opticsResource.ledTempC;
  fiveSecFill.screenTempC = opticsResource.screenTempC;
  fiveSecFill.ledOn = statusResource.isPrintingFlag;

  if (!fiveSecHistory.isFull()) {
    fiveSecHistory.push(&fiveSecFill);
    fiveSecCycleCount = FIVESECMAXELEMENT;
    ESParaSite::DataToJson::historyToJson();
    // ESParaSite::DataDigest::printRows();
  } else if (fiveSecHistory.isFull() && fiveSecCycleCount < FIVESECMAXELEMENT) {
    fiveSecHistory.drop();
    fiveSecHistory.push(&fiveSecFill);
    fiveSecCycleCount++;
    ESParaSite::DataToJson::historyToJson();
    // ESParaSite::DataDigest::printRows();
  } else if (fiveSecHistory.isFull() && fiveSecCycleCount == FIVESECMAXELEMENT) {
    fivesToThirty();
    fiveSecHistory.drop();
    fiveSecHistory.push(&fiveSecFill);
    ESParaSite::DataToJson::historyToJson();
    // ESParaSite::DataDigest::printRows();
    fiveSecCycleCount = 0;
  }
}

void fivesToThirty() {
  const int n = fiveSecHistory.getCount();
  float tempAHArray[] = {0};
  float tempATArray[] = {0};
  float tempCHArray[] = {0};
  float tempCTArray[] = {0};
  float tempLTArray[] = {0};
  float tempSTArray[] = {0};
  int tempLOArray[] = {0};

  history thirtySecFill;
  //Serial.print("Size of Queue: ");
  //Serial.print(fiveSecHistory.getCount());
  // Now we crack open all of the structs in the queue and reassemble them
  // into arrays so we can do math against them.
  for (int i = 0; i <= n; i++)
  {
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
  for (int i = 0; i < n; i++)
      sum += tempAHArray[i];

  thirtySecFill.ambientHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempATArray[i];

  thirtySecFill.ambientTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempCHArray[i];

  thirtySecFill.chamberHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempCTArray[i];

  thirtySecFill.chamberTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempLTArray[i];

  thirtySecFill.ledTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempSTArray[i];

  thirtySecFill.screenTempC = roundf((sum / n) * 100) / 100;

  // For the LED On value, we will set it on if more than half of the
  // intervals are on.
  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempLOArray[n];
  if (sum >= (n / 2)) {
    thirtySecFill.ledOn = 1;
  } else {
    thirtySecFill.ledOn = 0;
  }

  if (!thirtySecHistory.isFull()) {
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount = THIRTYSECMAXELEMENT;
  } else if (thirtySecHistory.isFull() && thirtySecCycleCount < THIRTYSECMAXELEMENT) {
    thirtySecHistory.drop();
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount++;
  } else if (thirtySecHistory.isFull() && thirtySecCycleCount == THIRTYSECMAXELEMENT) {
    thirtysToThreeH();
    thirtySecHistory.drop();
    thirtySecHistory.push(&thirtySecFill);
    thirtySecCycleCount = 0;
  }
}

void thirtysToThreeH() {
    const int n = thirtySecHistory.getCount();
    float tempAHArray[] = {0};
    float tempATArray[] = {0};
    float tempCHArray[] = {0};
    float tempCTArray[] = {0};
    float tempLTArray[] = {0};
    float tempSTArray[] = {0};
    int tempLOArray[] = {0};

    history threeHSecFill;

    // Now we crack open all of the structs in the queue and reassemble them
    // into arrays so we can do math against them.
    for (int i = 0; i <= n; i++)
    {
        history tempStruct;

        thirtySecHistory.peekIdx(&tempStruct, i);

        // Every 30 seconds we want to reduce the "five second" data points to a
        // mean and place that into a 30 seconds data set. First thing we will do is
        // grab the first timestamp.
        if (i == 0)
        {
            threeHSecFill.dataTimestamp = tempStruct.dataTimestamp;
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
  for (int i = 0; i < n; i++)
    sum += tempAHArray[i];

  threeHSecFill.ambientHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempATArray[i];

  threeHSecFill.ambientTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempCHArray[i];

  threeHSecFill.chamberHumidity = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempCTArray[i];

  threeHSecFill.chamberTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempLTArray[i];

  threeHSecFill.ledTempC = roundf((sum / n) * 100) / 100;

  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempSTArray[i];

  threeHSecFill.screenTempC = roundf((sum / n) * 100) / 100;

  // For the LED On value, we will set it on if more than half of the
  // intervals are on.
  sum = 0;
  for (int i = 0; i < n; i++)
    sum += tempLOArray[i];
  if (sum >= (n / 2)) {
    threeHSecFill.ledOn = 1;
  } else {
    threeHSecFill.ledOn = 0;
  }

  if (!threeHSecHistory.isFull()) {
    threeHSecHistory.push(&threeHSecFill);
  } else {
    threeHSecHistory.drop();
    threeHSecHistory.push(&threeHSecFill);
  }
}

void ESParaSite::DataDigest::printRows() {
  int i;
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

  Serial.println(F("300 Second Queue"));
  for (i = 0; i <= THREEHSECMAXELEMENT; i++) {
    history myRec3 = {0};

    threeHSecHistory.peekIdx(&myRec3, i);
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
}
