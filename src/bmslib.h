#ifndef BMSLIB_H
#define BMSLIB_H

#include <Arduino.h>
#include <Wire.h>

// Version information
#define BMSLIB_VERSION_MAJOR 1
#define BMSLIB_VERSION_MINOR 0
#define BMSLIB_VERSION_PATCH 0

// BMS Register Map
#define BMS_I2C_ADDRESS          0x55    // Default I2C address

// Standard Commands
#define BMS_REG_CNTL            0x00
#define BMS_REG_SOC             0x02
#define BMS_REG_ME              0x03
#define BMS_REG_RM              0x04
#define BMS_REG_FCC             0x06
#define BMS_REG_VOLT            0x08
#define BMS_REG_AI              0x0A
#define BMS_REG_TEMP            0x0C
#define BMS_REG_FLAGS           0x0E
#define BMS_REG_CURRENT         0x10
#define BMS_REG_FLAGSB          0x12

// Extended Commands
#define BMS_REG_ATTE            0x18    // AverageTimeToEmpty
#define BMS_REG_ATTF            0x1A    // AverageTimeToFull
#define BMS_REG_PCHG            0x1C    // PassedCharge
#define BMS_REG_DOD0T           0x1E    // DoD0Time
#define BMS_REG_AE              0x24    // AvailableEnergy 
#define BMS_REG_AP              0x26    // AveragePower
#define BMS_REG_SERNUM          0x28    // Serial Number
#define BMS_REG_INTTEMP         0x2A    // Internal Temperature
#define BMS_REG_CC              0x2C    // Cycle Count
#define BMS_REG_SOH             0x2E    // State of Health
#define BMS_REG_CHGV            0x30    // Charge Voltage
#define BMS_REG_CHGI            0x32    // Charge Current
#define BMS_REG_PKCFG           0x3A    // Pack Configuration
#define BMS_REG_DCAP            0x3C    // Design Capacity
#define BMS_REG_DFCLS           0x3E    // Data Flash Class
#define BMS_REG_DFBLK           0x3F    // Data Flash Block

// Calibration Registers
#define BMS_REG_VOLTAGE_CAL     0x0D
#define BMS_REG_CURRENT_CAL     0x0E
#define BMS_REG_TEMP_CAL        0x0F
#define BMS_REG_SHUNT_RESISTANCE 0x10
#define BMS_REG_CAL_STATUS      0x11

// Chemistry and Power Management Registers
#define BMS_REG_CHEM            0x40    // Battery chemistry register
#define BMS_REG_SELF_DISCH      0x41    // Self discharge configuration
#define BMS_REG_POWER_MODE      0x42    // Power mode control
#define BMS_REG_SLEEP_CUR       0x43    // Sleep current threshold
#define BMS_REG_SHUTDOWN_V      0x44    // Shutdown voltage threshold

// BMS Commands
#define BMS_SLEEP_COMMAND       0xA55A
#define BMS_WAKE_COMMAND        0x5AA5
#define BMS_WATCHDOG_RESET      0xCC33
#define BMS_CONFIG_MODE_ENTER   0x5555
#define BMS_CONFIG_MODE_EXIT    0xAAAA
#define BMS_FACTORY_RESET       0x0F0F

// Status bits
#define BMS_STATUS_SLEEP        0x0002  // Sleep mode status bit

class BMSLib {
public:
    // DateTime structure
    struct DateTime {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;

        DateTime() : year(0), month(0), day(0), hour(0), minute(0) {}
        DateTime(uint16_t y, uint8_t mo, uint8_t d, uint8_t h, uint8_t mi) 
            : year(y), month(mo), day(d), hour(h), minute(mi) {}
    };

    // Calibration structures
    struct VoltageCalibration {
        uint16_t actualVoltage;    // Real voltage in mV measured with calibrated meter
        uint16_t measuredVoltage;  // Voltage reported by BMS
    };

    struct CurrentCalibration {
        int16_t actualCurrent;     // Real current in mA measured with calibrated meter
        int16_t measuredCurrent;   // Current reported by BMS
        uint16_t shuntResistance;  // Shunt resistance in μΩ (microohms)
    };

    struct TempCalibration {
        uint16_t actualTemp;       // Real temperature in 0.1°K from calibrated sensor
        uint16_t measuredTemp;     // Temperature reported by BMS
    };

    // Lifetime Statistics Structure
    struct LifetimeStats {
        uint16_t maxTemp;              // Maximum temperature seen (0.1°K)
        uint16_t minTemp;              // Minimum temperature seen (0.1°K)
        int16_t maxChargeCurrent;      // Maximum charge current (mA)
        int16_t maxDischargeCurrent;   // Maximum discharge current (mA)
        uint16_t maxPackVoltage;       // Maximum pack voltage (mV)
        uint16_t minPackVoltage;       // Minimum pack voltage (mV)
        uint16_t updateCount;          // Number of updates to lifetime data
        DateTime lastUpdate;           // Timestamp of last update
    };

    // Detailed Status Structure
    struct DetailedStatus {
        bool isCharging;          // Battery is being charged
        bool isDischarging;       // Battery is being discharged
        bool isBalancing;         // Cells are being balanced
        bool isFull;             // Battery is fully charged
        bool isCalibrated;       // BMS is calibrated
        bool needsUpdate;        // Needs impedance track update
        bool sleepEnabled;       // Sleep mode is enabled
        bool shutdownRequested;  // Shutdown was requested
        uint8_t errorCode;       // Last error code
        uint16_t safetyStatus;   // Safety alert flags
        float stateOfCharge;     // Current SOC (%)
        float stateOfHealth;     // Current SOH (%)
        uint16_t remainingCapacity; // Remaining capacity (mAh)
        uint16_t fullCapacity;     // Full charge capacity (mAh)
        int16_t averageCurrent;    // Average current (mA)
        uint16_t temperature;      // Current temperature (0.1°K)
    };

    // Battery Chemistry Types
    enum class BatteryChemistry {
        LION     = 0x0100,  // Lithium Ion
        LIFEPO4  = 0x0200,  // Lithium Iron Phosphate
        NiMH     = 0x0300,  // Nickel Metal Hydride
        NiCd     = 0x0400,  // Nickel Cadmium
        PbAcid   = 0x0500   // Lead Acid
    };

    // Power Modes
    enum class PowerMode {
        NORMAL    = 0x00,   // Normal operation
        SLEEP     = 0x01,   // Sleep mode
        DEEPSLEEP = 0x02,   // Deep sleep mode
        SHUTDOWN  = 0x03    // Complete shutdown
    };

    // Self-Discharge Configuration
    struct SelfDischargeConfig {
        uint16_t rate;            // Self-discharge rate in 0.1% per day
        uint16_t temperatureCoef; // Temperature coefficient for self-discharge
        bool enabled;             // Enable/disable self-discharge compensation
    };

    // Power Management Configuration
    struct PowerConfig {
        uint16_t sleepCurrent;    // Sleep mode entry current threshold (mA)
        uint16_t shutdownVoltage; // Shutdown voltage threshold (mV)
        uint16_t wakeVoltage;     // Wake-up voltage threshold (mV)
        uint8_t sleepDelay;       // Delay before entering sleep (seconds)
    };
	
	// Capacity Configuration Structure
    struct CapacityConfig {
        uint16_t designCapacity;    // Design capacity in mAh
        uint16_t designEnergy;      // Design energy in mWh
        uint16_t cycleCountThresh;  // Cycle count threshold in mAh
        uint8_t chargeTermination;  // Charge termination percentage
        uint8_t reserveCapacity;    // Reserve capacity percentage
    };

    // Constructor/Destructor
    BMSLib(TwoWire &wirePort = Wire);
    ~BMSLib();

    // Basic functions
    bool begin();
    void getVersion(uint8_t &major, uint8_t &minor, uint8_t &patch);
    bool isOnline();

    // Raw data reading functions
    uint16_t readVoltage();            // Returns millivolts
    int16_t readCurrent();             // Returns milliamps
    uint16_t readCapacity();           // Returns mAh
    uint16_t readTemperature();        // Returns 0.1K
    uint16_t readSoC();                // Returns percentage (0-100%)
    uint16_t readSoH();                // Returns percentage (0-100%)
    uint16_t readCycleCount();         // Returns cycle count
    uint16_t readDesignCapacity();     // Returns mAh
    uint16_t readFullChargeCapacity(); // Returns mAh
    uint16_t readRemainingCapacity();  // Returns mAh
    uint16_t readSafetyStatus();       // Returns safety status flags

    // Helper functions for unit conversion
    float readVoltage_inVolts();
    float readCurrent_inAmps();
    float readCapacity_inAmpHours();
    float readTemperature_inCelsius();
    float readFullChargeCapacity_inAmpHours();
    float readRemainingCapacity_inAmpHours();
    
	// Battery Capacity functions
    bool setDesignCapacity(uint16_t capacity_mAh);  // Set design capacity in mAh
    bool setFullChargeCapacity(uint16_t capacity_mAh);  // Set full charge capacity in mAh
    bool setCapacityConfig(const CapacityConfig& config);
    bool getCapacityConfig(CapacityConfig& config);

    // Charging history functions
    bool getLastChargeTime(DateTime& dateTime);  // Pass by reference version
    DateTime getLastChargeTime();                // Return value version
    uint16_t getChargeCycles();

    // Calibration functions
    bool calibrateVoltage(const VoltageCalibration& cal);
    bool calibrateCurrent(const CurrentCalibration& cal);
    bool calibrateTemperature(const TempCalibration& cal);
    bool performFullCalibration(const VoltageCalibration& vcal, 
                              const CurrentCalibration& ccal,
                              const TempCalibration& tcal);
    bool isCalibrated();
    bool clearCalibration();

    // Lifetime and status functions
    bool getLifetimeStats(LifetimeStats& stats);
    bool resetLifetimeStats();
    bool getDetailedStatus(DetailedStatus& status);

    // Chemistry Management Functions
    bool setBatteryChemistry(BatteryChemistry chemistry);
    BatteryChemistry getBatteryChemistry();
    bool isChemistrySupported(BatteryChemistry chemistry);

    // Self-Discharge Management Functions
    bool configureSelfDischarge(const SelfDischargeConfig& config);
    bool getSelfDischargeConfig(SelfDischargeConfig& config);
    float getEstimatedSelfDischarge();  // Returns estimated self-discharge in percent

    // Extended Power Management Functions
    bool setPowerMode(PowerMode mode);
    PowerMode getPowerMode();
    bool configurePowerSaving(const PowerConfig& config);
    bool getPowerConfig(PowerConfig& config);
    uint16_t getAveragePowerConsumption();  // Returns average power consumption in mW

    // Safety status checks
    bool isOverVoltage();
    bool isUnderVoltage();
    bool isOverCurrent();
    bool isOverTemperature();

    // Power management
    bool sleep();
    bool wake();
    bool resetWatchdog();
    bool isInSleepMode();

    // Configuration mode
    bool enterConfigMode();
    bool exitConfigMode();
    
    // Factory reset
    bool factoryReset();

private:
    // Constants
    static constexpr uint16_t MIN_VOLTAGE = 2000;      // 2.0V minimum valid voltage
    static constexpr uint16_t MAX_VOLTAGE = 4500;      // 4.5V maximum valid voltage
    static constexpr int16_t MAX_CURRENT = 5000;       // 5.0A maximum current
    static constexpr uint16_t MAX_TEMPERATURE = 3430;  // 70°C maximum temperature
    static constexpr float TEMP_COEFFICIENT = 0.0001f;  // Temperature coefficient for voltage compensation

    // Member variables
    TwoWire *_wire;
    bool _configMode;

    // I2C operations
    bool readWord(uint8_t command, uint16_t &value);
    bool writeWord(uint8_t command, uint16_t data);
    
    // Data flash operations
    bool readDataFlash(uint8_t offset, uint8_t* data, uint8_t length);
    bool writeDataFlash(uint8_t offset, const uint8_t* data, uint8_t length);
    
    // Helper functions
    float compensateTemperature(float voltage, float temperature);
    bool validateTemperature(uint16_t temp);
    bool validateVoltage(uint16_t voltage);
    bool validateCurrent(int16_t current);
};

#endif // BMSLIB_H