/*
 * BMSLib - Alarms Example
 * 
 * This example demonstrates the alarm functionality of the BMSLib library,
 * including setting up alarms, monitoring them, and handling alerts.
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, ESP32, ESP8266, etc.)
 * - BQ34Z100-R2 BMS connected via I2C
 * - Battery pack connected to BMS
 * 
 * Optional:
 * - LED connected to pin 13 for visual alarm indication
 */

#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;
const int alarmLedPin = 13;

void configureAlarms() {
  Serial.println("Configuring alarms...");
  
  BMSAlarmConfig alarmConfig;
  
  // Configure which alarms are enabled
  alarmConfig.enableOverVoltage = true;
  alarmConfig.enableUnderVoltage = true;
  alarmConfig.enableOverCurrent = true;
  alarmConfig.enableOverTemperature = true;
  alarmConfig.enableUnderTemperature = true;
  alarmConfig.enableLowSoC = true;
  alarmConfig.enableDischarging = false;
  alarmConfig.enableCharging = false;

  // Set alarm thresholds
  alarmConfig.socLowThreshold = 20;        // Alert at 20% SoC
  alarmConfig.tempHighThreshold = 3230;    // 50°C
  alarmConfig.tempLowThreshold = 2731;     // 0°C
  alarmConfig.voltLowThreshold = 3000;     // 3.0V
  alarmConfig.voltHighThreshold = 4200;    // 4.2V
  alarmConfig.currentThreshold = 2000;     // 2A

  if (!bms.setAlarmConfig(alarmConfig)) {
    Serial.println("Failed to configure alarms!");
    Serial.printf("Error: %d\n", (int)bms.getLastError());
    return;
  }

  Serial.println("Alarms configured successfully!");
  printAlarmConfig(alarmConfig);
}

void printAlarmConfig(const BMSAlarmConfig& config) {
  Serial.println("\nAlarm Configuration:");
  Serial.println("-------------------");
  Serial.printf("Over Voltage Alarm: %s\n", config.enableOverVoltage ? "Enabled" : "Disabled");
  Serial.printf("Under Voltage Alarm: %s\n", config.enableUnderVoltage ? "Enabled" : "Disabled");
  Serial.printf("Over Current Alarm: %s\n", config.enableOverCurrent ? "Enabled" : "Disabled");
  Serial.printf("Over Temperature Alarm: %s\n", config.enableOverTemperature ? "Enabled" : "Disabled");
  Serial.printf("Under Temperature Alarm: %s\n", config.enableUnderTemperature ? "Enabled" : "Disabled");
  Serial.printf("Low SoC Alarm: %s\n", config.enableLowSoC ? "Enabled" : "Disabled");
  
  Serial.println("\nAlarm Thresholds:");
  Serial.printf("Low SoC: %d%%\n", config.socLowThreshold);
  Serial.printf("High Temperature: %.1f°C\n", (config.tempHighThreshold / 10.0) - 273.15);
  Serial.printf("Low Temperature: %.1f°C\n", (config.tempLowThreshold / 10.0) - 273.15);
  Serial.printf("Low Voltage: %.2fV\n", config.voltLowThreshold / 1000.0);
  Serial.printf("High Voltage: %.2fV\n", config.voltHighThreshold / 1000.0);
  Serial.printf("Current Threshold: %.2fA\n", config.currentThreshold / 1000.0);
}

void checkAlarms() {
  uint16_t alarmStatus = bms.getAlarmStatus();
  bool hasAlarm = false;

  Serial.println("\nChecking Alarms:");
  Serial.println("---------------");

  if (bms.isOverVoltageAlarm()) {
    Serial.println("ALARM: Over Voltage!");
    hasAlarm = true;
  }
  
  if (bms.isUnderVoltageAlarm()) {
    Serial.println("ALARM: Under Voltage!");
    hasAlarm = true;
  }
  
  if (bms.isOverCurrentAlarm()) {
    Serial.println("ALARM: Over Current!");
    hasAlarm = true;
  }
  
  if (bms.isOverTemperatureAlarm()) {
    Serial.println("ALARM: Over Temperature!");
    hasAlarm = true;
  }
  
  if (bms.isUnderTemperatureAlarm()) {
    Serial.println("ALARM: Under Temperature!");
    hasAlarm = true;
  }
  
  if (bms.isLowSoCAlarm()) {
    Serial.println("ALARM: Low State of Charge!");
    hasAlarm = true;
  }
  
  if (bms.isDischargingAlarm()) {
    Serial.println("ALARM: Discharging!");
    hasAlarm = true;
  }
  
  if (bms.isChargingAlarm()) {
    Serial.println("ALARM: Charging!");
    hasAlarm = true;
  }

  // Update LED status
  digitalWrite(alarmLedPin, hasAlarm ? HIGH : LOW);

  if (!hasAlarm) {
    Serial.println("No active alarms");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(alarmLedPin, OUTPUT);
  digitalWrite(alarmLedPin, LOW);

  Serial.println("BMSLib Alarms Example");
  Serial.println("--------------------");

  if (!bms.begin()) {
    Serial.println("Failed to initialize BMS!");
    while (1) {
      digitalWrite(alarmLedPin, HIGH);
      delay(500);
      digitalWrite(alarmLedPin, LOW);
      delay(500);
    }
  }

  // Configure alarms
  configureAlarms();

  // Clear any existing alarms
  if (bms.clearAlarms()) {
    Serial.println("Cleared existing alarms");
  }
}

void loop() {
  // Print current battery parameters
  Serial.println("\nCurrent Battery Status:");
  Serial.printf("Voltage: %.2fV\n", bms.readVoltage_inVolts());
  Serial.printf("Current: %.2fA\n", bms.readCurrent_inAmps());
  Serial.printf("Temperature: %.1f°C\n", bms.readTemperature_inCelsius());
  Serial.printf("SoC: %d%%\n", bms.readSoC());

  // Check for alarms
  checkAlarms();

  // Example of modifying single alarm threshold
  static uint8_t counter = 0;
  if (++counter >= 12) {  // Every minute (12 * 5 seconds)
    counter = 0;
    
    // Toggle between 20% and 30% for demo purposes
    static bool toggleThreshold = false;
    uint8_t newThreshold = toggleThreshold ? 20 : 30;
    
    Serial.printf("\nChanging Low SoC alarm threshold to %d%%\n", newThreshold);
    if (!bms.setLowSoCAlarm(newThreshold)) {
      Serial.println("Failed to update alarm threshold!");
    }
    
    toggleThreshold = !toggleThreshold;
  }

  delay(5000);  // Check every 5 seconds
}

/*
 * Additional Example: Interrupt-based Alarm Monitoring
 * 
 * If your hardware supports interrupts and you've connected
 * the BMS's alert pin to an interrupt-capable GPIO, you can
 * use the following code:
 *
 * const int alertPin = 2;  // Use an interrupt-capable pin
 * volatile bool alertReceived = false;
 * 
 * void alertISR() {
 *   alertReceived = true;
 * }
 * 
 * void setup() {
 *   // ... other setup code ...
 *   
 *   pinMode(alertPin, INPUT_PULLUP);
 *   attachInterrupt(digitalPinToInterrupt(alertPin), alertISR, FALLING);
 * }
 * 
 * void loop() {
 *   if (alertReceived) {
 *     alertReceived = false;
 *     checkAlarms();
 *   }
 *   
 *   // ... other loop code ...
 * }
 */