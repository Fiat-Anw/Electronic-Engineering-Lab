#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define display size and I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// Create an SSD1306 display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

int x;
// Function to center text horizontally
int16_t getCenteredX(const char* text, int textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  
  // Get text dimensions
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  // Calculate centered X position
  return (SCREEN_WIDTH - w) / 2;
}

void setup() {

  // Start serial communication
  Serial.begin(9600);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Stop if display initialization fails
  }

  // Set text properties
  display.setTextSize(2);  // Normal 1:1 scale
  display.setTextColor(SSD1306_WHITE); // White text color
}

void loop() {

  display.clearDisplay(); // Clear the display buffer

  // Center and print first line
  x = getCenteredX("IoT 2447", 1);  // Calculate X for the first line
  display.setCursor(x, 10);             // Y = 10 for first line
  display.println(F("IoT 2447"));

  x = getCenteredX("S2G5", 1);  // Calculate X for the first line
  display.setCursor(x, 30);                  // Y = 30 for second line
  display.println(F("S2G5"));

  // Show the content on the display
  display.display();

  delay(1000);  // Hold the message for 1 second

  display.clearDisplay(); // Clear the display buffer

  // Center and print first line
  x = getCenteredX("OLED", 1);  // Calculate X for the first line
  display.setCursor(x, 10);             // Y = 10 for first line
  display.println(F("OLED"));

  x = getCenteredX("Testing", 1);  // Calculate X for the first line
  display.setCursor(x, 30);             // Y = 30 for second line
  display.println(F("Testing"));

  x = getCenteredX("S2G5", 1);  // Calculate X for the first line
  display.setCursor(x, 50);             // Y = 50 for third line
  display.println(F("S2G5"));

  // Show the content on the display
  display.display();

  delay(1000);  // Hold the message for 1 second

}
