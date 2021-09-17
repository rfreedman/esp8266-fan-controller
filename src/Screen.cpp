//
// Created by Rich on 9/16/21.
//

#include "Screen.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void Screen::setupDisplay() {
    // Serial.println("Initializing display");
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        // Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    // Serial.println(F("SSD1306 allocation Succeeded"));
    delay(1000);
    drawbitmap();
    delay(2000);
}

void Screen::drawbitmap(void) {
    display.clearDisplay();

    display.drawBitmap(
            (display.width()  - LOGO_WIDTH ) / 2,
            (display.height() - LOGO_HEIGHT) / 2,
            logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.display();
}

void Screen::clearScreen() {
    display.clearDisplay();
}

void Screen::updateSplashScreen(const char *message) {
    clearScreen();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, LINE_1);
    display.write(message);
    display.display();
}

/**
 * dateStr should be padded to 5 chars: MM/DD
 * timeStr should be 8 chars: hh:mm PM
 */
void Screen::updateScreen(
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

    // line 1 - date and time
    display.setCursor(DISPLAY_START_X, LINE_1);
    display.write(dateTimeStr);

    // line 2 - temp and humidity
    display.setCursor(DISPLAY_START_X, LINE_2);
    snprintf(buf, sizeof(buf), "T: %3d F  H: %2d%%", temp, humidity);
    display.write(buf);

    // line 3 - Fan and WiFi status
    display.setCursor(DISPLAY_START_X, LINE_3);
    snprintf(buf, sizeof(buf), "Fan: %3s  WiFi %s", fanOn ? "ON" : "OFF", wifiConnected ? "+" : "x");
    display.write(buf);

    // Line 4 - on/off set points
    display.setCursor(DISPLAY_START_X, LINE_4);
    snprintf(buf, sizeof(buf), "ON / OFF %d / %d", 75, 72);
    display.write(buf);

    display.display();
}