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

long lastMsg = 0;

// Display Setup
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//////////////////////

void setup(){
  // Don't use Pin 22! Output OLED Display - I2C SCL
  // Don't use Pin 21! Output OLED Display - I2C SDA
  // Input PINS not Working currently
  pinMode(2, OUTPUT); // Output PIN for displaying the selected Output
  pinMode(4, OUTPUT); // Output PIN wich controls the USB Switch
  pinMode(26, INPUT); // Input PIN for Displaying PC
  pinMode(27, INPUT); // Input PIN for Displaying Mac

  // enable serial data print
  Serial.begin(115200);

  // initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Display Connecting
  display_settings();
  /// Debugging
  // display.print("27: ");
  // display.println(digitalRead(27));
  // display.print("26: ");
  // display.println(digitalRead(26));
  display.setCursor(0, 16);
  display.print("Connecting to WiFi ..");
  display.display();

  // start WIFI Connection
  wifi_setup();

  // MQTT Setup
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // If On Start Mac is Selected Switch to PC
  if (digitalRead(27) == HIGH){
    digitalWrite(2, LOW);
    digitalWrite(4, HIGH);
    delay(100);
    digitalWrite(4, LOW);
    Serial.println("Led Off");
    sel_pc = "PC 1";
  }
}

void wifi_setup(){
  WiFi.setHostname("ESP32_USBSwitcher");
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(1000);
    }
  Serial.println(WiFi.localIP());
}

void display_settings(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/usbOutput") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //Impulse to Switch
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH);
      delay(100);
      digitalWrite(4, LOW);
      Serial.println("Led On");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      // Impulse to Switch
      digitalWrite(2, LOW);
      digitalWrite(4, HIGH);
      delay(100);
      digitalWrite(4, LOW);
      Serial.println("Led Off");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("WiFiClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/usbOutput");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(){
  wifistrngth = 100 + WiFi.RSSI();
  if (wifistrngth < 0)
  {
  wifistrngth = 0;
  }
  if (!client.connected()) {
  reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.print("SSID: ");
    client.print(WiFi.SSID());
    client.print("Signal: ");
    client.print(wifistrngth);
    client.print("IP: ");
    client.print(WiFi.localIP());
    client.print("MAC: ");
    client.print(WiFi.macAddress());


    if (digitalRead(27) == HIGH){
      sel_pc = "PC 2";
    }
    else if (digitalRead(26) == HIGH)
    {
      sel_pc = "PC 1";
    }
    else{
      sel_pc = "waiting...";
    }

    // Send States to MQTT
    client.publish("esp32/selectedOutput", sel_pc);

    // Display Text
    display_settings();
    display.print("SSID: ");
    display.println(ssid);
    display.print("Strength: ");
    display.print(wifistrngth);
    display.println("%");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.print("Status: ");
    display.println(sel_pc);
    display.display();
  }
}
