# ESP32 RFID Access Control System

A simple RFID-based access control system using an ESP32, RFID-RC522 reader, and OLED display.

## Features

- Read RFID cards/tags using RFID-RC522 module
- Display access status on 0.96" OLED screen
- Visual feedback with "ACCESS GRANTED" or "LOCKED" messages
- Easy to add/remove authorized card UIDs
- Serial monitor output for debugging

## Hardware Requirements

- ESP32 Development Board
- RFID-RC522 Reader Module
- JMD0.96D-1 OLED Display (128x64, I2C, 0.96")
- Jumper wires
- Breadboard (optional)

## Wiring Diagram

### RFID-RC522 Connections
| RC522 Pin | ESP32 Pin |
|-----------|-----------|
| SDA       | GPIO 5    |
| SCK       | GPIO 18   |
| MOSI      | GPIO 23   |
| MISO      | GPIO 19   |
| IRQ       | NC        |
| GND       | GND       |
| RST       | GPIO 22   |
| 3.3V      | 3.3V      |

### OLED Display Connections (I2C)
| OLED Pin | ESP32 Pin |
|----------|-----------|
| SDA      | GPIO 21   |
| SCL      | GPIO 22   |
| VCC      | 3.3V      |
| GND      | GND       |

## Required Libraries

Install these libraries through the Arduino IDE Library Manager:

1. **MFRC522** by GithubCommunity
2. **Adafruit GFX Library** by Adafruit
3. **Adafruit SSD1306** by Adafruit

## Installation

1. Clone this repository
2. Open `esp32-rfid-reader.ino` in Arduino IDE
3. Install the required libraries
4. Connect your ESP32 to your computer
5. Select your ESP32 board and port in Tools menu
6. Upload the sketch

## Configuration

### Adding Authorized Cards

1. Upload the sketch to your ESP32
2. Open the Serial Monitor (115200 baud)
3. Scan your RFID card/tag
4. Note the UID displayed in the serial monitor (e.g., "AA BB CC DD")
5. Add the UID to the `authorizedUIDs` array in the code:

```cpp
String authorizedUIDs[] = {
  "AA BB CC DD",  // Your card UID
  "11 22 33 44",  // Another card UID
};
```

6. Re-upload the sketch

## Usage

1. Power on the ESP32
2. The OLED display will show "SCAN CARD"
3. Place an RFID card/tag near the reader
4. The display will show:
   - **ACCESS GRANTED** with a checkmark for authorized cards
   - **LOCKED** with an X symbol for unauthorized cards
5. After 3 seconds, the display returns to "SCAN CARD"

## Troubleshooting

### OLED Display Not Working
- Check I2C address (default is 0x3C)
- Verify wiring connections
- Try running an I2C scanner sketch to detect the display

### RFID Reader Not Detecting Cards
- Check SPI connections
- Ensure RST pin is properly connected
- Verify 3.3V power supply
- Try moving the card closer to the reader

### Cards Not Being Recognized
- Ensure the UID in the code matches exactly (including spaces)
- UIDs are case-sensitive
- Check serial monitor for the actual UID being read

## Customization

You can customize the display messages and graphics by modifying these functions:
- `displayAccessGranted()` - Access granted screen
- `displayAccessDenied()` - Access denied screen
- `displayIdleScreen()` - Idle waiting screen
- `displayMessage()` - Custom message display

## License

This project is open source and available under the MIT License.

## Contributing

Feel free to submit issues and pull requests for improvements!
