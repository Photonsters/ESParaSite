/* ESParasite Data Logger v0.1
	Authors: Andy (DocMadmag) Eakin
	
	Please see /ATTRIB for full credits and OSS License Info
  	Please see /LIBRARIES for necessary libraries
  	Please see /VERSION for Hstory
	
	All Derived Content is subject to the most restrictive licence of it's source.
	
	All Original content is free and unencumbered software released into the public domain.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_MLX90614.h>
#include <dht.h>
#include <EepromAT24C32.h>
#include <RtcDateTime.h>
#include <RtcDS3231.h>
#include <RtcDS3234.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <ThreeWire.h>

const char* ssid     = "yourwifinetwork";
const char* password = "yourwifipassword";

ESP8266WebServer server(80);

//Initialize remaining sensors
dht12 DHT(0x5c);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SI1145 uv = Adafruit_SI1145();
RtcDS3231<TwoWire> Rtc(Wire);

//Global Variables
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String webString = "";   // String to display
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor
float dhtHumidity; // Values read from DHT12 sensor
float dhtTemp;  // Values read from DHT12 sensor
float siUVindex; //UV Index
float siVis; //Visible Light Value
float siIR; //IR Value


void handle_root() {
  server.send(200, "text/plain", "Hello from the weather esp8266, read from /temp or /humidity");
  delay(100);
}

void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("ESParasite Data Logging Server");
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //http server config
  server.on("/", handle_root);

  server.on("/chamberTemp", []() { // if you add this subdirectory to your webserver call, you get text below :)
    getdhttemp();       // read sensor
    webString = "Print Chamber Temperature: " + String((int)dhtTemp) + " C"; // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });

  server.on("/chamberHumidity", []() { // if you add this subdirectory to your webserver call, you get text below :)
    getdhttemp();           // read sensor
    webString = "Print Chamber Humidity: " + String((int)dhtHumidity) + "%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
  });

  server.on("/uvledStatus", []() { // if you add this subdirectory to your webserver call, you get text below :)
    getuv();           // read sensor
    webString = "UV Index: " + String((int)siUVindex) + "%";
    server.send(200, "text/plain", webString);               // send to someones browser when asked
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.println();
  Serial.println();

  // initialize I2C bus
  Serial.println("Initialize I2C bus");
  Wire.begin(0, 2);
  Serial.println("OK!");
  Serial.println();

  // initialize DHT12 temperature sensor
  unsigned long b = micros();
  dht::ReadStatus chk = DHT.read();
  unsigned long e = micros();

  Serial.print(F("Read DHT12 sensor: "));
  switch (chk)
  {
    case dht::OK:
      Serial.print(F("OK, took "));
      Serial.print (e - b); Serial.print(F(" usec, "));
      break;
    case dht::ERROR_CHECKSUM:
      Serial.println(F("Checksum error"));
      break;
    case dht::ERROR_TIMEOUT:
      Serial.println(F("Timeout error"));
      break;
    case dht::ERROR_CONNECT:
      Serial.println(F("Connect error"));
      break;
    case dht::ERROR_ACK_L:
      Serial.println(F("AckL error"));
      break;
    case dht::ERROR_ACK_H:
      Serial.println(F("AckH error"));
      break;
    default:
      Serial.println(F("Unknown error"));
      break;
  }
  Serial.println();

  // initialize UV sensor
  Serial.println("Read SI1145 sensor");
  if (! uv.begin()) {
    Serial.println("SI1145 Initialization Failure");
    while (1);
  }
  Serial.println("OK!");
  Serial.println();

  // initialize MLX temperature sensor
  Serial.println("Read MLX90614 sensor");
  if (! mlx.begin()) {
    Serial.println("MLX90614 Initialization Failure");
    while (1);
  }
  Serial.println("OK!");
  Serial.println();

  // initialize DS3231 RTC
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Cuases:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  //Dump all Sensor data to Serial
  Serial.println();
  Serial.println("============================================================");
  Serial.println();
  Serial.println("DS3231 Real-Time Clock Timestamp and Temperature:");
  getrtcdata ();
  Serial.println();
  Serial.println("DHT12 Print Chamber Environmental Data:");
  getdhttemp();
  Serial.println();
  Serial.println("SI1145 UV and Light Sensor Data:");
  getuv();
  Serial.println();
  Serial.println("MLX90614 Temp Sensor Data:");
  getmlxtemp();
}

void loop(void)
{
  server.handleClient();
}

void getdhttemp() {
  Serial.print(F("Humidity: "));
  Serial.print((float)DHT.getHumidity() / (float)10);
  Serial.print(F("%, "));

  Serial.print(F(". Temperature (degrees C): "));
  dhtTemp = ((float)DHT.getTemperature() / (float)10);
  Serial.print(((int)dhtTemp));

  Serial.print(F(", Dew Point (degrees C): "));
  Serial.println(DHT.dewPoint());
}

void getuv() {
  Serial.println("===================");
  Serial.print("Vis: "); Serial.println(uv.readVisible());
  Serial.print("IR: "); Serial.println(uv.readIR());

  // Uncomment if you have an IR LED attached to LED pin!
  //Serial.print("Prox: "); Serial.println(uv.readProx());

  float UVindex = uv.readUV();
  // the index is multiplied by 100 so to get the
  // integer index, divide by 100!
  UVindex /= 100.0;
  Serial.print("UV: ");  Serial.println(UVindex);
  delay(1000);
}

void getmlxtemp() {
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC());
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF());
  Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");

  Serial.println();
  delay(500);
}

void getrtcdata () {
  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Cuases:
      //    1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
    }
  }

  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  Serial.println();

  RtcTemperature temp = Rtc.GetTemperature();
  temp.Print(Serial);
  // you may also get the temperature as a float and print it
  // Serial.print(temp.AsFloatDegC());
  Serial.println("C");

  delay(10000); // ten seconds
}

int convertCtoF(int temp_c)  {
  int temp_f;
  temp_f = ((int)round(1.8 * temp_c + 32));
  return temp_f;
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime & dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}
