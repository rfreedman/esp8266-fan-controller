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

#include "Screen.h"

#include "DHT.h"

#include "NTPTime.h"

// !!!! TODO - move DHT to D3 to stop conflict with OLED
#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1 to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "mch_private";
const char *password = "torbertcoryell";

long loopCount = 0;

WiFiUDP ntpUDP;

int GMTOffset = 0; // SET TO UTC TIME (No offset)
int ntpUpdateIntervalMs = 60 * 60 * 1000; // update once per hour
NTPClient timeClient(ntpUDP, "pool.ntp.org", GMTOffset, ntpUpdateIntervalMs);

int temp = -999;
int humidity = -999;

unsigned long lastTempUpdateTime = 0;

Screen screen;

void printIpAddressToStringBuf(uint32_t ipAddress, char *buf) {
    uint8_t *ipAddr = (uint8_t * ) & ipAddress;
    sprintf(
            buf,
            (const char *) "%d.%d.%d.%d",
            ipAddr[0],
            ipAddr[1],
            ipAddr[2],
            ipAddr[3]
    );
}

void setupSerial() {
    Serial.begin(115200);
    while (!Serial);
    Serial.print("\n\n\n\n");
}

void setupWiFi() {
    Serial.println("Initializing WiFi");
    screen.updateSplashScreen("Connecting to WiFi");

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
    printIpAddressToStringBuf(WiFi.localIP(), buf2);
    snprintf(buf, sizeof(buf), "WiFi Connected\n\nIP Address: \n\n%s", buf2);
    screen.updateSplashScreen(buf);
    delay(2000);
}

void setupNTP() {
    Serial.println("Getting time from NTP server");
    screen.updateSplashScreen("Getting NTP Time");
    timeClient.begin();
    delay(1000);

    if (timeClient.update()) {
        unsigned long epoch = timeClient.getEpochTime();
        setTime(epoch);
        Serial.print("It is now: ");
        Serial.println(NTPTime::getCurrentDateTimeFormatted());
        Serial.println("");
        screen.updateSplashScreen((const char *) NTPTime::getCurrentDateTimeFormatted().c_str());
        delay(1000);
    } else {
        Serial.print("NTP Update Failed!!");
        screen.updateSplashScreen("NTP Update Failed");
        for (;;);
    }
}

void readTempAndHumidity() {
    Serial.println("updating Temp and Humidity");
    delay(250);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

    float readHumidity = dht.readHumidity();

    // Read temperature as Fahrenheit (isFahrenheit = true)
    float readTemp = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(readHumidity) || isnan(readTemp)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    humidity = round(readHumidity);

    Serial.print(F("Humidity: "));
    Serial.print(humidity);

    temp = round(readTemp);

    Serial.print(F("%  Temperature: "));
    Serial.print(temp);
    Serial.println(F("°F "));
}

void fakeReadTempAndHumidity() {
    Serial.println("updating Temp and Humidity");
    delay(250);
    temp = round(80.5);
    humidity = round(60.5);
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

    screen.setupDisplay();

    setupWiFi();

    setupNTP();
}


void loop() {
    ++loopCount;

    // this should be named maybeUpdate()
    // it checks to see if the update threshold has passed,
    // and updates if it has. Won't update on it's own,
    // so kick it here.
    timeClient.update();

    updateTempAndHumidity();

    // TODO - check temp vs. threshold, toggle fan state if appropriate

    screen.updateScreen(
            (const char *) NTPTime::getCurrentDateTimeFormatted().c_str(),
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
