#include "BMSLib.h"

BMSLib::BMSLib(TwoWire &wirePort) : 
    _wire(&wirePort),
    _configMode(false) {
}

BMSLib::~BMSLib() {
}

bool BMSLib::begin() {
    _wire->begin();
    delay(100);  // Allow I2C to stabilize
    return isOnline();
}

void BMSLib::getVersion(uint8_t &major, uint8_t &minor, uint8_t &patch) {
    major = BMSLIB_VERSION_MAJOR;
    minor = BMSLIB_VERSION_MINOR;
    patch = BMSLIB_VERSION_PATCH;
}

bool BMSLib::isOnline() {
    uint16_t controlValue;
    return readWord(BMS_REG_CNTL, controlValue);
}

bool BMSLib::readWord(uint8_t command, uint16_t &value) {
    _wire->beginTransmission(BMS_I2C_ADDRESS);
    _wire->write(command);
    if (_wire->endTransmission(false) != 0) {
        return false;
    }
    
    if (_wire->requestFrom(BMS_I2C_ADDRESS, (uint8_t)2) != 2) {
        return false;
    }
    
    uint8_t lowByte = _wire->read();
    uint8_t highByte = _wire->read();
    value = (highByte << 8) | lowByte;
    
    return true;
}

bool BMSLib::writeWord(uint8_t command, uint16_t data) {
    _wire->beginTransmission(BMS_I2C_ADDRESS);
    _wire->write(command);
    _wire->write(data & 0xFF);        // Low byte
    _wire->write((data >> 8) & 0xFF); // High byte
    return (_wire->endTransmission() == 0);
}

uint16_t BMSLib::readVoltage() {
    uint16_t value;
    if (!readWord(BMS_REG_VOLT, value) || !validateVoltage(value)) {
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
    if (!readWord(BMS_REG_TEMP, value) || !validateTemperature(value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::readSoC() {
    uint16_t value;
    if (!readWord(BMS_REG_SOC, value)) {
        return 0;
    }
    return value > 100 ? 100 : value;
}

uint16_t BMSLib::readSoH() {
    uint16_t value;
    if (!readWord(BMS_REG_SOH, value)) {
        return 0;
    }
    return value > 100 ? 100 : value;
}

uint16_t BMSLib::readCycleCount() {
    uint16_t value;
    if (!readWord(BMS_REG_CC, value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::readDesignCapacity() {
    uint16_t value;
    if (!readWord(BMS_REG_DCAP, value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::readFullChargeCapacity() {
    uint16_t value;
    if (!readWord(BMS_REG_FCC, value)) {
        return 0;
    }
    return value;
}

uint16_t BMSLib::readRemainingCapacity() {
    uint16_t value;
    if (!readWord(BMS_REG_RM, value)) {
        return 0;
    }
    return value;
}

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

float BMSLib::readCapacity_inAmpHours() {
    return readCapacity() / 1000.0f;
}

float BMSLib::readFullChargeCapacity_inAmpHours() {
    return readFullChargeCapacity() / 1000.0f;
}

float BMSLib::readRemainingCapacity_inAmpHours() {
    return readRemainingCapacity() / 1000.0f;
}

bool BMSLib::setDesignCapacity(uint16_t capacity_mAh) {
    if (!enterConfigMode()) {
        return false;
    }

    // Write to Design Capacity register
    bool success = writeWord(BMS_REG_DCAP, capacity_mAh);

    exitConfigMode();
    return success;
}

bool BMSLib::setFullChargeCapacity(uint16_t capacity_mAh) {
    if (!enterConfigMode()) {
        return false;
    }

    // Write to Full Charge Capacity register
    bool success = writeWord(BMS_REG_FCC, capacity_mAh);

    exitConfigMode();
    return success;
}

bool BMSLib::setCapacityConfig(const CapacityConfig& config) {
    if (!enterConfigMode()) {
        return false;
    }

    bool success = true;
    
    // Write each configuration value
    success &= writeWord(BMS_REG_DCAP, config.designCapacity);
    
    // Design Energy Data Flash Class = 48, offset = 13
    if (!writeWord(BMS_REG_DFCLS, 48) || !writeWord(BMS_REG_DFBLK, 0)) {
        exitConfigMode();
        return false;
    }
    
    uint8_t buffer[8];
    buffer[0] = config.designEnergy & 0xFF;
    buffer[1] = (config.designEnergy >> 8) & 0xFF;
    buffer[2] = config.cycleCountThresh & 0xFF;
    buffer[3] = (config.cycleCountThresh >> 8) & 0xFF;
    buffer[4] = config.chargeTermination;
    buffer[5] = config.reserveCapacity;
    
    success &= writeDataFlash(13, buffer, sizeof(buffer));

    exitConfigMode();
    return success;
}

bool BMSLib::getCapacityConfig(CapacityConfig& config) {
    if (!enterConfigMode()) {
        return false;
    }

    bool success = true;
    
    // Read Design Capacity
    uint16_t value;
    success &= readWord(BMS_REG_DCAP, value);
    config.designCapacity = value;
    
    // Read other parameters from Data Flash
    if (!writeWord(BMS_REG_DFCLS, 48) || !writeWord(BMS_REG_DFBLK, 0)) {
        exitConfigMode();
        return false;
    }
    
    uint8_t buffer[8];
    success &= readDataFlash(13, buffer, sizeof(buffer));
    
    if (success) {
        config.designEnergy = (buffer[1] << 8) | buffer[0];
        config.cycleCountThresh = (buffer[3] << 8) | buffer[2];
        config.chargeTermination = buffer[4];
        config.reserveCapacity = buffer[5];
    }

    exitConfigMode();
    return success;
}

bool BMSLib::calibrateVoltage(const VoltageCalibration& cal) {
    if (!enterConfigMode()) {
        return false;
    }

    // Validate input values
    if (!validateVoltage(cal.actualVoltage) || !validateVoltage(cal.measuredVoltage)) {
        exitConfigMode();
        return false;
    }

    // Calculate calibration coefficient
    float coefficient = static_cast<float>(cal.actualVoltage) / cal.measuredVoltage;
    uint16_t gainValue = static_cast<uint16_t>(coefficient * 1000); // Store with 3 decimal precision

    // Write calibration data
    bool success = writeWord(BMS_REG_VOLTAGE_CAL, gainValue);
    
    exitConfigMode();
    return success;
}

bool BMSLib::calibrateCurrent(const CurrentCalibration& cal) {
    if (!enterConfigMode()) {
        return false;
    }

    // Validate input values
    if (!validateCurrent(cal.actualCurrent) || 
        !validateCurrent(cal.measuredCurrent) || 
        cal.shuntResistance == 0) {
        exitConfigMode();
        return false;
    }

    // Calculate calibration coefficient
    float coefficient = static_cast<float>(cal.actualCurrent) / cal.measuredCurrent;
    uint16_t gainValue = static_cast<uint16_t>(coefficient * 1000);

    // Write calibration data
    bool success = true;
    success &= writeWord(BMS_REG_CURRENT_CAL, gainValue);
    success &= writeWord(BMS_REG_SHUNT_RESISTANCE, cal.shuntResistance);

    exitConfigMode();
    return success;
}

bool BMSLib::calibrateTemperature(const TempCalibration& cal) {
    if (!enterConfigMode()) {
        return false;
    }

    // Validate input values
    if (!validateTemperature(cal.actualTemp) || !validateTemperature(cal.measuredTemp)) {
        exitConfigMode();
        return false;
    }

    // Calculate calibration coefficient
    float coefficient = static_cast<float>(cal.actualTemp) / cal.measuredTemp;
    uint16_t gainValue = static_cast<uint16_t>(coefficient * 1000);

    // Write calibration data
    bool success = writeWord(BMS_REG_TEMP_CAL, gainValue);

    exitConfigMode();
    return success;
}

bool BMSLib::performFullCalibration(const VoltageCalibration& vcal,
                                  const CurrentCalibration& ccal,
                                  const TempCalibration& tcal) {
    if (!enterConfigMode()) {
        return false;
    }

    bool success = true;
    
    // Perform all calibrations
    success &= calibrateVoltage(vcal);
    success &= calibrateCurrent(ccal);
    success &= calibrateTemperature(tcal);

    // If all calibrations successful, mark calibration status
    if (success) {
        uint16_t calStatus = 0xAA55; // Calibration complete marker
        success &= writeWord(BMS_REG_CAL_STATUS, calStatus);
    }

    exitConfigMode();
    return success;
}

bool BMSLib::isCalibrated() {
    uint16_t calStatus;
    if (!readWord(BMS_REG_CAL_STATUS, calStatus)) {
        return false;
    }
    return calStatus == 0xAA55;
}

bool BMSLib::clearCalibration() {
    if (!enterConfigMode()) {
        return false;
    }

    // Reset all calibration values to defaults
    bool success = true;
    success &= writeWord(BMS_REG_VOLTAGE_CAL, 1000);  // 1.000 gain
    success &= writeWord(BMS_REG_CURRENT_CAL, 1000);  // 1.000 gain
    success &= writeWord(BMS_REG_TEMP_CAL, 1000);     // 1.000 gain
    success &= writeWord(BMS_REG_CAL_STATUS, 0x0000); // Clear calibration status

    exitConfigMode();
    return success;
}

bool BMSLib::setBatteryChemistry(BatteryChemistry chemistry) {
    if (!isChemistrySupported(chemistry)) {
        return false;
    }

    if (!enterConfigMode()) {
        return false;
    }

    bool success = writeWord(BMS_REG_CHEM, static_cast<uint16_t>(chemistry));
    
    // Allow time for chemistry change to take effect
    if (success) {
        delay(100);
    }

    exitConfigMode();
    return success;
}

BMSLib::BatteryChemistry BMSLib::getBatteryChemistry() {
    uint16_t value;
    if (!readWord(BMS_REG_CHEM, value)) {
        return BatteryChemistry::LION; // Default to Li-ion if read fails
    }
    return static_cast<BatteryChemistry>(value);
}

bool BMSLib::isChemistrySupported(BatteryChemistry chemistry) {
    uint16_t chemValue = static_cast<uint16_t>(chemistry);
    return (chemValue >= static_cast<uint16_t>(BatteryChemistry::LION) && 
            chemValue <= static_cast<uint16_t>(BatteryChemistry::PbAcid));
}

bool BMSLib::configureSelfDischarge(const SelfDischargeConfig& config) {
    if (!enterConfigMode()) {
        return false;
    }

    // Pack configuration into register format
    uint16_t configValue = 0;
    configValue = (config.rate & 0x3FF) |                    // 10 bits for rate
                 ((config.temperatureCoef & 0x1F) << 10) |   // 5 bits for temp coef
                 ((config.enabled ? 1 : 0) << 15);           // 1 bit for enable

    bool success = writeWord(BMS_REG_SELF_DISCH, configValue);

    exitConfigMode();
    return success;
}

bool BMSLib::getSelfDischargeConfig(SelfDischargeConfig& config) {
    uint16_t value;
    if (!readWord(BMS_REG_SELF_DISCH, value)) {
        return false;
    }

    // Unpack register value
    config.rate = value & 0x3FF;
    config.temperatureCoef = (value >> 10) & 0x1F;
    config.enabled = (value >> 15) & 0x01;

    return true;
}

float BMSLib::getEstimatedSelfDischarge() {
    SelfDischargeConfig config;
    if (!getSelfDischargeConfig(config) || !config.enabled) {
        return 0.0f;
    }

    // Get temperature for compensation
    float temp = readTemperature_inCelsius();
    float tempCoef = 1.0f + (config.temperatureCoef / 100.0f) * (temp - 25.0f);
    
    return (config.rate / 10.0f) * tempCoef;  // Return % per day
}

bool BMSLib::setPowerMode(PowerMode mode) {
    // Cannot enter config mode when setting shutdown
    if (mode != PowerMode::SHUTDOWN && !enterConfigMode()) {
        return false;
    }

    bool success = writeWord(BMS_REG_POWER_MODE, static_cast<uint16_t>(mode));
    
    if (success && mode == PowerMode::SHUTDOWN) {
        delay(500);  // Allow time for shutdown
    } else if (success) {
        delay(100);  // Allow time for mode change
        exitConfigMode();
    }

    return success;
}

BMSLib::PowerMode BMSLib::getPowerMode() {
    uint16_t value;
    if (!readWord(BMS_REG_POWER_MODE, value)) {
        return PowerMode::NORMAL;  // Default to normal if read fails
    }
    return static_cast<PowerMode>(value);
}

bool BMSLib::configurePowerSaving(const PowerConfig& config) {
    if (!enterConfigMode()) {
        return false;
    }

    bool success = true;
    success &= writeWord(BMS_REG_SLEEP_CUR, config.sleepCurrent);
    success &= writeWord(BMS_REG_SHUTDOWN_V, config.shutdownVoltage);

    // Pack wake voltage and sleep delay into one register
    uint16_t wakeConfig = (config.wakeVoltage & 0xFFF0) | (config.sleepDelay & 0x0F);
    success &= writeWord(BMS_REG_POWER_MODE + 1, wakeConfig);

    exitConfigMode();
    return success;
}

bool BMSLib::getPowerConfig(PowerConfig& config) {
    if (!enterConfigMode()) {
        return false;
    }

    bool success = true;
    success &= readWord(BMS_REG_SLEEP_CUR, config.sleepCurrent);
    success &= readWord(BMS_REG_SHUTDOWN_V, config.shutdownVoltage);

    // Unpack wake voltage and sleep delay
    uint16_t wakeConfig;
    success &= readWord(BMS_REG_POWER_MODE + 1, wakeConfig);
    if (success) {
        config.wakeVoltage = wakeConfig & 0xFFF0;
        config.sleepDelay = wakeConfig & 0x0F;
    }

    exitConfigMode();
    return success;
}

uint16_t BMSLib::getAveragePowerConsumption() {
    uint16_t voltage = readVoltage();
    int16_t current = readCurrent();
    
    // Return power in mW
    return static_cast<uint16_t>((static_cast<int32_t>(voltage) * 
                                 static_cast<int32_t>(abs(current))) / 1000);
}

bool BMSLib::getLastChargeTime(DateTime& dateTime) {
    if (!enterConfigMode()) {
        return false;
    }

    // Select State Data class
    if (!writeWord(BMS_REG_DFCLS, 82) ||     // State class = 82
        !writeWord(BMS_REG_DFBLK, 0)) {      // First block
        exitConfigMode();
        return false;
    }

    // Read just the date/time values
    uint8_t buffer[4];
    bool success = readDataFlash(14, buffer, sizeof(buffer));  // Offset 14 for date/time
    
    if (success) {
        // Get raw values
        uint16_t rawDate = (buffer[1] << 8) | buffer[0];
        uint16_t rawTime = (buffer[3] << 8) | buffer[2];
        
        // Convert date
        dateTime.year = (rawDate >> 9) + 1980;
        dateTime.month = (rawDate >> 5) & 0x0F;
        dateTime.day = rawDate & 0x1F;
        
        // Convert time
        dateTime.hour = rawTime / 60;
        dateTime.minute = rawTime % 60;
    }

    exitConfigMode();
    return success;
}

BMSLib::DateTime BMSLib::getLastChargeTime() {
    DateTime dt;
    if (!getLastChargeTime(dt)) {
        return DateTime();
    }
    return dt;
}

uint16_t BMSLib::getChargeCycles() {
    uint16_t value;
    if (!readWord(BMS_REG_CC, value)) {
        return 0;
    }
    return value;
}

bool BMSLib::getLifetimeStats(LifetimeStats& stats) {
    if (!enterConfigMode()) {
        return false;
    }

    // Select Lifetime Data class
    if (!writeWord(BMS_REG_DFCLS, 59) ||     // Lifetime Data class = 59
        !writeWord(BMS_REG_DFBLK, 0)) {      // First block
        exitConfigMode();
        return false;
    }

    // Read lifetime data block
    uint8_t buffer[32];
    bool success = readDataFlash(0, buffer, sizeof(buffer));
    
    if (success) {
        // Parse the data according to datasheet
        stats.maxTemp = (buffer[1] << 8) | buffer[0];
        stats.minTemp = (buffer[3] << 8) | buffer[2];
        stats.maxChargeCurrent = (buffer[5] << 8) | buffer[4];
        stats.maxDischargeCurrent = (buffer[7] << 8) | buffer[6];
        stats.maxPackVoltage = (buffer[9] << 8) | buffer[8];
        stats.minPackVoltage = (buffer[11] << 8) | buffer[10];
        stats.updateCount = (buffer[13] << 8) | buffer[12];
        
        // Get timestamp of last update
        uint16_t rawDate = (buffer[15] << 8) | buffer[14];
        uint16_t rawTime = (buffer[17] << 8) | buffer[16];
        
        // Convert date
        stats.lastUpdate.year = (rawDate >> 9) + 1980;
        stats.lastUpdate.month = (rawDate >> 5) & 0x0F;
        stats.lastUpdate.day = rawDate & 0x1F;
        
        // Convert time
        stats.lastUpdate.hour = rawTime / 60;
        stats.lastUpdate.minute = rawTime % 60;
    }

    exitConfigMode();
    return success;
}

bool BMSLib::resetLifetimeStats() {
    if (!enterConfigMode()) {
        return false;
    }

    // Select Lifetime Data class
    if (!writeWord(BMS_REG_DFCLS, 59) ||     // Lifetime Data class = 59
        !writeWord(BMS_REG_DFBLK, 0)) {      // First block
        exitConfigMode();
        return false;
    }

    // Prepare reset data
    uint8_t resetBuffer[32] = {0};  // All zeros
    
    // Set temperature to room temperature (25°C = 298.15K)
    resetBuffer[0] = 0x4A;  // 2981 & 0xFF
    resetBuffer[1] = 0x0B;  // 2981 >> 8
    resetBuffer[2] = 0x4A;  // Same for min temp
    resetBuffer[3] = 0x0B;

    bool success = writeDataFlash(0, resetBuffer, sizeof(resetBuffer));

    exitConfigMode();
    return success;
}

bool BMSLib::getDetailedStatus(DetailedStatus& status) {
    uint16_t flags, flagsB;
    
    // Read status flags
    if (!readWord(BMS_REG_FLAGS, flags) || 
        !readWord(BMS_REG_FLAGSB, flagsB)) {
        return false;
    }

    // Parse flags according to datasheet
    status.isCharging = (flags & 0x0001) != 0;
    status.isDischarging = (flags & 0x0002) != 0;
    status.isBalancing = (flags & 0x0004) != 0;
    status.isFull = (flags & 0x0008) != 0;
    status.isCalibrated = (flags & 0x0010) != 0;
	status.needsUpdate = (flags & 0x0020) != 0;
    status.sleepEnabled = (flags & 0x0040) != 0;
    status.shutdownRequested = (flags & 0x0080) != 0;

    // Get safety status
    status.safetyStatus = readSafetyStatus();
    
    // Get error code from control status
    uint16_t controlStatus;
    if (!readWord(BMS_REG_CNTL, controlStatus)) {
        return false;
    }
    status.errorCode = (controlStatus >> 8) & 0xFF;

    // Get additional data
    status.stateOfCharge = readSoC();
    status.stateOfHealth = readSoH();
    status.remainingCapacity = readRemainingCapacity();
    status.fullCapacity = readFullChargeCapacity();
    status.averageCurrent = readCurrent();
    status.temperature = readTemperature();

    return true;
}

bool BMSLib::sleep() {
    if (_configMode) {
        exitConfigMode();
    }
    return writeWord(BMS_REG_CNTL, BMS_SLEEP_COMMAND);
}

bool BMSLib::wake() {
    bool success = writeWord(BMS_REG_CNTL, BMS_WAKE_COMMAND);
    if (success) {
        delay(100);
    }
    return success;
}

bool BMSLib::resetWatchdog() {
    return writeWord(BMS_REG_CNTL, BMS_WATCHDOG_RESET);
}

bool BMSLib::enterConfigMode() {
    if (_configMode) return true;
    
    if (!writeWord(BMS_REG_CNTL, BMS_CONFIG_MODE_ENTER)) {
        return false;
    }
    
    delay(100);
    _configMode = true;
    return true;
}

bool BMSLib::exitConfigMode() {
    if (!_configMode) return true;
    
    if (!writeWord(BMS_REG_CNTL, BMS_CONFIG_MODE_EXIT)) {
        return false;
    }
    
    delay(100);
    _configMode = false;
    return true;
}

bool BMSLib::factoryReset() {
    if (_configMode) {
        exitConfigMode();
    }
    
    if (!writeWord(BMS_REG_CNTL, BMS_FACTORY_RESET)) {
        return false;
    }
    
    delay(500);
    return isOnline();
}

bool BMSLib::isOverVoltage() {
    uint16_t status = readSafetyStatus();
    return (status & 0x0001) != 0;
}

bool BMSLib::isUnderVoltage() {
    uint16_t status = readSafetyStatus();
    return (status & 0x0002) != 0;
}

bool BMSLib::isOverCurrent() {
    uint16_t status = readSafetyStatus();
    return (status & 0x0004) != 0;
}

bool BMSLib::isOverTemperature() {
    uint16_t status = readSafetyStatus();
    return (status & 0x0008) != 0;
}

bool BMSLib::readDataFlash(uint8_t offset, uint8_t* data, uint8_t length) {
    if (!_configMode) {
        return false;
    }

    for (uint8_t i = 0; i < length; i++) {
        _wire->beginTransmission(BMS_I2C_ADDRESS);
        _wire->write(0x40 + offset + i);  // Data flash starts at 0x40
        if (_wire->endTransmission(false) != 0) {
            return false;
        }
        
        if (_wire->requestFrom(BMS_I2C_ADDRESS, (uint8_t)1) != 1) {
            return false;
        }
        
        data[i] = _wire->read();
    }
    
    return true;
}

bool BMSLib::writeDataFlash(uint8_t offset, const uint8_t* data, uint8_t length) {
    if (!_configMode) {
        return false;
    }

    for (uint8_t i = 0; i < length; i++) {
        _wire->beginTransmission(BMS_I2C_ADDRESS);
        _wire->write(0x40 + offset + i);
        _wire->write(data[i]);
        if (_wire->endTransmission() != 0) {
            return false;
        }
    }
    
    return true;
}

bool BMSLib::isInSleepMode() {
    uint16_t status;
    if (!readWord(BMS_REG_CNTL, status)) {
        return false;
    }
    return (status & BMS_STATUS_SLEEP) != 0;
}

float BMSLib::compensateTemperature(float voltage, float temperature) {
    float tempDiff = temperature - 25.0f;  // Difference from room temperature
    return voltage * (1.0f + (tempDiff * TEMP_COEFFICIENT));
}

bool BMSLib::validateTemperature(uint16_t temp) {
    return (temp >= 2731 && temp <= MAX_TEMPERATURE);
}

bool BMSLib::validateVoltage(uint16_t voltage) {
    return (voltage >= MIN_VOLTAGE && voltage <= MAX_VOLTAGE);
}

bool BMSLib::validateCurrent(int16_t current) {
    return (current >= -MAX_CURRENT && current <= MAX_CURRENT);
}

uint16_t BMSLib::readSafetyStatus() {
    uint16_t value;
    if (!readWord(BMS_REG_FLAGS, value)) {
        return 0;
    }
    return value;
}