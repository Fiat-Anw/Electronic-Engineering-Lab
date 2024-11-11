/*
  Project Name: IoT Lab2 Template
  File name: IoT-2.ino
  Last modified on: October, 2021
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // for SSD1306
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  // for BME280
#include <WiFi.h>  // for ESP32 WiFi
#include <PubSubClient.h>  // for NETPIE
#include "credentials.h"  // WiFi and NETPIE credentials
#include "iot_iconset_16x16.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1  // OLED reset pin (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS  0x3C  // OLED I2C address
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // OLED
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x77  // BME I2C address
Adafruit_BME280 bme;  // BME
WiFiClient espClient;
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
PubSubClient client(espClient);

float pressure, altitude, temperature, humidity;
char msg[100];

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize OLED
  while(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED failed to begin"));
    delay(100);
  }
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE, BLACK);
  oled.setCursor(0,0);
  oled.println(F("OLED passed"));
  oled.display();
  delay(100);

  // Initialize BME280
  while(!bme.begin(BME_ADDRESS)) {
    oled.setCursor(0,oled.getCursorY());
    oled.print(F("BME failed to begin"));
    oled.display();
    delay(100);
  }
  oled.setCursor(0,oled.getCursorY());
  oled.println(F("BME passed"));
  oled.display();
  delay(100);

  // Connect to WiFi
  connectWiFi();
  
  // Initialize MQTT
  client.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();
}

void loop() {
  // Read sensor values
  pressure = bme.readPressure() / 100.0f;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  temperature = bme.readTemperature();  // in Celsius
  humidity = bme.readHumidity();  // in %RH

  // Update OLED Display
  oled.clearDisplay();
  oled.setTextSize(1);
  
  // Display altitude
  oled.drawBitmap(0, 0, altitude_icon16x16, 16, 16, 1);
  oled.setCursor(20, 0);
  oled.println("Alt: " + String(altitude, 2) + " m");

  // Display temperature in Celsius
  oled.drawBitmap(0, 16, temperature_icon16x16, 16, 16, 1);
  oled.setCursor(20, 20);
  oled.println("Temp: " + String(temperature, 2) + " C");

  // Display humidity in %RH
  oled.drawBitmap(0, 32, humidity_icon16x16, 16, 16, 1);
  oled.setCursor(20, 40);
  oled.println("Hum: " + String(humidity, 2) + " %");

  // WiFi status icon
  if (WiFi.status() == WL_CONNECTED) {
    oled.drawBitmap(112, 48, wifi1_icon16x16, 16, 16, 1);
  } else {
    oled.fillRect(112, 48, 16, 16, 0);
    oled.setCursor(0, 56);
    oled.println("Reconnecting...");
  }
  oled.display();

  // Attempt to reconnect WiFi and MQTT if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    reconnectMQTT();
  } else {
    client.loop();

    // Prepare JSON payload
    String payload = "{\"data\": {";
    payload.concat("\"pressure\":" + String(pressure, 4));
    payload.concat(", ");
    payload.concat("\"altitude\":" + String(altitude, 2));
    payload.concat(", ");
    payload.concat("\"temperature\":" + String(temperature * 9 / 5 + 32, 2));  // Fahrenheit for NETPIE
    payload.concat(", ");
    payload.concat("\"humidity\":" + String(humidity, 2));
    payload.concat("}}");

    payload.toCharArray(msg, (payload.length() + 1));
    client.publish("@shadow/data/update", msg);
  }

  delay(3000);
}

// Function to connect to WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {  // Try for ~20 seconds
    delay(1000);
    attempts++;
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    oled.setCursor(0, oled.getCursorY());
    oled.println("WiFi connected");
    oled.display();
  } else {
    Serial.println("Failed to connect WiFi");
    oled.setCursor(0, oled.getCursorY());
    oled.println("WiFi failed");
    oled.display();
  }
}

// Function to connect/reconnect to MQTT
void reconnectMQTT() {
  while (!client.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      delay(2000);  // Wait 2 seconds before retrying
    }
  }
}
