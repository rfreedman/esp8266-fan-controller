# esp8266-fan-controller
Arduino code for an ESP-8266 - based attic fan controller, with OLED display and logging to a REST API

This is the source code for a ESP-8266 project with the following features:
* Monitor temperature and humidity using a DHT-22 Sensor
* Control an external "IOT Relay" (https://dlidirect.com/products/iot-power-relay) to turn a fan on / off depending on temperature
* Display current time, temperature, humidity and fan on/off state on an SSD1306 monochrome OLED
* Log time, temperature and humidity via a back-end REST API using WiFi
* Recieve commands over WiFi to change the on / off temperature set points
* Set up a WiFi AP to allow updating the WiFi SSID and Password to use to connect to the real Wifi AP
