# Grimoire

A magic gadget project that brings interactive lighting and gesture sensing to an ESP32 microcontroller. This project combines RGB NeoPixel LEDs with an APDS-9960 gesture sensor to create a responsive, gesture-controlled lighting device.

## Overview

Grimoire is an Arduino-based ESP32 project that demonstrates advanced sensor integration and LED control. It uses hand gestures detected by the APDS-9960 sensor to control NeoPixel RGB LEDs, enabling intuitive, gesture-based interactions.

## Hardware Requirements

- **Microcontroller**: ESP32 (LOLIN D32)
- **RGB LED Strip**: Adafruit NeoPixel (WS2812B compatible)
- **Gesture Sensor**: APDS-9960 RGB and Gesture Sensor
- **Audio Module**: Open Smart ReDMP3 (for sound playback)
- **Power Supply**: Appropriate power source for ESP32 and LEDs
- **Connections**: I2C for sensors, GPIO for LEDs

### Pin Connections

| ESP32 Pin | Component | Function |
|-----------|-----------|----------|
| 3.3V | VCC | Power (APDS-9960) |
| GND | GND | Ground |
| GPIO 21 (SDA) | SDA | I2C Data |
| GPIO 22 (SCL) | SCL | I2C Clock |
| GPIO INT | INT | Interrupt (APDS-9960) |
| GPIO DATA | DIN | NeoPixel Data Line |

## Features

- **Gesture Recognition**: Detect hand gestures (UP, DOWN, LEFT, RIGHT, NEAR, FAR)
- **RGB LED Control**: Dynamic color control via NeoPixel LEDs
- **Audio Integration**: Sound playback support with ReDMP3 module
- **Interactive**: Real-time response to user gestures
- **Efficient**: Optimized for ESP32 performance

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) or Arduino IDE
- USB cable for ESP32 programming

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/jongwonbaek2000/grimoire.git
   cd grimoire
   ```

2. **Install dependencies** (if using PlatformIO):
   ```bash
   platformio lib install
   ```

3. **Connect your hardware**:
   - Connect APDS-9960 sensor via I2C (GPIO 21/22)
   - Connect NeoPixel strip to designated GPIO pin
   - Connect ReDMP3 module via appropriate serial/I2C interface
   - Connect power supply

4. **Build and upload**:
   ```bash
   platformio run --target upload
   ```
   
   Or via Arduino IDE:
   - Open the sketch in Arduino IDE
   - Select Board: LOLIN D32
   - Select the appropriate COM port
   - Click Upload

5. **Monitor serial output**:
   ```bash
   platformio device monitor --baud 115200
   ```

## Project Structure

```
grimoire/
├── src/                          # Main source code
├── include/                       # Header files
├── lib/                           # External libraries
│   └── SparkFun_APDS9960_...    # APDS-9960 sensor library
├── test/                          # Unit tests
├── platformio.ini                 # PlatformIO configuration
└── README.md                      # This file
```

## Configuration

Edit `platformio.ini` to customize:
- **Board**: Current board is LOLIN D32
- **Platform**: ESP32 (espressif32)
- **Baud Rate**: 115200 (for serial monitoring)
- **Libraries**: Adafruit NeoPixel and others

## Dependencies

- **Adafruit NeoPixel** (v1.15.4+): For controlling RGB LED strips
- **SparkFun APDS9960 Library**: For gesture and color sensing
- **Arduino Framework**: ESP32 Arduino core

## Usage

Once uploaded and connected:

1. Power on the ESP32
2. Wave your hand over the APDS-9960 sensor
3. Observe the NeoPixel LEDs respond to your gestures
4. The serial monitor (115200 baud) will display gesture detection data

## Gesture Commands

The sensor recognizes the following gestures:
- **UP**: Swipe hand upward
- **DOWN**: Swipe hand downward
- **LEFT**: Swipe hand to the left
- **RIGHT**: Swipe hand to the right
- **NEAR**: Move hand closer to sensor
- **FAR**: Move hand away from sensor

## Resources

- [APDS-9960 Hookup Guide](https://learn.sparkfun.com/tutorials/apds-9960-rgb-and-gesture-sensor-hookup-guide)
- [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [PlatformIO Documentation](https://docs.platformio.org/)

## Troubleshooting

### Sensor not responding
- Verify I2C connections (GPIO 21/22)
- Check sensor power (3.3V)
- Ensure correct baud rate (115200)

### NeoPixels not lighting
- Confirm GPIO pin configuration matches hardware
- Check power supply is sufficient
- Verify data line connection

### Gesture recognition issues
- Ensure sensor is properly calibrated
- Check ambient light conditions (APDS-9960 works best in moderate light)
- Verify interrupt pin connection

## Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest improvements
- Submit pull requests

## License

This project is open source. Please maintain attribution and release any modifications under the same license.

## Author

**jongwonbaek2000**

---

**Status**: In Development  
**Last Updated**: May 2026

For more information or issues, please visit the [GitHub repository](https://github.com/jongwonbaek2000/grimoire).
