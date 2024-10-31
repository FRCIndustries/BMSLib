# BMSLib

A comprehensive Arduino library for interfacing with the BQ34Z100-R2 Battery Management System. This library provides complete battery monitoring, configuration, and management capabilities across multiple Arduino platforms.

[![Arduino Badge](https://img.shields.io/badge/Arduino-Library-blue)]()
[![Platform Badge](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266%20%7C%20Arduino-green)]()
[![Version Badge](https://img.shields.io/badge/Version-1.2.0-blue)]()

## Documentation

- [Complete API Reference](docs/APIReference.md) - Detailed documentation of all functions and features
- [Hardware Setup Guide](docs/Hardware.md) - Connection and hardware setup guide
- [Calibration Guide](docs/Calibration.md) - Detailed calibration procedures

## Features

- **Comprehensive Battery Monitoring**
  - Voltage, current, and temperature measurements
  - State of Charge (SoC) and State of Health (SoH)
  - Capacity tracking and cycle counting
  - Temperature-compensated measurements
  - Available energy and power monitoring
  - Time-to-empty and time-to-full predictions

- **Advanced Power Management**
  - Sleep/wake control
  - Watchdog management
  - Charge voltage and current control
  - Factory reset capability

- **Complete Safety System**
  - Over/under voltage protection
  - Over current protection
  - Temperature monitoring
  - Configurable alarm thresholds
  - Real-time safety status monitoring

- **Configuration Management**
  - Battery chemistry configuration
  - Capacity settings
  - Protection thresholds
  - Alarm configuration
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
        Serial.printf("Error: %d\n", static_cast<int>(bms.getLastError()));
        while (1);
    }
}

void loop() {
    BatteryStatus status;
    if (bms.getBatteryStatus(status)) {
        Serial.printf("Voltage: %.2fV\n", status.voltage);
        Serial.printf("Current: %.2fA\n", status.current);
        Serial.printf("Temperature: %.1f°C\n", status.temperature);
        Serial.printf("State of Charge: %d%%\n", status.soc);
        
        if (status.hasError) {
            Serial.println("Warning: Battery error detected!");
        }
    }
    
    delay(1000);
}
```

## Examples

The library includes several example sketches:
- `Basic/Basic.ino`: Simple battery monitoring with error handling
- `Advanced/Advanced.ino`: Advanced features including power management and configuration
- `Alarms/Alarms.ino`: Complete alarm system demonstration

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

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This library is released under the [MIT License](LICENSE).

## Support

For bugs, feature requests, and questions:
- Open an issue on GitHub
- [Contact information]

## Version History

- 1.2.0: Added comprehensive alarm system and power monitoring
- 1.1.0: Added temperature compensation and advanced error handling
- 1.0.0: Initial release

## References

- [BQ34Z100-R2 Technical Reference](https://www.ti.com/product/BQ34Z100-R2)
- [Hardware Setup Guide](docs/Hardware.md)
- [Calibration Guide](docs/Calibration.md)
