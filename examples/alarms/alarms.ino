/*
 * BMSLib - Advanced Example
 * 
 * This example demonstrates advanced features including:
 * - Battery status monitoring
 * - Power management
 * - Configuration
 * - Error handling
 * - Temperature compensation
 * - Available energy/power monitoring
 * 
 * Hardware Requirements:
 * - Arduino board (Uno, Nano, ESP32, ESP8266, etc.)
 * - BQ34Z100-R2 BMS connected via I2C
 * - Battery pack connected to BMS
 */

#include <Wire.h>
#include <BMSLib.h>

BMSLib bms;

void setupBMS() {
    Serial.println("\nConfiguring BMS...");
    
    BMSConfig config;
    // Customize configuration for your battery pack
    config.chemistry = BatteryChemistry::LIION;
    config.designCapacity = 2000;         // 2000mAh
    config.overvoltageThreshold = 4200;   // 4.2V
    config.undervoltageThreshold = 2800;  // 2.8V
    config.overcurrentThreshold = 3000;   // 3A
    config.temperatureLimit = 3230;       // 50°C
    
    if (!bms.setConfiguration(config)) {
        Serial.println("Failed to configure BMS!");
        Serial.printf("Error: %d\n", static_cast<int>(bms.getLastError()));
        return;
    }
    
    Serial.println("BMS configured successfully!");
    printConfiguration(config);
}

void printConfiguration(const BMSConfig& config) {
    Serial.println("\nBMS Configuration:");
    Serial.println("-------------------");
    Serial.printf("Chemistry: %s\n", 
        config.chemistry == BatteryChemistry::LIION ? "Li-ion" :
        config.chemistry == BatteryChemistry::LIPO ? "LiPo" :
        config.chemistry == BatteryChemistry::LIFEP04 ? "LiFePO4" : "Unknown");
    Serial.printf("Design Capacity: %dmAh\n", config.designCapacity);
    Serial.printf("Overvoltage Threshold: %.2fV\n", config.overvoltageThreshold / 1000.0f);
    Serial.printf("Undervoltage Threshold: %.2fV\n", config.undervoltageThreshold / 1000.0f);
    Serial.printf("Overcurrent Threshold: %.2fA\n", config.overcurrentThreshold / 1000.0f);
    Serial.printf("Temperature Limit: %.1f°C\n", (config.temperatureLimit / 10.0f) - 273.15f);
}

void printBatteryStatus() {
    BatteryStatus status;
    if (!bms.getBatteryStatus(status)) {
        Serial.println("Failed to read battery status!");
        Serial.printf("Error: %d\n", static_cast<int>(bms.getLastError()));
        return;
    }

    Serial.println("\nBattery Status:");
    Serial.println("--------------");
    Serial.printf("Voltage: %.2fV\n", status.voltage);
    Serial.printf("Current: %.2fA %s\n", abs(status.current), 
        status.isCharging ? "(Charging)" : 
        status.isDischarging ? "(Discharging)" : "(Idle)");
    Serial.printf("Temperature: %.1f°C\n", status.temperature);
    Serial.printf("State of Charge: %d%%\n", status.soc);
    Serial.printf("State of Health: %d%%\n", status.soh);
    Serial.printf("Cycle Count: %d\n", status.cycleCount);
    Serial.printf("Remaining Capacity: %.2fAh\n", status.remainingCapacity);
    Serial.printf("Full Charge Capacity: %.2fAh\n", status.fullChargeCapacity);
    
    if (status.hasError) {
        Serial.println("\nWarnings/Errors:");
        if (bms.isOverVoltage()) Serial.println("- Over Voltage!");
        if (bms.isUnderVoltage()) Serial.println("- Under Voltage!");
        if (bms.isOverCurrent()) Serial.println("- Over Current!");
        if (bms.isOverTemperature()) Serial.println("- Over Temperature!");
    }

    // Power predictions
    uint16_t timeToEmpty = bms.getAverageTimeToEmpty();
    if (timeToEmpty != 65535) {
        Serial.printf("Estimated Time to Empty: %d minutes\n", timeToEmpty);
    }
    
    uint16_t timeToFull = bms.getAverageTimeToFull();
    if (timeToFull != 65535) {
        Serial.printf("Estimated Time to Full: %d minutes\n", timeToFull);
    }

    // Available energy/power
    float availableEnergy = bms.getAvailableEnergy_inWh();
    float availablePower = bms.getAvailablePower_inW();
    Serial.printf("Available Energy: %.1fWh\n", availableEnergy);
    Serial.printf("Available Power: %.1fW\n", availablePower);

    // Get error margin
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
        Serial.printf("Error: %d\n", static_cast<int>(bms.getLastError()));
        while (1) delay(1000);
    }
    
    // Configure the BMS
    setupBMS();
}

void loop() {
    static unsigned long lastUpdate = 0;
    const unsigned long UPDATE_INTERVAL = 5000; // Update every 5 seconds
    
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = millis();
        printBatteryStatus();
        
        // Reset watchdog
        bms.resetWatchdog();
    }
    
    // Check for serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 's':
                Serial.println("Putting BMS to sleep...");
                if (bms.sleep()) {
                    Serial.println("BMS is now in sleep mode");
                }
                break;
                
            case 'w':
                Serial.println("Waking BMS...");
                if (bms.wake()) {
                    Serial.println("BMS is now awake");
                }
                break;
                
            case 'r':
                Serial.println("Resetting BMS to factory defaults...");
                if (bms.factoryReset()) {
                    Serial.println("Factory reset successful");
                    setupBMS();  // Reconfigure BMS
                }
                break;
        }
    }
}