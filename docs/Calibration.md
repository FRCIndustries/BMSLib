# BMSLib Calibration Guide

## Table of Contents
- [Overview](#overview)
- [Required Equipment](#required-equipment)
- [Safety Precautions](#safety-precautions)
- [Voltage Calibration](#voltage-calibration)
- [Current Calibration](#current-calibration)
- [Temperature Calibration](#temperature-calibration)
- [Capacity Learning](#capacity-learning)
- [Troubleshooting](#troubleshooting)

## Overview

Proper calibration of the BQ34Z100-R2 is essential for accurate battery monitoring. The calibration process involves:
1. Voltage calibration
2. Current calibration
3. Temperature calibration
4. Capacity learning cycle

All calibration procedures should be performed in a controlled environment (20-25°C) after the system has been powered on for at least 30 minutes.

## Required Equipment

### Essential Equipment
- Precision multimeter (accuracy ≤0.1%)
- Calibrated current source/load
- Calibrated temperature sensor (±0.5°C accuracy)
- Adjustable power supply (0-5V, current limited)
- High-precision resistors for current sense
- Digital thermometer

### Optional Equipment
- Oscilloscope
- Electronic load
- Temperature chamber
- Data logging system

## Safety Precautions

⚠️ **IMPORTANT SAFETY WARNINGS**
- Disconnect battery before beginning calibration
- Use current-limited power supply
- Monitor temperature during calibration
- Do not exceed voltage/current limits
- Follow proper ESD procedures

## Voltage Calibration

### Preparation
1. Power up the BMS with a stable power supply
2. Connect digital multimeter in parallel
3. Allow 30 minutes warm-up time

### Procedure

```cpp
// Example voltage calibration code
void calibrateVoltage() {
  float actualVoltage;  // From multimeter reading
  
  Serial.println("Enter actual voltage (V): ");
  while (!Serial.available()) delay(100);
  actualVoltage = Serial.parseFloat();
  
  if (bms.calibrateVoltage(actualVoltage * 1000)) {  // Convert to mV
    Serial.println("Voltage calibration successful");
  } else {
    Serial.println("Calibration failed!");
    Serial.print("Error: ");
    Serial.println((int)bms.getLastError());
  }
}
```

### Steps
1. **Initial Setup**
   ```
   Power Supply ──┬── BMS
                 └── Digital Multimeter
   ```

2. **Low Point Calibration (3.0V)**
   - Set power supply to 3.0V
   - Record actual voltage from multimeter
   - Enter calibration mode
   - Store low point reading

3. **High Point Calibration (4.2V)**
   - Set power supply to 4.2V
   - Record actual voltage from multimeter
   - Store high point reading
   - Verify calibration

4. **Verification**
   - Test multiple voltage points
   - Verify accuracy within ±0.1%
   - Log calibration values

## Current Calibration

### Preparation
1. Connect current sense resistor
2. Connect current source/load
3. Connect precision ammeter in series

### Procedure

```cpp
// Example current calibration code
void calibrateCurrent() {
  float actualCurrent;  // From ammeter reading
  
  Serial.println("Apply zero current and press Enter");
  while (!Serial.available()) delay(100);
  Serial.read();
  
  if (!bms.calibrateCurrentZero()) {
    Serial.println("Zero calibration failed!");
    return;
  }
  
  Serial.println("Apply 1A load and enter actual current (A): ");
  while (!Serial.available()) delay(100);
  actualCurrent = Serial.parseFloat();
  
  if (bms.calibrateCurrent(actualCurrent * 1000)) {  // Convert to mA
    Serial.println("Current calibration successful");
  } else {
    Serial.println("Calibration failed!");
  }
}
```

### Steps
1. **Zero Current Calibration**
   ```
   No Load ── BMS (Open Circuit)
   ```
   - Ensure no current flow
   - Calibrate zero point
   - Verify stable reading

2. **Charge Current Calibration**
   ```
   Power Supply ── Ammeter ── BMS ── Load
   ```
   - Apply known charge current (e.g., 1A)
   - Record actual current
   - Store calibration point

3. **Discharge Current Calibration**
   - Reverse current flow
   - Apply known discharge current
   - Store calibration point
   - Verify bidirectional accuracy

4. **Verification**
   - Test multiple current levels
   - Verify both charging/discharging
   - Check accuracy at low currents

## Temperature Calibration

### Preparation
1. Connect temperature sensor
2. Prepare temperature reference
3. Allow system to stabilize

### Procedure

```cpp
// Example temperature calibration code
void calibrateTemperature() {
  float actualTemp;  // From reference thermometer
  
  Serial.println("Enter actual temperature (°C): ");
  while (!Serial.available()) delay(100);
  actualTemp = Serial.parseFloat();
  
  // Convert to 0.1K units
  uint16_t tempK = (uint16_t)((actualTemp + 273.15) * 10);
  
  if (bms.calibrateTemperature(tempK)) {
    Serial.println("Temperature calibration successful");
  } else {
    Serial.println("Calibration failed!");
  }
}
```

### Steps
1. **Room Temperature Calibration**
   - Allow system to reach equilibrium
   - Record reference temperature
   - Calibrate at room temperature

2. **Multi-Point Calibration (Optional)**
   - Test at various temperatures
   - Store calibration points
   - Verify temperature curve

3. **Verification**
   - Test multiple temperature points
   - Verify accuracy ±1°C
   - Check temperature updates

## Capacity Learning

### Preparation
1. Fully charge battery
2. Connect electronic load
3. Prepare for discharge cycle

### Procedure

```cpp
// Example capacity learning monitoring code
void monitorCapacityLearning() {
  BatteryStatus status;
  
  while (bms.getBatteryStatus(status)) {
    Serial.printf("SoC: %d%%  Capacity: %.2fAh\n", 
                 status.soc, status.remainingCapacity);
    
    if (status.soc == 0) {
      Serial.println("Learning cycle complete");
      break;
    }
    delay(60000);  // Check every minute
  }
}
```

### Steps
1. **Initial Setup**
   - Charge to 100% SoC
   - Allow rest period
   - Record initial capacity

2. **Discharge Cycle**
   - Apply constant discharge
   - Monitor voltage/current
   - Record discharge curve

3. **Charge Cycle**
   - Apply standard charge
   - Monitor charge acceptance
   - Record full capacity

4. **Verification**
   - Compare learned capacity
   - Verify SoC accuracy
   - Check capacity updates

## Troubleshooting

### Common Issues

#### Voltage Calibration
- Unstable readings
- Non-linear response
- Calibration value rejection

#### Current Calibration
- Zero drift
- Non-linear response
- Offset errors

#### Temperature Calibration
- Slow response
- Noise in readings
- Sensor placement issues

### Solutions

1. **Voltage Issues**
   - Check power supply stability
   - Verify connection integrity
   - Repeat calibration procedure

2. **Current Issues**
   - Check sense resistor value
   - Verify current path
   - Clean all connections

3. **Temperature Issues**
   - Improve thermal contact
   - Shield from airflow
   - Allow longer stabilization

### Verification Procedures

1. **Voltage Verification**
   ```cpp
   void verifyVoltage() {
     float voltage = bms.readVoltage_inVolts();
     // Compare with multimeter reading
     // Should be within ±0.1%
   }
   ```

2. **Current Verification**
   ```cpp
   void verifyCurrent() {
     float current = bms.readCurrent_inAmps();
     // Compare with ammeter reading
     // Should be within ±1%
   }
   ```

3. **Temperature Verification**
   ```cpp
   void verifyTemperature() {
     float temp = bms.readTemperature_inCelsius();
     // Compare with reference thermometer
     // Should be within ±1°C
   }
   ```

## Additional Notes

- Keep detailed calibration records
- Perform calibration annually
- Verify calibration after firmware updates
- Monitor drift over time
- Consider environmental factors