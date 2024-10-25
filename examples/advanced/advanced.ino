/*
 * BMSLib - Advanced Example
 * 
 * This example demonstrates advanced features of the BMSLib library,
 * including configuration, power management, and detailed status monitoring.
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, ESP32, ESP8266, etc.)
 * - BQ34Z100-R2 BMS connected via I2C
 * - Battery pack connected to BMS
 */

#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;

void printBatteryStatus(const BatteryStatus& status) {
  Serial.println("\nDetailed Battery Status:");
  Serial.println("------------------------");
  Serial.printf("Voltage: %.2fV\n", status.voltage);
  Serial.printf("Current: %.2fA\n", status.current);
  Serial.printf("Temperature: %.1f°C\n", status.temperature);
  Serial.printf("State of Charge: %d%%\n", status.soc);
  Serial.printf("State of Health: %d%%\n", status.soh);
  Serial.printf("Cycle Count: %d\n", status.cycleCount);
  Serial.printf("Remaining Capacity: %.2fAh\n", status.remainingCapacity);
  Serial.printf("Full Charge Capacity: %.2fAh\n", status.fullChargeCapacity);
  Serial.printf("Status: %s\n", status.isCharging ? "Charging" : 
                               (status.isDischarging ? "Discharging" : "Idle"));
  if (status.hasError) {
    Serial.println("WARNING: Error condition detected!");
  }
}

void configureBMS() {
  Serial.println("\nConfiguring BMS...");
  
  BMSConfig config;
  // Customize configuration
  config.overcurrentThreshold = 3000;    // 3A
  config.overvoltageThreshold = 4200;    // 4.2V
  config.undervoltageThreshold = 3000;   // 3.0V
  config.temperatureLimit = 3230;        // 50°C
  config.designCapacity = 2000;          // 2000mAh
  config.chemistry = BatteryChemistry::LIION;
  
  if (!bms.setConfiguration(config)) {
    Serial.println("Failed to configure BMS!");
    Serial.printf("Error: %d\n", (int)bms.getLastError());
    return;
  }
  
  Serial.println("BMS configured successfully!");
}

void printPowerMetrics() {
  Serial.println("\nPower Metrics:");
  Serial.println("--------------");
  
  // Available energy and power
  float energy = bms.getAvailableEnergy_inWh();
  float power = bms.getAvailablePower_inW();
  Serial.printf("Available Energy: %.2fWh\n", energy);
  Serial.printf("Available Power: %.2fW\n", power);
  
  // Charge parameters
  float chargeVoltage = bms.getChargeVoltage_inVolts();
  float chargeCurrent = bms.getChargeCurrent_inAmps();
  Serial.printf("Charge Voltage Limit: %.2fV\n", chargeVoltage);
  Serial.printf("Charge Current Limit: %.2fA\n", chargeCurrent);
  
  // Error margin
  uint8_t maxError = bms.getMaxError();
  Serial.printf("Maximum Error: %d%%\n", maxError);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("BMSLib Advanced Example");
  Serial.println("----------------------");
  
  if (!bms.begin()) {
    Serial.println("Failed to initialize BMS!");
    while (1) delay(1000);
  }
  
  // Configure the BMS
  configureBMS();
}

void loop() {
  // Get comprehensive battery status
  BatteryStatus status;
  if (bms.getBatteryStatus(status)) {
    printBatteryStatus(status);
  } else {
    Serial.println("Failed to read battery status!");
  }
  
  // Print power metrics
  printPowerMetrics();
  
  // Demonstrate power management (example)
  static bool toggle = false;
  if (toggle) {
    // Note: Don't actually use sleep/wake this frequently!
    // This is just for demonstration.
    Serial.println("\nPutting BMS to sleep...");
    if (bms.sleep()) {
      delay(1000);  // Wait a bit
      Serial.println("Waking BMS...");
      if (!bms.wake()) {
        Serial.println("Failed to wake BMS!");
      }
    } else {
      Serial.println("Failed to put BMS to sleep!");
    }
  }
  toggle = !toggle;
  
  // Reset watchdog
  bms.resetWatchdog();
  
  delay(10000);  // Wait 10 seconds before next reading
}