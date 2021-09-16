// Humidity and Temperature Sensor / Fan Controller
//
// Rich Freedman 2021
//
// USES DHT22 Humidity / Temperature Sensor and an external power strip with relay ("IOT Relay" from Digital Loggers)
//
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <Timezone.h> // https://github.com/JChristensen/Timezone

#include "DHT.h"

#include "NTPTime.h"


// =========== OLED Display Stuff =============
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Hardware SPI (slower than Software API ?)
#define OLED_MOSI  13
#define OLED_CLK   14
#define OLED_DC     2
#define OLED_CS    15
#define OLED_RESET  4

// Software SPI? Faster?
// #define OLED_MOSI  13
// #define OLED_CLK   14
// #define OLED_DC    12
// #define OLED_CS     5
// #define OLED_RESET  4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// ======== End OLED Display ==================

// !!!! TODO - move DHT to D3 to stop conflict with OLED


#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

const char *ssid     = "mch_private";
const char *password = "torbertcoryell";

long loopCount = 0;

const int CHARS_PER_LINE = 16;
const int LINE_HEIGHT_PX = 16;
const int LINE_1 = 0;
const int LINE_2 = LINE_1 + LINE_HEIGHT_PX;
const int LINE_3 = LINE_2 + LINE_HEIGHT_PX;
const int LINE_4 = LINE_3 + LINE_HEIGHT_PX;

WiFiUDP ntpUDP;
 
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// NTPClient timeClient(ntpUDP);
 
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
int GTMOffset = 0; // SET TO UTC TIME
NTPClient timeClient(ntpUDP, "pool.ntp.org", GTMOffset*60*60, 60*60*1000);

 // US Eastern Timezone
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

int temp = -999;
int humidity = -999;

unsigned long lastTempUpdateTime = 0;

void printIpAddressToStringBuf(uint32_t ipAddress, char* buf) {
  uint8_t* ipAddr = (uint8_t*) &ipAddress;
  sprintf(
    buf, 
    (const char *) "%d.%d.%d.%d",
    ipAddr[0],
    ipAddr[1],
    ipAddr[2],
    ipAddr[3]
  );  
}

void clearScreen() {
  display.clearDisplay();
}

void updateSplashScreen(const char *message) {
  clearScreen();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,LINE_1);
  display.write(message);
  display.display();
}

/**
 * dateStr should be padded to 5 chars: MM/DD
 * timeStr should be 8 chars: hh:mm PM 
 */
void updateScreen(
   const char *dateTimeStr,
   int temp,
   int humidity,
   boolean fanOn,
   boolean wifiConnected,
   int onTemp,
   int offTemp
 ) {
  clearScreen();
  char buf[17]; // 16 chars + null
  
  display.setCursor(0, LINE_1);
  display.write(dateTimeStr);

 // TODO: use bold font for temp and humidity numbers?
  display.setCursor(0, LINE_2);
  snprintf(buf, sizeof(buf), "T: %3d F  H: %2d%%", temp, humidity);
  display.write(buf);

  display.setCursor(0, LINE_3);
  snprintf(buf, sizeof(buf), "Fan: %3s  WiFi %s", fanOn ? "ON" : "OFF", wifiConnected ? "+" : "x" );
  display.write(buf);

  display.setCursor(0, LINE_4);
  snprintf(buf, sizeof(buf), "ON / OFF %d / %d", 75, 72);
  display.write(buf);

  display.display();  
 }

 void setupSerial() {
  Serial.begin(115200);
  while (! Serial);
  Serial.print("\n\n\n\n");
 }

 void setupDisplay() {
  Serial.println("Initializing display");
  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000);
  Serial.println(F("SSD1306 allocation Succeeded"));
}

void setupWiFi() {
  Serial.println("Initializing WiFi");
  updateSplashScreen("Connecting to WiFi");
  
  WiFi.begin("mch_private", "torbertcoryell");

  Serial.println("Waiting for WiFi connection");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("\nWiFi connected");   
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  char buf[150];
  char buf2[100];
  printIpAddressToStringBuf( WiFi.localIP(), buf2);
  snprintf(buf, sizeof(buf), "WiFi Connected\n\nIP Address: \n\n%s", buf2);
  updateSplashScreen(buf);  
  delay(2000);
}

void setupNTP() {
  Serial.println("Getting time from NTP server");
  updateSplashScreen("Getting NTP Time");
  timeClient.begin();
  delay ( 1000 );
  
  if (timeClient.update()) {
     unsigned long epoch = timeClient.getEpochTime();
     setTime(epoch);
     Serial.print("It is now: ");
     Serial.println(NTPTime::getCurrentDateTimeFormatted());
     Serial.println("");
     updateSplashScreen((const char*) NTPTime::getCurrentDateTimeFormatted().c_str());
     delay(1000);
  } else {
     Serial.print ( "NTP Update Failed!!" );
     updateSplashScreen("NTP Update Failed");
     for(;;);
  }  
}


void readTempAndHumidity() {
  Serial.println("updating Temp and Humidity"); 
  delay(250);

  
  /*
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
  float h = dht.readHumidity();
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
   if (isnan(h) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(NTPTime::getCurrentDateTimeFormatted());
  Serial.print("  ");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  
  Serial.print(F("%  Temperature: "));
  Serial.print(f);
  Serial.println(F("°F "));

   */  
}

void fakeReadTempAndHumidity() {
  Serial.println("updating Temp and Humidity"); 
  delay(250);
  temp = 80;
  humidity = 60;
}

void updateTempAndHumidity() {
  // We can't read the DHT22 temp/humidity sensor more frequently than every 2 seconds.
  // So, update only once per minute.
  unsigned long currentSeconds = now();
  unsigned long diff = currentSeconds - lastTempUpdateTime;
  
  if (lastTempUpdateTime == 0 || diff >= 60) {
    lastTempUpdateTime = currentSeconds;
    fakeReadTempAndHumidity();
  } else {
     delay(250); // simulate time to read the temp/humidity sensor
  }
}

void setup() {
  setupSerial();

  setupDisplay();

  setupWiFi();

  setupNTP();
}



void loop() {
  ++loopCount;

  updateTempAndHumidity();

  // TODO - check temp vs. threshold, toggle fan state if appropriate

  updateScreen(
   (const char*) NTPTime::getCurrentDateTimeFormatted().c_str(),
   temp,
   humidity,
   true, // fan on
   true, // wifi connected
   75,   // onTemp
   72    // offTemp
 );

  // TODO - push data to REST api at appropriate interval (15 min, 30 min, 60 min ?)

  /*
  // Wait 1 hour between measurements.
  // TODO - we want to stay active (update the clock on the display once per second, and the temp/humidity every 5 seconds or so),
  // but only push data to the server once per hour.
  // So keep track of the last data push time, and only push after an hour has elapsed.
  // See https://www.arduino.cc/en/Tutorial/BuiltInExamples/BlinkWithoutDelay
  */
  
  delay(700);
}
