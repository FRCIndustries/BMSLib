# BMSLib API Reference

## Table of Contents
- [Initialization and Status](#initialization-and-status)
- [Battery Measurements](#battery-measurements)
- [Power Management](#power-management)
- [Alarm System](#alarm-system)
- [Configuration](#configuration)
- [Safety Functions](#safety-functions)
- [Data Structures](#data-structures)

## Initialization and Status

### Constructor
```cpp
BMSLib(TwoWire &wirePort = Wire)
```
Creates a new BMSLib instance using specified I2C port.

### Basic Functions

| Function | Description | Parameters | Return Type | Example |
|----------|-------------|------------|-------------|---------|
| `begin()` | Initialize BMS | None | bool | `if(!bms.begin()) {...}` |
| `isOnline()` | Check BMS communication | None | bool | `if(bms.isOnline()) {...}` |
| `getLastError()` | Get last error code | None | BMSError | `BMSError err = bms.getLastError();` |
| `getVersion()` | Get library version | `uint8_t &major, uint8_t &minor, uint8_t &patch` | void | `bms.getVersion(major, minor, patch);` |

## Battery Measurements

### Raw Measurements

| Function | Description | Return Type | Units | Example |
|----------|-------------|-------------|--------|---------|
| `readVoltage()` | Battery voltage | uint16_t | mV | `uint16_t mV = bms.readVoltage();` |
| `readCurrent()` | Current flow | int16_t | mA | `int16_t mA = bms.readCurrent();` |
| `readTemperature()` | Battery temperature | uint16_t | 0.1K | `uint16_t temp = bms.readTemperature();` |
| `readCapacity()` | Current capacity | uint16_t | mAh | `uint16_t cap = bms.readCapacity();` |
| `readSoC()` | State of Charge | uint16_t | % | `uint16_t soc = bms.readSoC();` |
| `readSoH()` | State of Health | uint16_t | % | `uint16_t soh = bms.readSoH();` |
| `readCycleCount()` | Charge cycles | uint16_t | count | `uint16_t cycles = bms.readCycleCount();` |
| `readDesignCapacity()` | Design capacity | uint16_t | mAh | `uint16_t design = bms.readDesignCapacity();` |
| `readFullChargeCapacity()` | Full charge capacity | uint16_t | mAh | `uint16_t full = bms.readFullChargeCapacity();` |
| `readRemainingCapacity()` | Remaining capacity | uint16_t | mAh | `uint16_t remain = bms.readRemainingCapacity();` |

### Converted Measurements

| Function | Description | Return Type | Units | Example |
|----------|-------------|-------------|--------|---------|
| `readVoltage_inVolts()` | Battery voltage | float | V | `float v = bms.readVoltage_inVolts();` |
| `readCurrent_inAmps()` | Current flow | float | A | `float i = bms.readCurrent_inAmps();` |
| `readTemperature_inCelsius()` | Temperature | float | °C | `float t = bms.readTemperature_inCelsius();` |
| `readCapacity_inAmpHours()` | Capacity | float | Ah | `float c = bms.readCapacity_inAmpHours();` |
| `readFullChargeCapacity_inAmpHours()` | Full charge capacity | float | Ah | `float fc = bms.readFullChargeCapacity_inAmpHours();` |
| `readRemainingCapacity_inAmpHours()` | Remaining capacity | float | Ah | `float rc = bms.readRemainingCapacity_inAmpHours();` |
| `readSoC_inPercentage()` | State of Charge | float | % | `float soc = bms.readSoC_inPercentage();` |
| `readSoH_inPercentage()` | State of Health | float | % | `float soh = bms.readSoH_inPercentage();` |

## Power Management

| Function | Description | Parameters | Return Type | Example |
|----------|-------------|------------|-------------|---------|
| `sleep()` | Enter sleep mode | None | bool | `bms.sleep();` |
| `wake()` | Exit sleep mode | None | bool | `bms.wake();` |
| `resetWatchdog()` | Reset watchdog timer | None | bool | `bms.resetWatchdog();` |
| `getAverageTimeToEmpty()` | Time until battery empty | None | uint16_t | `uint16_t tte = bms.getAverageTimeToEmpty();` |
| `getAverageTimeToFull()` | Time until battery full | None | uint16_t | `uint16_t ttf = bms.getAverageTimeToFull();` |
| `getAvailableEnergy()` | Available energy | None | uint16_t | `uint16_t ae = bms.getAvailableEnergy();` |
| `getAvailablePower()` | Available power | None | uint16_t | `uint16_t ap = bms.getAvailablePower();` |
| `getChargeVoltage()` | Charge voltage setting | None | uint16_t | `uint16_t cv = bms.getChargeVoltage();` |
| `getChargeCurrent()` | Charge current setting | None | uint16_t | `uint16_t cc = bms.getChargeCurrent();` |
| `setChargeVoltage()` | Set charge voltage | uint16_t voltage | bool | `bms.setChargeVoltage(4200);` |
| `setChargeCurrent()` | Set charge current | uint16_t current | bool | `bms.setChargeCurrent(1000);` |

## Alarm System

### Configuration

| Function | Description | Parameters | Return Type | Example |
|----------|-------------|------------|-------------|---------|
| `setAlarmConfig()` | Configure all alarms | const BMSAlarmConfig& | bool | `bms.setAlarmConfig(config);` |
| `getAlarmConfig()` | Get alarm configuration | BMSAlarmConfig& | bool | `bms.getAlarmConfig(config);` |
| `getAlarmStatus()` | Get current alarm status | None | uint16_t | `uint16_t status = bms.getAlarmStatus();` |
| `clearAlarms()` | Clear all alarms | None | bool | `bms.clearAlarms();` |

### Individual Alarm Functions

| Function | Description | Return Type | Example |
|----------|-------------|-------------|---------|
| `isOverVoltageAlarm()` | Check over-voltage alarm | bool | `if(bms.isOverVoltageAlarm()) {...}` |
| `isUnderVoltageAlarm()` | Check under-voltage alarm | bool | `if(bms.isUnderVoltageAlarm()) {...}` |
| `isOverCurrentAlarm()` | Check over-current alarm | bool | `if(bms.isOverCurrentAlarm()) {...}` |
| `isOverTemperatureAlarm()` | Check over-temperature alarm | bool | `if(bms.isOverTemperatureAlarm()) {...}` |
| `isUnderTemperatureAlarm()` | Check under-temperature alarm | bool | `if(bms.isUnderTemperatureAlarm()) {...}` |
| `isLowSoCAlarm()` | Check low SoC alarm | bool | `if(bms.isLowSoCAlarm()) {...}` |
| `isDischargingAlarm()` | Check discharging alarm | bool | `if(bms.isDischargingAlarm()) {...}` |
| `isChargingAlarm()` | Check charging alarm | bool | `if(bms.isChargingAlarm()) {...}` |

### Alarm Threshold Settings

| Function | Description | Parameters | Return Type | Example |
|----------|-------------|------------|-------------|---------|
| `setLowSoCAlarm()` | Set low SoC threshold | uint8_t threshold | bool | `bms.setLowSoCAlarm(20);` |
| `setHighTemperatureAlarm()` | Set high temperature threshold | uint16_t threshold | bool | `bms.setHighTemperatureAlarm(3230);` |
| `setLowTemperatureAlarm()` | Set low temperature threshold | uint16_t threshold | bool | `bms.setLowTemperatureAlarm(2731);` |
| `setLowVoltageAlarm()` | Set low voltage threshold | uint16_t threshold | bool | `bms.setLowVoltageAlarm(3000);` |
| `setHighVoltageAlarm()` | Set high voltage threshold | uint16_t threshold | bool | `bms.setHighVoltageAlarm(4200);` |
| `setCurrentAlarm()` | Set current threshold | uint16_t threshold | bool | `bms.setCurrentAlarm(2000);` |

## Configuration

| Function | Description | Parameters | Return Type | Example |
|----------|-------------|------------|-------------|---------|
| `setConfiguration()` | Set BMS configuration | const BMSConfig& | bool | `bms.setConfiguration(config);` |
| `getConfiguration()` | Get BMS configuration | BMSConfig& | bool | `bms.getConfiguration(config);` |
| `enterConfigMode()` | Enter configuration mode | None | bool | `bms.enterConfigMode();` |
| `exitConfigMode()` | Exit configuration mode | None | bool | `bms.exitConfigMode();` |
| `factoryReset()` | Reset to factory defaults | None | bool | `bms.factoryReset();` |

## Safety Functions

| Function | Description | Return Type | Example |
|----------|-------------|-------------|---------|
| `isOverVoltage()` | Check over-voltage condition | bool | `if(bms.isOverVoltage()) {...}` |
| `isUnderVoltage()` | Check under-voltage condition | bool | `if(bms.isUnderVoltage()) {...}` |
| `isOverCurrent()` | Check over-current condition | bool | `if(bms.isOverCurrent()) {...}` |
| `isOverTemperature()` | Check over-temperature condition | bool | `if(bms.isOverTemperature()) {...}` |

## Data Structures

### BMSError
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

### BMSConfig
```cpp
struct BMSConfig {
    uint16_t overcurrentThreshold;   // mA
    uint16_t overvoltageThreshold;   // mV
    uint16_t undervoltageThreshold;  // mV
    uint16_t temperatureLimit;       // 0.1K
    uint16_t designCapacity;         // mAh
    BatteryChemistry chemistry;
};
```

### BMSAlarmConfig
```cpp
struct BMSAlarmConfig {
    bool enableOverVoltage;
    bool enableUnderVoltage;
    bool enableOverCurrent;
    bool enableOverTemperature;
    bool enableUnderTemperature;
    bool enableLowSoC;
    bool enableDischarging;
    bool enableCharging;
    uint8_t socLowThreshold;
    uint16_t tempHighThreshold;
    uint16_t tempLowThreshold;
    uint16_t voltLowThreshold;
    uint16_t voltHighThreshold;
    uint16_t currentThreshold;
};
```

### BatteryStatus
```cpp
struct BatteryStatus {
    float voltage;             // V
    float current;            // A
    float temperature;        // °C
    uint8_t soc;             // %
    uint8_t soh;             // %
    bool isCharging;
    bool isDischarging;
    bool hasError;
    uint16_t safetyStatus;
    uint16_t cycleCount;
    float remainingCapacity; // Ah
    float fullChargeCapacity;// Ah
    BMSError lastError;
};
```