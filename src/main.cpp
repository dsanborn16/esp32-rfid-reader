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
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "LilDon2.4";
const char* password = "LilyandDon219";

// MQTT settings
const char* mqtt_server = "10.0.0.155";
const int mqtt_port = 1883;
const char* mqtt_username = "esp322";
const char* mqtt_password = "rfidscanner";
const char* mqtt_topic = "homeassistant/sensor/rfid_reader/state";
const char* mqtt_discovery_topic = "homeassistant/sensor/rfid_reader/config";

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

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Authorized RFID UIDs (add your card/tag UIDs here)
String authorizedUIDs[] = {
  "E3 3C 5C 1C",  // Donovan's card
  "F3 FB 53 94",  // Lily's card
};

// Names corresponding to each UID
String authorizedNames[] = {
  "Donovan",
  "Lily",
};

const int numAuthorizedUIDs = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

// Forward declarations
String getUID();
int getAuthorizedIndex(String uid);
void displayIdleScreen();
void displayAccessGranted(String name);
void displayAccessDenied();
void displayMessage(String title, String message, bool center);
void connectWiFi();
void reconnectMQTT();
void publishDiscoveryConfig();
void publishRFIDScan(String name, String uid, bool authorized);

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
  displayMessage("Connecting", "WiFi...", true);
  
  // Connect to WiFi
  connectWiFi();
  
  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  
  displayMessage("RFID Reader", "Ready", true);
  
  Serial.println("RFID Reader Ready");
  Serial.println("Scan your card...");
  
  delay(2000);
  displayIdleScreen();
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  
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
  int userIndex = getAuthorizedIndex(uidString);
  if (userIndex >= 0) {
    String userName = authorizedNames[userIndex];
    Serial.print("Access Granted! Welcome, ");
    Serial.println(userName);
    publishRFIDScan(userName, uidString, true);
    displayAccessGranted(userName);
  } else {
    Serial.println("Access Denied!");
    publishRFIDScan("Unknown", uidString, false);
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

// Get authorized user index, returns -1 if not found
int getAuthorizedIndex(String uid) {
  for (int i = 0; i < numAuthorizedUIDs; i++) {
    if (uid == authorizedUIDs[i]) {
      return i;
    }
  }
  return -1;
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

// Display access granted message with name
void displayAccessGranted(String name) {
  display.clearDisplay();
  
  // Draw checkmark
  display.fillCircle(64, 20, 15, SSD1306_WHITE);
  display.fillTriangle(56, 20, 60, 26, 72, 14, SSD1306_BLACK);
  display.fillTriangle(60, 26, 64, 22, 72, 14, SSD1306_BLACK);
  
  display.setTextSize(2);
  
  // Center the name
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 45);
  display.println(name);
  
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

// Connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

// Reconnect to MQTT broker
void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }
  
  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker at ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print("...");
    
    String clientId = "ESP32-RFID-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected!");
      // Publish Home Assistant discovery config
      publishDiscoveryConfig();
      Serial.println("Published discovery config");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" (trying again in 5s)");
    }
  }
}

// Publish Home Assistant MQTT Discovery configuration
void publishDiscoveryConfig() {
  String config = "{";
  config += "\"name\":\"RFID Reader\",";
  config += "\"state_topic\":\"homeassistant/sensor/rfid_reader/state\",";
  config += "\"value_template\":\"{{ value_json.name }}\",";
  config += "\"json_attributes_topic\":\"homeassistant/sensor/rfid_reader/state\",";
  config += "\"unique_id\":\"esp32_rfid_reader\",";
  config += "\"device\":{";
  config += "\"identifiers\":[\"esp32_rfid\"],";
  config += "\"name\":\"ESP32 RFID Reader\",";
  config += "\"manufacturer\":\"ESP32\",";
  config += "\"model\":\"RFID-RC522\"";
  config += "}";
  config += "}";
  
  mqttClient.publish(mqtt_discovery_topic, config.c_str(), true);
}

// Publish RFID scan event to MQTT
void publishRFIDScan(String name, String uid, bool authorized) {
  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected, reconnecting...");
    reconnectMQTT();
    return;
  }
  
  // Create JSON payload
  String payload = "{";
  payload += "\"name\":\"" + name + "\",";
  payload += "\"uid\":\"" + uid + "\",";
  payload += "\"authorized\":" + String(authorized ? "true" : "false") + ",";
  payload += "\"timestamp\":" + String(millis());
  payload += "}";
  
  Serial.print("Publishing to ");
  Serial.print(mqtt_topic);
  Serial.print(": ");
  Serial.println(payload);
  
  // Publish to MQTT
  if (mqttClient.publish(mqtt_topic, payload.c_str())) {
    Serial.println("MQTT: Published successfully");
  } else {
    Serial.println("MQTT: Publish failed");
  }
}
