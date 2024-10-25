# BMSLib

A comprehensive Arduino library for interfacing with the BQ34Z100 Battery Management System. This library provides a complete interface for battery monitoring, configuration, and management across multiple Arduino platforms.

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

- Supports all Arduino boards with I2C capability
- Tested on the ESP32 platform only at this time.

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

## API Reference

### Core Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `begin()` | Initialize BMS communication | bool |
| `isOnline()` | Check if BMS is responding | bool |
| `getLastError()` | Get last error status | BMSError |

### Battery Measurements

| Function | Description | Return Type |
|----------|-------------|-------------|
| `readVoltage_inVolts()` | Get battery voltage | float |
| `readCurrent_inAmps()` | Get current flow | float |
| `readTemperature_inCelsius()` | Get battery temperature | float |
| `readSoC()` | Get State of Charge | uint16_t |
| `readSoH()` | Get State of Health | uint16_t |

### Power Management

| Function | Description | Return Type |
|----------|-------------|-------------|
| `sleep()` | Enter sleep mode | bool |
| `wake()` | Exit sleep mode | bool |
| `resetWatchdog()` | Reset watchdog timer | bool |

### Alarm Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `setAlarmConfig()` | Configure alarm settings | bool |
| `getAlarmStatus()` | Get current alarm status | uint16_t |
| `clearAlarms()` | Clear active alarms | bool |

### Configuration Functions

| Function | Description | Return Type |
|----------|-------------|-------------|
| `setConfiguration()` | Set BMS configuration | bool |
| `getConfiguration()` | Get current configuration | bool |
| `enterConfigMode()` | Enter configuration mode | bool |
| `exitConfigMode()` | Exit configuration mode | bool |

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

## Advanced Configuration

### Battery Chemistry Configuration

```cpp
BMSConfig config;
config.chemistry = BatteryChemistry::LIION;
config.designCapacity = 2000;  // mAh
bms.setConfiguration(config);
```

### Alarm Configuration

```cpp
BMSAlarmConfig alarmConfig;
alarmConfig.enableOverVoltage = true;
alarmConfig.voltHighThreshold = 4200;  // mV
bms.setAlarmConfig(alarmConfig);
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

## Author

Chris Formeister

## Acknowledgments

- Texas Instruments BQ34Z100-R2 Technical Reference
- Arduino Community

## Support

For bugs, feature requests, and questions:
- Open an issue on GitHub
- [Contact information]

## Version History

- 1.2.0: Added alarm system support
- 1.1.0: Added advanced power management
- 1.0.0: Initial release
