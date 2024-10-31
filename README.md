# BMSLib

A comprehensive Arduino library for interfacing with the BQ34Z100-R2 Battery Management System. This library provides a complete interface for battery monitoring, configuration, and management across multiple Arduino platforms. Originally intended for personal use, the library has grown into a comprehensive solution anyone can use.

[![Arduino Badge](https://img.shields.io/badge/Arduino-Library-blue)]()
[![Platform Badge](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266%20%7C%20Arduino-green)]()
[![Version Badge](https://img.shields.io/badge/Version-1.2.0-blue)]()

## Documentation

- [Complete API Reference](docs/APIReference.md) - Detailed documentation of all functions and features
- [Examples](examples/) - Example sketches demonstrating library usage
- [Hardware Setup](docs/Hardware.md) - Connection and hardware setup guide

## Features

- **Basic Battery Monitoring**
  - Voltage, current, and temperature readings
  - State of Charge (SoC) and State of Health (SoH)
  - Capacity measurement and tracking
  - Cycle count monitoring

- **Advanced Power Management**
  - Charge voltage and current control
  - Power and energy monitoring
  - Time-to-empty and time-to-full predictions
  - Sleep mode support
  - Watchdog management

- **Safety Features**
  - Comprehensive alarm system
  - Over/under voltage protection
  - Over current protection
  - Temperature monitoring
  - Safety status tracking

- **Configuration and Calibration**
  - Battery chemistry configuration
  - Capacity settings
  - Threshold adjustments
  - Calibration support

## Hardware Compatibility

- Arduino Uno/Nano (ATmega328P)
- Arduino Mega (ATmega2560)
- ESP32
- ESP8266
- Any Arduino-compatible board with I2C support

## Installation

1. Download the library (ZIP)
2. In Arduino IDE: Sketch -> Include Library -> Add .ZIP Library
3. Select the downloaded ZIP file
4. Restart Arduino IDE

## Basic Usage

```cpp
#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;

void setup() {
  Serial.begin(115200);
  if (!bms.begin()) {
    Serial.println("BMS initialization failed!");
    while (1);
  }
}

void loop() {
  float voltage = bms.readVoltage_inVolts();
  float current = bms.readCurrent_inAmps();
  float temp = bms.readTemperature_inCelsius();
  uint8_t soc = bms.readSoC();
  
  Serial.printf("Voltage: %.2fV\n", voltage);
  Serial.printf("Current: %.2fA\n", current);
  Serial.printf("Temperature: %.1f°C\n", temp);
  Serial.printf("State of Charge: %d%%\n", soc);
  
  delay(1000);
}
```

## Quick Reference

For full documentation of all functions, see the [API Reference](docs/APIReference.md).

### Common Functions
| Category | Example Functions | Description |
|----------|------------------|-------------|
| Basic Operations | `begin()`, `isOnline()` | Core initialization and status |
| Battery Measurements | `readVoltage_inVolts()`, `readCurrent_inAmps()` | Basic battery parameters |
| Power Management | `sleep()`, `wake()` | Power state control |
| Alarms | `setAlarmConfig()`, `getAlarmStatus()` | Alarm system management |
| Configuration | `setConfiguration()`, `getConfiguration()` | BMS configuration |

## Error Handling

The library provides comprehensive error handling through the `BMSError` enumeration:

```cpp
enum class BMSError {
    NONE,
    COMMUNICATION_ERROR,
    INVALID_STATE,
    CALIBRATION_ERROR,
    TIMEOUT_ERROR,
    PARAMETER_ERROR,
    INITIALIZATION_ERROR,
    CONFIGURATION_ERROR
};
```

## Platform-Specific Features

### ESP32/ESP8266
- Thread-safe I2C communication
- Mutex protection for multi-core operations

### ATmega (Uno/Nano/Mega)
- Optimized for limited RAM
- Reduced stack usage

## Examples

The library includes several example sketches:
- `Basic/Basic.ino`: Simple battery monitoring
- `Advanced/Advanced.ino`: Advanced features and configuration
- `Alarms/Alarms.ino`: Alarm system configuration and monitoring

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This library is released under the [MIT License](LICENSE).

## Support

For bugs, feature requests, and questions:
- Open an issue on GitHub
- [Contact information]

## Version History

- 1.2.0: Added alarm system support
- 1.1.0: Added advanced power management
- 1.0.0: Initial release

## References

- [BQ34Z100-R2 Technical Reference](https://www.ti.com/product/BQ34Z100-R2)
- [Hardware Setup Guide](docs/Hardware.md)
