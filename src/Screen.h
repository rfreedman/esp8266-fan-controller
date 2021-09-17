//
// Created by Rich on 9/16/21.
//
// Screen utilities for the SSD1306 OLED
//
//

#ifndef FAN_CONTROLLER_SCREEN_H
#define FAN_CONTROLLER_SCREEN_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI  13
#define OLED_CLK   14
#define OLED_DC     2
#define OLED_CS    15
#define OLED_RESET  4

const int CHARS_PER_LINE = 16;
const int LINE_HEIGHT_PX = 16;

const int DISPLAY_START_X = 5;
const int LINE_1 = 0;
const int LINE_2 = LINE_1 + LINE_HEIGHT_PX;
const int LINE_3 = LINE_2 + LINE_HEIGHT_PX;
const int LINE_4 = LINE_3 + LINE_HEIGHT_PX;


#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static const unsigned char PROGMEM logo_bmp[] = {
0b00000000, 0b11000000,
0b00000001, 0b11000000,
0b00000001, 0b11000000,
0b00000011, 0b11100000,
0b11110011, 0b11100000,
0b11111110, 0b11111000,
0b01111110, 0b11111111,
0b00110011, 0b10011111,
0b00011111, 0b11111100,
0b00001101, 0b01110000,
0b00011011, 0b10100000,
0b00111111, 0b11100000,
0b00111111, 0b11110000,
0b01111100, 0b11110000,
0b01110000, 0b01110000,
0b00000000, 0b00110000
};

class Screen {

    private:

    public:
        void setupDisplay();
        void drawbitmap();
        void clearScreen();
        void updateSplashScreen(const char *message);
        void updateScreen(
                const char *dateTimeStr,
                int temp,
                int humidity,
                boolean fanOn,
                boolean wifiConnected,
                int onTemp,
                int offTemp
        );
};


#endif //FAN_CONTROLLER_SCREEN_H
