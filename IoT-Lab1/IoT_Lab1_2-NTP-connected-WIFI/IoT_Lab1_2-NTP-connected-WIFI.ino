#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace with your network credentials
const char* ssid = "--"; // Wifi ssid
const char* password = "--"; // Wifi password

// NTP server and time settings for Thailand (GMT+7)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600 * 7;  // GMT+7
const int daylightOffset_sec = 0;

int buttonPin = 27; // GPIO for the push button (adjust based on your setup)
bool showIPAddress = false; // Toggle state for OLED display
bool lastButtonState = HIGH; // Store the last button state

struct tm timeinfo;

void displayCenteredText(const String& text, int y) {
    int16_t x1, y1;
    uint16_t w, h;

    // Measure the text's width and height in pixels
    display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

    // Calculate x position to center the text horizontally
    int x = (display.width() - w) / 2;

    // Set the cursor to the calculated position
    display.setCursor(x, y);
    display.println(text);
}

void setup() {
    // Initialize serial monitor for debugging
    Serial.begin(115200);
    
    // Initialize button pin
    pinMode(buttonPin, INPUT_PULLUP);

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Configure NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
    // Read button state
    bool buttonState = digitalRead(buttonPin);

    // Check for button press (HIGH to LOW transition)
    if (buttonState == LOW && lastButtonState == HIGH) {
        showIPAddress = !showIPAddress; // Toggle display state
        delay(50); // Small delay for debounce
    }

    // Update the last button state
    lastButtonState = buttonState;

    display.clearDisplay();
    
    if (showIPAddress) {
        // Display IP address
        displayCenteredText("ESP32 IP is:", 0);  // Header text
        displayCenteredText(WiFi.localIP().toString(), 20);  // IP address
    } else {
        // Get current time and date
        if (getLocalTime(&timeinfo)) {
            // Display time in HH:MM:SS format
            char timeStr[9];
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
            displayCenteredText(String("Time: ") + timeStr, 0);

            // Display date in DD/MM/YY format
            char dateStr[9];
            strftime(dateStr, sizeof(dateStr), "%d/%m/%y", &timeinfo);
            displayCenteredText(String("Date: ") + dateStr, 20);

        } else {
            Serial.println("Failed to obtain time");
            displayCenteredText("Time Sync Error", 0);
        }
    }

    // Display the group name
    displayCenteredText("[ S2G5 ]", 50);

    display.display();
    delay(100); // Update every 100 ms
}
