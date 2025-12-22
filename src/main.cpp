/*
 * ESP32 RFID Access Control System
 * 
 * Hardware:
 * - ESP32 Development Board
 * - RFID-RC522 Reader
 * - JMD0.96D-1 OLED Display (128x64, I2C)
 * 
 * RFID-RC522 Connections:
 * SDA  -> GPIO 5
 * SCK  -> GPIO 18
 * MOSI -> GPIO 23
 * MISO -> GPIO 19
 * IRQ  -> Not Connected
 * GND  -> GND
 * RST  -> GPIO 4
 * 3.3V -> 3.3V
 * 
 * OLED Display Connections (I2C):
 * SDA -> GPIO 21
 * SCL -> GPIO 22
 * VCC -> 3.3V
 * GND -> GND
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// RFID pins
#define SS_PIN 5
#define RST_PIN 4

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Initialize RFID reader
MFRC522 rfid(SS_PIN, RST_PIN);

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Authorized RFID UIDs (add your card/tag UIDs here)
String authorizedUIDs[] = {
  "AA BB CC DD",  // Example UID - replace with your actual card UID
  "11 22 33 44",  // Add more authorized UIDs as needed
};

const int numAuthorizedUIDs = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

void setup() {
  Serial.begin(115200);
  
  // Initialize SPI bus
  SPI.begin();
  
  // Initialize RFID reader
  rfid.PCD_Init();
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Display startup message
  displayMessage("RFID Reader", "Ready", true);
  
  Serial.println("RFID Reader Ready");
  Serial.println("Scan your card...");
  
  delay(2000);
  displayIdleScreen();
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Select one of the cards
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  // Get the UID
  String uidString = getUID();
  
  Serial.print("Card UID: ");
  Serial.println(uidString);
  
  // Check if the card is authorized
  if (isAuthorized(uidString)) {
    Serial.println("Access Granted!");
    displayAccessGranted();
  } else {
    Serial.println("Access Denied!");
    displayAccessDenied();
  }
  
  // Halt PICC
  rfid.PICC_HaltA();
  
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  
  delay(3000);
  displayIdleScreen();
}

// Get UID as a string
String getUID() {
  String content = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  content = content.substring(1); // Remove leading space
  return content;
}

// Check if UID is authorized
bool isAuthorized(String uid) {
  for (int i = 0; i < numAuthorizedUIDs; i++) {
    if (uid == authorizedUIDs[i]) {
      return true;
    }
  }
  return false;
}

// Display idle screen
void displayIdleScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("SCAN");
  display.setCursor(10, 35);
  display.println("CARD");
  display.display();
}

// Display access granted message
void displayAccessGranted() {
  display.clearDisplay();
  
  // Draw checkmark
  display.fillCircle(64, 20, 15, SSD1306_WHITE);
  display.fillTriangle(56, 20, 60, 26, 72, 14, SSD1306_BLACK);
  display.fillTriangle(60, 26, 64, 22, 72, 14, SSD1306_BLACK);
  
  display.setTextSize(2);
  display.setCursor(10, 45);
  display.println("ACCESS");
  display.setCursor(8, 45);
  display.println("GRANTED");
  
  display.display();
}

// Display access denied message
void displayAccessDenied() {
  display.clearDisplay();
  
  // Draw X symbol
  display.fillCircle(64, 20, 15, SSD1306_WHITE);
  display.drawLine(56, 12, 72, 28, SSD1306_BLACK);
  display.drawLine(57, 12, 73, 28, SSD1306_BLACK);
  display.drawLine(72, 12, 56, 28, SSD1306_BLACK);
  display.drawLine(73, 12, 57, 28, SSD1306_BLACK);
  
  display.setTextSize(2);
  display.setCursor(20, 45);
  display.println("LOCKED");
  
  display.display();
}

// Display a custom message
void displayMessage(String title, String message, bool center) {
  display.clearDisplay();
  display.setTextSize(2);
  
  if (center) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 10);
  } else {
    display.setCursor(0, 10);
  }
  
  display.println(title);
  
  display.setTextSize(1);
  
  if (center) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 40);
  } else {
    display.setCursor(0, 40);
  }
  
  display.println(message);
  display.display();
}
