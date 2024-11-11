/*
  Project Name: IoT Lab3 Template (Modified with TSL2561 for Light Intensity and LED Control)
  File name: IoT-3.ino
  Last modified on: November, 2024
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // for SSD1306
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  // for BME280
#include <Adafruit_TSL2561_U.h>  // for TSL2561
#include <WiFi.h>  // for ESP32 WiFi
#include <PubSubClient.h>  // for NETPIE
#include "credentials.h"  // WiFi and NETPIE credentials
#include "iot_iconset_16x16.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1  // OLED reset pin (or -1 if sharing Arduino reset pin)
#define OLED_ADDRESS  0x3C  // OLED I2C address (Adafruit = 0x3D China 0x3C)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x77  // BME I2C address (Red module = 0x77 Purple module = 0x76)
Adafruit_BME280 bme;  // I2C
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT);

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;

#define LED_BUILTIN 5
#define LIGHT_THRESHOLD 100  // Define a threshold for light intensity in lux

float temperature, humidity, pressure, altitude;
uint16_t lightIntensity;
char msg[300];
bool ledStatus = false;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if(String(topic) == "@msg/led") {
    ledStatus = (message == "on");
    digitalWrite(LED_BUILTIN, !ledStatus);  // Update LED state
    client.publish("@shadow/data/update", ledStatus ? "{\"data\" : {\"led\" : \"on\"}}" : "{\"data\" : {\"led\" : \"off\"}}");
    
    // Update OLED display for LED status
    oled.fillRect(16, 48, 96, 16, 0);
    oled.setCursor(16, 48);
    oled.println(ledStatus ? "on" : "off");
    oled.display();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1); // Initialize LED off
  
  // Initialize OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED initialization failed"));
    while (true);
  }
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE, BLACK);
  oled.setCursor(0, 0);
  oled.println(F("OLED initialized"));
  oled.display();

  // Initialize BME280 sensor
  if (!bme.begin(BME_ADDRESS)) {
    oled.println(F("BME280 initialization failed"));
    oled.display();
    while (true);
  }
  oled.println(F("BME280 initialized"));
  oled.display();

  // Initialize TSL2561 sensor
  if (!tsl.begin()) {
    oled.println(F("TSL2561 initialization failed"));
    oled.display();
    while (true);
  }
  tsl.enableAutoRange(true);            // Auto-gain for lighting conditions
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);  // Short integration time for faster readings
  oled.println(F("TSL2561 initialized"));
  oled.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connect to NETPIE
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    client.connect(mqtt_client, mqtt_username, mqtt_password);
    delay(500);
  }
  client.subscribe("@msg/led");
}

void loop() {
  if (!client.connected()) {
    // Reconnect to MQTT
    while (!client.connected()) {
      client.connect(mqtt_client, mqtt_username, mqtt_password);
      delay(500);
    }
    client.subscribe("@msg/led");
  }

  // Read sensor data
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;  // in hPa
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);  // in meters

  // Read light intensity from TSL2561
  sensors_event_t event;
  tsl.getEvent(&event);
  if (event.light) {
    lightIntensity = event.light;  // Light intensity in lux
  } else {
    lightIntensity = 0;  // Sensor error
  }

  // Control LED based on light intensity
  if (lightIntensity < LIGHT_THRESHOLD) {
    ledStatus = true;
    digitalWrite(LED_BUILTIN, LOW);  // Turn LED on
  } else {
    ledStatus = false;
    digitalWrite(LED_BUILTIN, HIGH);  // Turn LED off
  }

  // Display sensor data with icons on OLED
  oled.clearDisplay();
  oled.setTextSize(1);

  // Temperature
  oled.drawBitmap(0, 0, temperature_icon16x16, 16, 16, 1);  // Icon for temperature
  oled.setCursor(20, 0);
  oled.print("Temp: ");
  oled.print(temperature, 1);
  oled.print(" C");

  // Humidity
  oled.drawBitmap(0, 16, humidity_icon16x16, 16, 16, 1);  // Icon for humidity
  oled.setCursor(20, 16);
  oled.print("Humidity: ");
  oled.print(humidity, 1);
  oled.print(" %");

  oled.setCursor(20, 32);
  oled.print("Pressure: ");
  oled.print(pressure, 1);
  oled.print(" hPa");
  
  oled.setCursor(20, 48);
  oled.print("Light: ");
  oled.print(lightIntensity);
  oled.print(" lux");

  // LED Status
  if (ledStatus) {
    oled.drawBitmap(100, 48, bulb_on_icon16x16, 16, 16, 1);  // Icon for LED on
  } else {
    oled.drawBitmap(100, 48, bulb_off_icon16x16, 16, 16, 1);  // Icon for LED off
  }

  if(WiFi.status() == WL_CONNECTED) {
    oled.drawBitmap(112, 48, wifi1_icon16x16, 16, 16, 1);
  }

  oled.display();

  // Send data to NETPIE
  String payload = "{\"data\": {";
  payload += "\"temperature\":" + String(temperature, 1);
  payload += ", \"humidity\":" + String(humidity, 1);
  payload += ", \"pressure\":" + String(pressure, 1);
  payload += ", \"altitude\":" + String(altitude, 1);
  payload += ", \"light_intensity\":" + String(lightIntensity);
  payload += ", \"led_status\":\"" + String(ledStatus ? "on" : "off") + "\"";
  payload += "}}";
  
  payload.toCharArray(msg, payload.length() + 1);
  client.publish("@shadow/data/update", msg);

  client.loop();
  delay(3000);  // Delay for 3 seconds before next reading
}
