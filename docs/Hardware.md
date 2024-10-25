# BMSLib Hardware Setup Guide

## Table of Contents
- [Hardware Requirements](#hardware-requirements)
- [Pin Connections](#pin-connections)
- [Wiring Diagrams](#wiring-diagrams)
- [Battery Configuration](#battery-configuration)
- [Platform-Specific Setup](#platform-specific-setup)
- [Safety Considerations](#safety-considerations)
- [Troubleshooting](#troubleshooting)

## Hardware Requirements

### Required Components
- Arduino-compatible board (Uno, Nano, ESP32, ESP8266, etc.)
- BQ34Z100-R2 Battery Management System
- Lithium battery pack (compatible with your configuration)
- Pull-up resistors (2x 4.7kΩ for I2C)
- Decoupling capacitors (0.1µF ceramic)
- Connecting wires
- Optional: LED indicator for alarms

### Recommended Tools
- Multimeter for verification
- Logic analyzer (optional, for debugging)
- Thermal sensor (optional, for calibration)

## Pin Connections

### Basic I2C Connection
| BQ34Z100-R2 Pin | Arduino Pin | Notes |
|-----------------|-------------|-------|
| SDA | SDA (A4 on Uno/Nano) | I2C Data |
| SCL | SCL (A5 on Uno/Nano) | I2C Clock |
| GND | GND | Ground |
| VCC | 3.3V | Power Supply |

### Platform-Specific I2C Pins
| Platform | SDA | SCL | Notes |
|----------|-----|-----|-------|
| Arduino Uno/Nano | A4 | A5 | |
| Arduino Mega | 20 | 21 | |
| ESP32 | GPIO21 | GPIO22 | Configurable |
| ESP8266 | GPIO4 | GPIO5 | Configurable |

### Optional Connections
| BQ34Z100-R2 Pin | Arduino Pin | Purpose |
|-----------------|-------------|----------|
| ALERT | Any Digital Pin | Alarm monitoring |
| TS | Analog Input | Temperature sensing |
| PRES | Digital Input | Battery presence |

## Wiring Diagrams

### Basic Setup
```
                                  4.7kΩ     4.7kΩ
                                   ┌─┴─┐     ┌─┴─┐
                                   │   │     │   │
┌──────────────┐              VCC─┘   │     │   │         ┌─────────────┐
│              │                      │     │             │             │
│           SDA├──────────────────────┴─────┴────────────┤SDA          │
│  Arduino    │                            │             │ BQ34Z100-R2  │
│           SCL├────────────────────────────┴────────────┤SCL          │
│              │                                         │             │
│           GND├─────────────────────────────────────────┤GND          │
│              │                                         │             │
│           3.3V├────────────────────────────────────────┤VCC          │
└──────────────┘                                         └─────────────┘
```

### With Optional Features
```
┌──────────────┐                                         ┌─────────────┐
│              │                                         │             │
│           SDA├─────────────[4.7kΩ]──┬─────────────────┤SDA          │
│  Arduino    │                       │                 │ BQ34Z100-R2  │
│           SCL├─────────────[4.7kΩ]──┴─────────────────┤SCL          │
│              │                                         │             │
│           GND├─────────────────────────────────────────┤GND          │
│              │                                         │             │
│           3.3V├────────────────────────────────────────┤VCC          │
│              │                                         │             │
│           D2 ├─────────────────────────────────────────┤ALERT        │
│              │                                         │             │
│           A0 ├─────────────────────────────────────────┤TS           │
│              │                                         │             │
│           D3 ├─────────────────────────────────────────┤PRES         │
└──────────────┘                                         └─────────────┘
```

## Battery Configuration

### Supported Battery Types
- Li-ion (Default)
- LiPo
- LiFePO4

### Battery Pack Requirements
- Voltage Range: 2.8V to 4.2V (configurable)
- Maximum Current: Based on configuration
- Temperature Sensor: 10K NTC thermistor recommended

### Pack Configuration Example
```cpp
BMSConfig config;
config.chemistry = BatteryChemistry::LIION;
config.designCapacity = 2000;         // 2000mAh
config.overvoltageThreshold = 4200;   // 4.2V
config.undervoltageThreshold = 2800;  // 2.8V
config.temperatureLimit = 3230;       // 50°C
bms.setConfiguration(config);
```

## Platform-Specific Setup

### Arduino Uno/Nano
```cpp
#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;  // Uses default Wire
```

### ESP32
```cpp
#include <Wire.h>
#include <BMSLib.h>

// Optional: Use alternate I2C pins
Wire.begin(SDA_PIN, SCL_PIN);
BMSLib bms(Wire);
```

### ESP8266
```cpp
#include <Wire.h>
#include <BMSLib.h>

// Optional: Use alternate I2C pins
Wire.begin(SDA_PIN, SCL_PIN);
BMSLib bms(Wire);
```

## Safety Considerations

### General Safety
- Always verify battery polarity before connection
- Use appropriate gauge wires for current requirements
- Include proper fusing for overcurrent protection
- Ensure adequate ventilation for the battery pack

### Voltage Safety
- Never exceed maximum voltage ratings
- Verify voltage configurations before connecting battery
- Include voltage divider if required by your setup

### Temperature Safety
- Place temperature sensor in proper thermal contact
- Ensure temperature thresholds are properly configured
- Provide adequate thermal management

### Current Safety
- Size wires appropriately for maximum current
- Configure overcurrent protection thresholds
- Include current sense resistor of appropriate rating

## Troubleshooting

### Common Issues

#### No Communication
1. Verify I2C connections
2. Check pull-up resistors
3. Verify power supply voltage
4. Check device address (default 0x55)

#### Incorrect Readings
1. Verify sensor calibration
2. Check temperature sensor connection
3. Verify voltage sense connections
4. Check current sense resistor value

#### Communication Errors
1. Check I2C bus speed
2. Verify wire lengths
3. Check for interference
4. Verify pull-up resistor values

### Diagnostic Steps
1. Use multimeter to verify voltages
2. Monitor I2C traffic with logic analyzer
3. Verify temperature sensor resistance
4. Check all ground connections

### LED Status Indicators
| LED Pattern | Meaning |
|-------------|---------|
| Solid | Normal operation |
| Fast Blink | Alarm condition |
| Slow Blink | Low battery |
| Off | Sleep mode or error |

## Additional Resources
- [BQ34Z100-R2 Datasheet](https://www.ti.com/product/BQ34Z100-R2)
- [Application Notes](https://www.ti.com/lit/an/slua715/slua715.pdf)
- [Technical Reference](https://www.ti.com/lit/ug/sluua65a/sluua65a.pdf)