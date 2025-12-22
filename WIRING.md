# Wiring Guide

## Complete Wiring Diagram

### ESP32 Pin Layout Reference

```
                     ESP32
                 ╔═══════════╗
            3V3  ║  1    30  ║  GND
            EN   ║  2    29  ║  GPIO 23 (MOSI)
         GPIO 36 ║  3    28  ║  GPIO 22 (RST + SCL)
         GPIO 39 ║  4    27  ║  GPIO 1 (TX)
         GPIO 34 ║  5    26  ║  GPIO 3 (RX)
         GPIO 35 ║  6    25  ║  GPIO 21 (SDA)
         GPIO 32 ║  7    24  ║  GND
         GPIO 33 ║  8    23  ║  GPIO 19 (MISO)
         GPIO 25 ║  9    22  ║  GPIO 18 (SCK)
         GPIO 26 ║  10   21  ║  GPIO 5 (SS)
         GPIO 27 ║  11   20  ║  GPIO 17
         GPIO 14 ║  12   19  ║  GPIO 16
         GPIO 12 ║  13   18  ║  GPIO 4
            GND  ║  14   17  ║  GPIO 0
         GPIO 13 ║  15   16  ║  GPIO 2
                 ╚═══════════╝
```

## Component Connections

### RFID-RC522 Module

```
RC522 Module                ESP32
┌─────────────┐
│  [ ]  SDA   │────────────── GPIO 5
│  [ ]  SCK   │────────────── GPIO 18
│  [ ]  MOSI  │────────────── GPIO 23
│  [ ]  MISO  │────────────── GPIO 19
│  [ ]  IRQ   │  (Not Connected)
│  [ ]  GND   │────────────── GND
│  [ ]  RST   │────────────── GPIO 22
│  [ ]  3.3V  │────────────── 3.3V
└─────────────┘
```

### OLED Display (I2C)

```
OLED Display               ESP32
┌─────────────┐
│    0.96"    │
│   128x64    │
│             │
│  [ ]  VCC   │────────────── 3.3V
│  [ ]  GND   │────────────── GND
│  [ ]  SCL   │────────────── GPIO 22
│  [ ]  SDA   │────────────── GPIO 21
└─────────────┘
```

## Breadboard Layout

```
        3.3V Rail ──────┬──────┬────────
                        │      │
                    ┌───┴──┐ ┌─┴────┐
                    │RC522 │ │ OLED │
                    │Module│ │Display│
                    └──┬───┘ └─┬────┘
                       │       │
         GND Rail ─────┴───────┴────────
```

## Important Notes

### Power Supply
- **Both modules run on 3.3V**
- Do NOT connect to 5V as it may damage the ESP32
- Ensure your power supply can provide enough current (~250mA minimum)

### Shared Pins
- GPIO 22 is shared between RFID RST and OLED SCL (this is intentional and works fine)
- GPIO 21 is used only for OLED SDA (I2C)

### Cable Length
- Keep wires short (< 20cm) for reliable operation
- Use quality jumper wires
- Consider using a breadboard for stable connections

### I2C Address
- Default OLED I2C address: 0x3C
- If your display doesn't work, verify the address using an I2C scanner

## Testing Individual Components

### Test OLED Display First
Upload a simple Adafruit SSD1306 example sketch to verify the display works.

### Test RFID Reader Second
Use the MFRC522 library example "DumpInfo" to verify the reader works and to get your card UIDs.

### Combine Both
Once both work individually, upload the main sketch.
