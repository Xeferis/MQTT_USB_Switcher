#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

String sel_pc = "waiting...";

/// WiFi Setup
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

int wifistrngth;

// MQTT Setup
const char* mqtt_server = "YOUR_MQTT_SERVER";
WiFiClient espClient;
PubSubClient client(espClient);

// NEEDS TO BE FILLED !!!!