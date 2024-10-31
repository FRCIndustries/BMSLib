/*
 * BMSLib - Basic Example
 * 
 * This example demonstrates the basic functionality of the BMSLib library,
 * showing how to read battery parameters and handle basic errors.
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, ESP32, ESP8266, etc.)
 * - BQ34Z100-R2 BMS connected via I2C
 * - Battery pack connected to BMS
 * 
 * Connections:
 * - BMS SDA -> Arduino SDA (A4 on Uno/Nano)
 * - BMS SCL -> Arduino SCL (A5 on Uno/Nano)
 * - BMS GND -> Arduino GND
 */

#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) delay(10);  // Wait for Serial on boards that need it
    
    Serial.println("BMSLib Basic Example");
    Serial.println("-------------------");
    
    // Initialize BMS
    if (!bms.begin()) {
        Serial.println("Failed to initialize BMS!");
        Serial.print("Error: ");
        switch (bms.getLastError()) {
            case BMSError::COMMUNICATION_ERROR:
                Serial.println("Communication failed - check connections");
                break;
            case BMSError::INITIALIZATION_ERROR:
                Serial.println("Device initialization failed");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        while (1) delay(1000);  // Halt
    }
    
    Serial.println("BMS initialized successfully!");
    
    // Print library version
    uint8_t major, minor, patch;
    bms.getVersion(major, minor, patch);
    Serial.printf("Library Version: %d.%d.%d\n", major, minor, patch);
}

void loop() {
    // Read and display basic battery parameters
    Serial.println("\nBattery Status:");
    Serial.println("---------------");
    
    // Read voltage
    float voltage = bms.readVoltage_inVolts();
    Serial.printf("Voltage: %.2fV\n", voltage);
    
    // Read current (positive = charging, negative = discharging)
    float current = bms.readCurrent_inAmps();
    Serial.printf("Current: %.2fA", current);
    if (current > 0) {
        Serial.println(" (Charging)");
    } else if (current < 0) {
        Serial.println(" (Discharging)");
    } else {
        Serial.println(" (Idle)");
    }
    
    // Read temperature
    float temp = bms.readTemperature_inCelsius();
    Serial.printf("Temperature: %.1f°C\n", temp);
    
    // Read State of Charge
    uint16_t soc = bms.readSoC();
    Serial.printf("State of Charge: %d%%\n", soc);
    
    // Read State of Health
    uint16_t soh = bms.readSoH();
    Serial.printf("State of Health: %d%%\n", soh);
    
    // Read capacity info
    float capacity = bms.readCapacity_inAmpHours();
    float remainingCapacity = bms.readRemainingCapacity_inAmpHours();
    Serial.printf("Battery Capacity: %.2fAh\n", capacity);
    Serial.printf("Remaining Capacity: %.2fAh\n", remainingCapacity);

    // Check for any safety alerts
    if (bms.isOverVoltage()) Serial.println("WARNING: Over Voltage!");
    if (bms.isUnderVoltage()) Serial.println("WARNING: Under Voltage!");
    if (bms.isOverCurrent()) Serial.println("WARNING: Over Current!");
    if (bms.isOverTemperature()) Serial.println("WARNING: Over Temperature!");

    // Print error margin
    uint8_t maxError = bms.getMaxError();
    Serial.printf("Accuracy: ±%d%%\n", maxError);
    
    delay(5000);  // Wait 5 seconds before next reading
}