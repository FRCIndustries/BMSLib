// BMSLib.cpp
#include "BMSLib.h"

// Static callback functions for retry mechanism
static bool readWordCallback(void* context) {
    BMSLib::I2CContext* ctx = (BMSLib::I2CContext*)context;
    return ctx->instance->readWordDirect(ctx->command, *ctx->value);
}

static bool writeWordCallback(void* context) {
    BMSLib::I2CContext* ctx = (BMSLib::I2CContext*)context;
    return ctx->instance->writeWordDirect(ctx->command, ctx->data);
}

BMSLib::BMSLib(TwoWire &wirePort) : 
    _wire(&wirePort),
    _configMode(false),
    _lastError(BMSError::NONE) {
    #if BMS_HAS_MUTEX
    _i2cMutex = xSemaphoreCreateMutex();
    #endif
}

BMSLib::~BMSLib() {
    #if BMS_HAS_MUTEX
    if (_i2cMutex != nullptr) {
        vSemaphoreDelete(_i2cMutex);
    }
    #endif
}

bool BMSLib::takeMutex() {
    #if BMS_HAS_MUTEX
    if (xSemaphoreTake(_i2cMutex, pdMS_TO_TICKS(I2C_TIMEOUT_MS)) != pdTRUE) {
        _lastError = BMSError::TIMEOUT_ERROR;
        return false;
    }
    #endif
    return true;
}

void BMSLib::giveMutex() {
    #if BMS_HAS_MUTEX
    xSemaphoreGive(_i2cMutex);
    #endif
}

bool BMSLib::begin() {
    _wire->begin();
    _lastError = BMSError::NONE;
    
    // Allow I2C to stabilize
    delay(100);
    
    // Verify communication
    if (!isOnline()) {
        _lastError = BMSError::INITIALIZATION_ERROR;
        return false;
    }
    
    // Read and verify device ID
    uint16_t deviceId;
    if (!readWord(BMS_REG_CONTROL, deviceId) || deviceId != BMS_DEVICE_ID) {
        _lastError = BMSError::INITIALIZATION_ERROR;
        return false;
    }
    
    // Reset watchdog
    if (!resetWatchdog()) {
        _lastError = BMSError::INITIALIZATION_ERROR;
        return false;
    }
    
    return true;
}

bool BMSLib::retryOperation(RetryCallback callback, void* context) {
    for (uint8_t retry = 0; retry < MAX_RETRY_COUNT; retry++) {
        if (callback(context)) {
            return true;
        }
        delay(10 * (retry + 1));  // Exponential backoff
    }
    _lastError = BMSError::COMMUNICATION_ERROR;
    return false;
}

bool BMSLib::readWordDirect(uint8_t command, uint16_t& value) {
    if (!takeMutex()) {
        return false;
    }
    
    _wire->beginTransmission(BMS_I2C_ADDRESS);
    _wire->write(command);
    if (_wire->endTransmission(false) != 0) {
        giveMutex();
        _lastError = BMSError::COMMUNICATION_ERROR;
        return false;
    }
    
    if (_wire->requestFrom(BMS_I2C_ADDRESS, (uint8_t)2) != 2) {
        giveMutex();
        _lastError = BMSError::COMMUNICATION_ERROR;
        return false;
    }
    
    uint8_t lowByte = _wire->read();
    uint8_t highByte = _wire->read();
    value = (highByte << 8) | lowByte;
    
    giveMutex();
    _lastError = BMSError::NONE;
    return true;
}

bool BMSLib::writeWordDirect(uint8_t command, uint16_t data) {
    if (!takeMutex()) {
        return false;
    }
    
    _wire->beginTransmission(BMS_I2C_ADDRESS);
    _wire->write(command);
    _wire->write(data & 0xFF);        // Low byte
    _wire->write((data >> 8) & 0xFF); // High byte
    bool status = (_wire->endTransmission() == 0);
    
    giveMutex();
    
    if (!status) {
        _lastError = BMSError::COMMUNICATION_ERROR;
    }
    return status;
}

bool BMSLib::readWord(uint8_t command, uint16_t &value) {
    I2CContext context = {
        .instance = this,
        .command = command,
        .value = &value
    };
    return retryOperation(readWordCallback, &context);
}

bool BMSLib::writeWord(uint8_t command, uint16_t data) {
    I2CContext context = {
        .instance = this,
        .command = command,
        .data = data
    };
    return retryOperation(writeWordCallback, &context);
}

// Raw data reading functions with validation
uint16_t BMSLib::readVoltage() {
    uint16_t value;
    if (!readWord(BMS_REG_VOLTAGE, value)) {
        return 0;
    }
    if (!validateVoltage(value)) {
        return 0;
    }
    return value;
}

int16_t BMSLib::readCurrent() {
    uint16_t value;
    if (!readWord(BMS_REG_CURRENT, value)) {
        return 0;
    }
    int16_t current = static_cast<int16_t>(value);
    if (!validateCurrent(current)) {
        return 0;
    }
    return current;
}

uint16_t BMSLib::readTemperature() {
    uint16_t value;
    if (!readWord(BMS_REG_TEMPERATURE, value)) {
        return 0;
    }
    if (!validateTemperature(value)) {
        return 0;
    }
    return value;
}

// Helper functions for unit conversion
float BMSLib::readVoltage_inVolts() {
    uint16_t voltage = readVoltage();
    if (voltage == 0) return 0.0f;
    
    float volts = voltage / 1000.0f;
    float temp = readTemperature_inCelsius();
    
    return compensateTemperature(volts, temp);
}

float BMSLib::readCurrent_inAmps() {
    return readCurrent() / 1000.0f;
}

float BMSLib::readTemperature_inCelsius() {
    uint16_t temp = readTemperature();
    return (temp / 10.0f) - 273.15f;
}

// Configuration management
bool BMSLib::enterConfigMode() {
    if (_configMode) return true;
    
    if (!writeWord(BMS_REG_CONTROL, BMS_CONFIG_MODE_ENTER)) {
        _lastError = BMSError::CONFIGURATION_ERROR;
        return false;
    }
    
    delay(100); // Allow time for mode change
    _configMode = true;
    return true;
}

bool BMSLib::exitConfigMode() {
    if (!_configMode) return true;
    
    if (!writeWord(BMS_REG_CONTROL, BMS_CONFIG_MODE_EXIT)) {
        _lastError = BMSError::CONFIGURATION_ERROR;
        return false;
    }
    
    delay(100); // Allow time for mode change
    _configMode = false;
    return true;
}

// Power management functions
bool BMSLib::sleep() {
    if (_configMode) {
        exitConfigMode();
    }
    return writeWord(BMS_REG_CONTROL, BMS_SLEEP_COMMAND);
}

bool BMSLib::wake() {
    bool success = writeWord(BMS_REG_CONTROL, BMS_WAKE_COMMAND);
    if (success) {
        delay(100); // Allow time for device to wake up
    }
    return success;
}

bool BMSLib::resetWatchdog() {
    return writeWord(BMS_REG_CONTROL, BMS_WATCHDOG_RESET);
}

// Additional BQ34Z100 specific functions
uint16_t BMSLib::getAverageTimeToEmpty() {
    uint16_t value;
    if (!readWord(BMS_REG_AVG_TIME_EMPTY, value)) {
        return 65535;  // Invalid/unknown time
    }
    return value;
}

uint16_t BMSLib::getAverageTimeToFull() {
    uint16_t value;
    if (!readWord(BMS_REG_AVG_TIME_FULL, value)) {
        return 65535;  // Invalid/unknown time
    }
    return value;
}

uint16_t BMSLib::getAvailableEnergy() {
    uint16_t value;
    if (!readWord(BMS_REG_AVAIL_ENERGY, value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::getAvailablePower() {
    uint16_t value;
    if (!readWord(BMS_REG_AVAIL_POWER, value)) {
        return 0;
    }
    return value;
}

uint8_t BMSLib::getMaxError() {
    uint16_t value;
    if (!readWord(BMS_REG_MAX_ERROR, value)) {
        return 100;  // Maximum error if read fails
    }
    return static_cast<uint8_t>(value > 100 ? 100 : value);
}

uint16_t BMSLib::getChargeVoltage() {
    uint16_t value;
    if (!readWord(BMS_REG_CHARGE_VOLTAGE, value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::getChargeCurrent() {
    uint16_t value;
    if (!readWord(BMS_REG_CHARGE_CURRENT, value)) {
        return 0;
    }
    return value;
}

bool BMSLib::setChargeVoltage(uint16_t voltage) {
    if (!validateChargeVoltage(voltage)) {
        return false;
    }

    if (!enterConfigMode()) return false;
    bool success = writeWord(BMS_REG_CHARGE_VOLTAGE, voltage);
    exitConfigMode();
    return success;
}

bool BMSLib::setChargeCurrent(uint16_t current) {
    if (!validateChargeCurrent(current)) {
        return false;
    }

    if (!enterConfigMode()) return false;
    bool success = writeWord(BMS_REG_CHARGE_CURRENT, current);
    exitConfigMode();
    return success;
}

// Unit conversion helpers for new functions
float BMSLib::getAvailableEnergy_inWh() {
    return getAvailableEnergy() / 100.0f;  // Convert from 10mWh to Wh
}

float BMSLib::getAvailablePower_inW() {
    return getAvailablePower() / 1000.0f;  // Convert from mW to W
}

float BMSLib::getChargeVoltage_inVolts() {
    return getChargeVoltage() / 1000.0f;  // Convert from mV to V
}

float BMSLib::getChargeCurrent_inAmps() {
    return getChargeCurrent() / 1000.0f;  // Convert from mA to A
}

// Safety status checks
bool BMSLib::isOverVoltage() {
    uint16_t status = readSafetyStatus();
    return (status & BMS_SAFETY_OV_MASK) != 0;
}

bool BMSLib::isUnderVoltage() {
    uint16_t status = readSafetyStatus();
    return (status & BMS_SAFETY_UV_MASK) != 0;
}

bool BMSLib::isOverCurrent() {
    uint16_t status = readSafetyStatus();
    return (status & BMS_SAFETY_OC_MASK) != 0;
}

bool BMSLib::isOverTemperature() {
    uint16_t status = readSafetyStatus();
    return (status & BMS_SAFETY_OT_MASK) != 0;
}

// Configuration and status
bool BMSLib::setConfiguration(const BMSConfig &config) {
    if (!validateChargeVoltage(config.overvoltageThreshold) ||
        !validateVoltage(config.undervoltageThreshold) ||
        !validateCurrent(config.overcurrentThreshold) ||
        !validateTemperature(config.temperatureLimit)) {
        return false;
    }

    if (!enterConfigMode()) return false;
    
    uint8_t configData[12];
    memcpy(&configData[0], &config.overcurrentThreshold, 2);
    memcpy(&configData[2], &config.overvoltageThreshold, 2);
    memcpy(&configData[4], &config.undervoltageThreshold, 2);
    memcpy(&configData[6], &config.temperatureLimit, 2);
    memcpy(&configData[8], &config.designCapacity, 2);
    memcpy(&configData[10], &config.chemistry, 2);
    
    bool success = writeDataFlash(0x0010, configData, sizeof(configData));
    
    exitConfigMode();
    return success;
}

bool BMSLib::getConfiguration(BMSConfig &config) {
    if (!enterConfigMode()) return false;
    
    uint8_t configData[12];
    bool success = readDataFlash(0x0010, configData, sizeof(configData));
    
    if (success) {
        memcpy(&config.overcurrentThreshold, &configData[0], 2);
        memcpy(&config.overvoltageThreshold, &configData[2], 2);
        memcpy(&config.undervoltageThreshold, &configData[4], 2);
        memcpy(&config.temperatureLimit, &configData[6], 2);
        memcpy(&config.designCapacity, &configData[8], 2);
        memcpy(&config.chemistry, &configData[10], 2);
    }
    
    exitConfigMode();
    return success;
}

bool BMSLib::getBatteryStatus(BatteryStatus &status) {
    status.voltage = readVoltage_inVolts();
    status.current = readCurrent_inAmps();
    status.temperature = readTemperature_inCelsius();
    status.soc = readSoC();
    status.soh = readSoH();
    status.safetyStatus = readSafetyStatus();
    status.cycleCount = readCycleCount();
    status.remainingCapacity = readRemainingCapacity_inAmpHours();
    status.fullChargeCapacity = readFullChargeCapacity_inAmpHours();
    
    status.isCharging = (status.current > 0);
    status.isDischarging = (status.current < 0);
    status.hasError = (status.safetyStatus != 0);
    status.lastError = _lastError;
    
    // Validate critical parameters
    if (status.voltage == 0 || status.temperature < -40 || status.temperature > 85) {
        _lastError = BMSError::PARAMETER_ERROR;
        return false;
    }
    
    return true;
}

// Factory reset
bool BMSLib::factoryReset() {
    if (_configMode) {
        exitConfigMode();
    }
    
    if (!writeWord(BMS_REG_CONTROL, BMS_FACTORY_RESET)) {
        _lastError = BMSError::COMMUNICATION_ERROR;
        return false;
    }
    
    delay(500); // Wait for reset to complete
    return isOnline();
}

// Basic status and info
void BMSLib::getVersion(uint8_t &major, uint8_t &minor, uint8_t &patch) {
    major = BMSLIB_VERSION_MAJOR;
    minor = BMSLIB_VERSION_MINOR;
    patch = BMSLIB_VERSION_PATCH;
}

bool BMSLib::isOnline() {
    uint16_t controlValue;
    return readWord(BMS_REG_CONTROL, controlValue);
}

BMSError BMSLib::getLastError() const {
    return _lastError;
}

// Data flash operations
bool BMSLib::writeDataFlash(uint16_t address, uint8_t *data, uint8_t length) {
    if (!takeMutex()) return false;

    bool success = true;
    success &= writeBlockData(0x61, 0x00);
    success &= writeBlockData(0x3E, (address >> 8) & 0xFF);
    success &= writeBlockData(0x3F, address & 0xFF);

    for (uint8_t i = 0; i < length && success; i++) {
        success &= writeBlockData(0x40 + i, data[i]);
    }

    if (success) {
        uint8_t checksum = computeChecksum();
        success &= writeBlockData(0x60, 0xFF - checksum);
    }

    giveMutex();
    return success;
}

bool BMSLib::readDataFlash(uint16_t address, uint8_t *data, uint8_t length) {
    if (!takeMutex()) return false;

    bool success = true;
    success &= writeBlockData(0x61, 0x00);
    success &= writeBlockData(0x3E, (address >> 8) & 0xFF);
    success &= writeBlockData(0x3F, address & 0xFF);

    for (uint8_t i = 0; i < length && success; i++) {
        success &= readBlockData(0x40 + i, data[i]);
    }

    giveMutex();
    return success;
}

// Validation functions
bool BMSLib::validateValue(uint16_t value, uint16_t min, uint16_t max) {
    if (value < min || value > max) {
        _lastError = BMSError::PARAMETER_ERROR;
        return false;
    }
    return true;
}

bool BMSLib::validateTemperature(uint16_t temp) {
    return validateValue(temp, 2731, MAX_TEMPERATURE); // 0°C to 70°C in 0.1K
}

bool BMSLib::validateVoltage(uint16_t voltage) {
    return validateValue(voltage, MIN_VOLTAGE, MAX_VOLTAGE);
}

bool BMSLib::validateCurrent(int16_t current) {
    return (current >= -MAX_CURRENT && current <= MAX_CURRENT);
}

bool BMSLib::validateChargeVoltage(uint16_t voltage) {
    return validateValue(voltage, MIN_VOLTAGE, MAX_CHARGE_VOLTAGE);
}

bool BMSLib::validateChargeCurrent(uint16_t current) {
    return validateValue(current, 0, MAX_CHARGE_CURRENT);
}

// Remaining helper functions
uint8_t BMSLib::computeChecksum() {
    uint8_t checksum = 0;
    uint8_t data;
    
    for (uint8_t i = 0; i < 32; i++) {
        if (readBlockData(0x40 + i, data)) {
            checksum += data;
        } else {
            _lastError = BMSError::COMMUNICATION_ERROR;
            return 0;
        }
    }
    return checksum;
}

float BMSLib::compensateTemperature(float voltage, float temperature) {
    float tempDiff = temperature - 25.0f;  // Difference from room temperature
    return voltage * (1.0f + (tempDiff * TEMP_COEFFICIENT / 1000000.0f));
}

bool BMSLib::setAlarmConfig(const BMSAlarmConfig &config) {
    if (!enterConfigMode()) return false;

    uint16_t enableMask = 0;
    if (config.enableOverVoltage) enableMask |= BMS_ALARM_OV;
    if (config.enableUnderVoltage) enableMask |= BMS_ALARM_UV;
    if (config.enableOverCurrent) enableMask |= BMS_ALARM_OC;
    if (config.enableOverTemperature) enableMask |= BMS_ALARM_OT;
    if (config.enableUnderTemperature) enableMask |= BMS_ALARM_UT;
    if (config.enableLowSoC) enableMask |= BMS_ALARM_SOC_LOW;
    if (config.enableDischarging) enableMask |= BMS_ALARM_DISCHG;
    if (config.enableCharging) enableMask |= BMS_ALARM_CHG;

    bool success = true;
    success &= writeWord(BMS_REG_ALARM_ENABLE, enableMask);
    success &= writeWord(BMS_REG_ALARM_SOC_LOW, config.socLowThreshold);
    success &= writeWord(BMS_REG_ALARM_TEMP_HIGH, config.tempHighThreshold);
    success &= writeWord(BMS_REG_ALARM_VOLT_LOW, config.voltLowThreshold);
    success &= writeWord(BMS_REG_ALARM_VOLT_HIGH, config.voltHighThreshold);
    success &= writeWord(BMS_REG_ALARM_CURRENT, config.currentThreshold);

    exitConfigMode();
    return success;
}

bool BMSLib::getAlarmConfig(BMSAlarmConfig &config) {
    if (!enterConfigMode()) return false;

    uint16_t enableMask;
    bool success = readWord(BMS_REG_ALARM_ENABLE, enableMask);
    if (success) {
        config.enableOverVoltage = (enableMask & BMS_ALARM_OV) != 0;
        config.enableUnderVoltage = (enableMask & BMS_ALARM_UV) != 0;
        config.enableOverCurrent = (enableMask & BMS_ALARM_OC) != 0;
        config.enableOverTemperature = (enableMask & BMS_ALARM_OT) != 0;
        config.enableUnderTemperature = (enableMask & BMS_ALARM_UT) != 0;
        config.enableLowSoC = (enableMask & BMS_ALARM_SOC_LOW) != 0;
        config.enableDischarging = (enableMask & BMS_ALARM_DISCHG) != 0;
        config.enableCharging = (enableMask & BMS_ALARM_CHG) != 0;

        uint16_t value;
        if (readWord(BMS_REG_ALARM_SOC_LOW, value)) config.socLowThreshold = value;
        if (readWord(BMS_REG_ALARM_TEMP_HIGH, value)) config.tempHighThreshold = value;
        if (readWord(BMS_REG_ALARM_VOLT_LOW, value)) config.voltLowThreshold = value;
        if (readWord(BMS_REG_ALARM_VOLT_HIGH, value)) config.voltHighThreshold = value;
        if (readWord(BMS_REG_ALARM_CURRENT, value)) config.currentThreshold = value;
    }

    exitConfigMode();
    return success;
}

uint16_t BMSLib::getAlarmStatus() {
    uint16_t status;
    if (!readWord(BMS_REG_ALARM_STATUS, status)) {
        return 0;
    }
    return status;
}

bool BMSLib::clearAlarms() {
    return writeWord(BMS_REG_ALARM_STATUS, 0x0000);
}

// Individual alarm status checks
bool BMSLib::isOverVoltageAlarm() {
    return (getAlarmStatus() & BMS_ALARM_OV) != 0;
}

bool BMSLib::isUnderVoltageAlarm() {
    return (getAlarmStatus() & BMS_ALARM_UV) != 0;
}

bool BMSLib::isOverCurrentAlarm() {
    return (getAlarmStatus() & BMS_ALARM_OC) != 0;
}

bool BMSLib::isOverTemperatureAlarm() {
    return (getAlarmStatus() & BMS_ALARM_OT) != 0;
}

bool BMSLib::isUnderTemperatureAlarm() {
    return (getAlarmStatus() & BMS_ALARM_UT) != 0;
}

bool BMSLib::isLowSoCAlarm() {
    return (getAlarmStatus() & BMS_ALARM_SOC_LOW) != 0;
}

bool BMSLib::isDischargingAlarm() {
    return (getAlarmStatus() & BMS_ALARM_DISCHG) != 0;
}

bool BMSLib::isChargingAlarm() {
    return (getAlarmStatus() & BMS_ALARM_CHG) != 0;
}

// Individual alarm threshold setters
bool BMSLib::setLowSoCAlarm(uint8_t threshold) {
    if (threshold > 100) {
        _lastError = BMSError::PARAMETER_ERROR;
        return false;
    }
    return writeWord(BMS_REG_ALARM_SOC_LOW, threshold);
}

bool BMSLib::setHighTemperatureAlarm(uint16_t threshold) {
    if (!validateTemperature(threshold)) {
        return false;
    }
    return writeWord(BMS_REG_ALARM_TEMP_HIGH, threshold);
}

bool BMSLib::setLowTemperatureAlarm(uint16_t threshold) {
    if (!validateTemperature(threshold)) {
        return false;
    }
    return writeWord(BMS_REG_ALARM_TEMP_HIGH, threshold);
}

bool BMSLib::setLowVoltageAlarm(uint16_t threshold) {
    if (!validateVoltage(threshold)) {
        return false;
    }
    return writeWord(BMS_REG_ALARM_VOLT_LOW, threshold);
}

bool BMSLib::setHighVoltageAlarm(uint16_t threshold) {
    if (!validateVoltage(threshold)) {
        return false;
    }
    return writeWord(BMS_REG_ALARM_VOLT_HIGH, threshold);
}

bool BMSLib::setCurrentAlarm(uint16_t threshold) {
    if (!validateCurrent(threshold)) {
        return false;
    }
    return writeWord(BMS_REG_ALARM_CURRENT, threshold);
}