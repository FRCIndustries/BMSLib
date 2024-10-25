// BMSLib.h
#ifndef BMSLIB_H
#define BMSLIB_H

#include <Arduino.h>
#include <Wire.h>

// Platform detection
#if defined(ESP32)
    #include <freertos/FreeRTOS.h>
    #include <freertos/semphr.h>
    #define BMS_HAS_MUTEX 1
#elif defined(ESP8266)
    #include <FreeRTOS.h>
    #include <semphr.h>
    #define BMS_HAS_MUTEX 1
#else
    #define BMS_HAS_MUTEX 0
#endif

// Version information
#define BMSLIB_VERSION_MAJOR 1
#define BMSLIB_VERSION_MINOR 0
#define BMSLIB_VERSION_PATCH 0

// Register addresses
#define BMS_REG_CONTROL            0x00
#define BMS_REG_TEMPERATURE        0x06
#define BMS_REG_VOLTAGE           0x08
#define BMS_REG_CURRENT           0x0A
#define BMS_REG_CAPACITY          0x0C
#define BMS_REG_SOC               0x0E
#define BMS_REG_SOH               0x0F
#define BMS_REG_CYCLE_COUNT       0x10
#define BMS_REG_DESIGN_CAPACITY   0x12
#define BMS_REG_SAFETY_STATUS     0x50
#define BMS_REG_FULL_CAPACITY     0x14
#define BMS_REG_REMAINING_CAPACITY 0x16
#define BMS_REG_AVG_TIME_EMPTY    0x18
#define BMS_REG_AVG_TIME_FULL     0x1A
#define BMS_REG_AVAIL_ENERGY      0x1C
#define BMS_REG_AVAIL_POWER       0x1E
#define BMS_REG_MAX_ERROR         0x20
#define BMS_REG_CHARGE_VOLTAGE    0x22
#define BMS_REG_CHARGE_CURRENT    0x24

// Constants
#define BMS_I2C_ADDRESS           0x55
#define BMS_DEVICE_ID             0x0100
#define BMS_CONFIG_MODE_ENTER     0x0013
#define BMS_CONFIG_MODE_EXIT      0x0043
#define BMS_SLEEP_COMMAND         0x0011
#define BMS_WAKE_COMMAND          0x0012
#define BMS_WATCHDOG_RESET        0x0016
#define BMS_FACTORY_RESET         0x0041

// Safety status bit masks
#define BMS_SAFETY_OV_MASK        0x8000
#define BMS_SAFETY_UV_MASK        0x4000
#define BMS_SAFETY_OC_MASK        0x2000
#define BMS_SAFETY_OT_MASK        0x1000

// Additional register addresses for alarms
#define BMS_REG_ALARM_STATUS       0x30  // Alarm status register
#define BMS_REG_ALARM_ENABLE      0x31  // Alarm enable register
#define BMS_REG_ALARM_SOC_LOW     0x32  // Low SoC alarm threshold
#define BMS_REG_ALARM_TEMP_HIGH   0x33  // High temperature alarm threshold
#define BMS_REG_ALARM_VOLT_LOW    0x34  // Low voltage alarm threshold
#define BMS_REG_ALARM_VOLT_HIGH   0x35  // High voltage alarm threshold
#define BMS_REG_ALARM_CURRENT     0x36  // Current alarm threshold

// Alarm status/enable bit masks
#define BMS_ALARM_OV              0x8000  // Over Voltage
#define BMS_ALARM_UV              0x4000  // Under Voltage
#define BMS_ALARM_OC              0x2000  // Over Current
#define BMS_ALARM_OT              0x1000  // Over Temperature
#define BMS_ALARM_UT              0x0800  // Under Temperature
#define BMS_ALARM_SOC_LOW         0x0400  // Low State of Charge
#define BMS_ALARM_DISCHG          0x0200  // Discharging
#define BMS_ALARM_CHG             0x0100  // Charging

// Alarm configuration structure
struct BMSAlarmConfig {
    bool enableOverVoltage;
    bool enableUnderVoltage;
    bool enableOverCurrent;
    bool enableOverTemperature;
    bool enableUnderTemperature;
    bool enableLowSoC;
    bool enableDischarging;
    bool enableCharging;
    uint8_t socLowThreshold;     // Percentage (0-100)
    uint16_t tempHighThreshold;  // 0.1K units
    uint16_t tempLowThreshold;   // 0.1K units
    uint16_t voltLowThreshold;   // mV
    uint16_t voltHighThreshold;  // mV
    uint16_t currentThreshold;   // mA

    BMSAlarmConfig() :
        enableOverVoltage(true),
        enableUnderVoltage(true),
        enableOverCurrent(true),
        enableOverTemperature(true),
        enableUnderTemperature(true),
        enableLowSoC(true),
        enableDischarging(false),
        enableCharging(false),
        socLowThreshold(20),         // 20%
        tempHighThreshold(3230),     // 50°C
        tempLowThreshold(2731),      // 0°C
        voltLowThreshold(3000),      // 3.0V
        voltHighThreshold(4200),     // 4.2V
        currentThreshold(2000)       // 2A
    {}
};

// Error states
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

// Battery chemistry types
enum class BatteryChemistry {
    LIION   = 0x0100,
    LIPO    = 0x0101,
    LIFEP04 = 0x0102
};

struct BMSConfig {
    uint16_t overcurrentThreshold;
    uint16_t overvoltageThreshold;
    uint16_t undervoltageThreshold;
    uint16_t temperatureLimit;
    uint16_t designCapacity;
    BatteryChemistry chemistry;
    
    BMSConfig() :
        overcurrentThreshold(2000),
        overvoltageThreshold(4200),
        undervoltageThreshold(2800),
        temperatureLimit(3230),
        designCapacity(2000),
        chemistry(BatteryChemistry::LIION) {}
};

struct BatteryStatus {
    float voltage;
    float current;
    float temperature;
    uint8_t soc;
    uint8_t soh;
    bool isCharging;
    bool isDischarging;
    bool hasError;
    uint16_t safetyStatus;
    uint16_t cycleCount;
    float remainingCapacity;
    float fullChargeCapacity;
    BMSError lastError;
};

class BMSLib {
public:
    BMSLib(TwoWire &wirePort = Wire);
    ~BMSLib();

    bool begin();
    void getVersion(uint8_t &major, uint8_t &minor, uint8_t &patch);
    bool isOnline();
    BMSError getLastError() const;

    // Configuration functions
    bool setConfiguration(const BMSConfig &config);
    bool getConfiguration(BMSConfig &config);
    bool getBatteryStatus(BatteryStatus &status);

    // Raw data reading functions
    uint16_t readVoltage();
    int16_t  readCurrent();
    uint16_t readCapacity();
    uint16_t readTemperature();
    uint16_t readFullChargeCapacity();
    uint16_t readRemainingCapacity();
    uint16_t readSoC();
    uint16_t readSoH();
    uint16_t readCycleCount();
    uint16_t readDesignCapacity();
    uint16_t readSafetyStatus();

    // Unit conversion functions
    float readVoltage_inVolts();
    float readCurrent_inAmps();
    float readCapacity_inAmpHours();
    float readTemperature_inCelsius();
    float readFullChargeCapacity_inAmpHours();
    float readRemainingCapacity_inAmpHours();
    float readSoC_inPercentage();
    float readSoH_inPercentage();

    // Additional BQ34Z100 specific functions
    uint16_t getAverageTimeToEmpty();
    uint16_t getAverageTimeToFull();
    uint16_t getAvailableEnergy();
    uint16_t getAvailablePower();
    uint8_t getMaxError();
    uint16_t getChargeVoltage();
    uint16_t getChargeCurrent();
    
    bool setChargeVoltage(uint16_t voltage);
    bool setChargeCurrent(uint16_t current);

    float getAvailableEnergy_inWh();
    float getAvailablePower_inW();
    float getChargeVoltage_inVolts();
    float getChargeCurrent_inAmps();

    // Power management
    bool sleep();
    bool wake();
    bool resetWatchdog();

    // Safety status checks
    bool isOverVoltage();
    bool isUnderVoltage();
    bool isOverCurrent();
    bool isOverTemperature();

    // Configuration mode
    bool enterConfigMode();
    bool exitConfigMode();
    
    // Factory reset
    bool factoryReset();
	
	// Alarm configuration functions
    bool setAlarmConfig(const BMSAlarmConfig &config);
    bool getAlarmConfig(BMSAlarmConfig &config);
    uint16_t getAlarmStatus();
    bool clearAlarms();
    
    // Individual alarm status checks
    bool isOverVoltageAlarm();
    bool isUnderVoltageAlarm();
    bool isOverCurrentAlarm();
    bool isOverTemperatureAlarm();
    bool isUnderTemperatureAlarm();
    bool isLowSoCAlarm();
    bool isDischargingAlarm();
    bool isChargingAlarm();
    
    // Individual alarm threshold setters
    bool setLowSoCAlarm(uint8_t threshold);
    bool setHighTemperatureAlarm(uint16_t threshold);
    bool setLowTemperatureAlarm(uint16_t threshold);
    bool setLowVoltageAlarm(uint16_t threshold);
    bool setHighVoltageAlarm(uint16_t threshold);
    bool setCurrentAlarm(uint16_t threshold);

private:
    // Constants
    static const uint32_t I2C_TIMEOUT_MS = 100;
    static const uint8_t MAX_RETRY_COUNT = 3;
    static const uint16_t TEMP_COEFFICIENT = 100;
    static const uint16_t MIN_VOLTAGE = 2000;
    static const uint16_t MAX_VOLTAGE = 4500;
    static const int16_t MAX_CURRENT = 5000;
    static const uint16_t MAX_TEMPERATURE = 3430;
    static const uint16_t MAX_CHARGE_VOLTAGE = 4500;
    static const uint16_t MAX_CHARGE_CURRENT = 5000;

    // Member variables
    TwoWire *_wire;
    bool _configMode;
    BMSError _lastError;

    #if BMS_HAS_MUTEX
    SemaphoreHandle_t _i2cMutex;
    #endif

    // Function pointer type and context structure for retry mechanism
    typedef bool (*RetryCallback)(void* context);
    
    struct I2CContext {
        BMSLib* instance;
        uint8_t command;
        uint16_t* value;
        uint16_t data;
    };

    // Platform-specific mutex management
    bool takeMutex();
    void giveMutex();
    
    // I2C operations
    bool retryOperation(RetryCallback callback, void* context);
    bool readWordDirect(uint8_t command, uint16_t& value);
    bool writeWordDirect(uint8_t command, uint16_t data);
    bool readWord(uint8_t command, uint16_t &value);
    bool writeWord(uint8_t command, uint16_t data);
    bool writeBlockData(uint8_t offset, uint8_t data);
    bool readBlockData(uint8_t offset, uint8_t &data);
    bool writeDataFlash(uint16_t address, uint8_t *data, uint8_t length);
    bool readDataFlash(uint16_t address, uint8_t *data, uint8_t length);
    
    // Helper functions
    uint8_t computeChecksum();
    float compensateTemperature(float voltage, float temperature);
    bool validateValue(uint16_t value, uint16_t min, uint16_t max);
    bool validateTemperature(uint16_t temp);
    bool validateVoltage(uint16_t voltage);
    bool validateCurrent(int16_t current);
    bool validateChargeVoltage(uint16_t voltage);
    bool validateChargeCurrent(uint16_t current);
};

#endif // BMSLIB_H