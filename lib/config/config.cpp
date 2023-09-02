#include <config.h>
#include <EEPROM.h>


ConfigSettings::ConfigSettings() {
    factoryReset();
}

bool ConfigSettings::begin() {
    return EEPROM.begin(sizeof(Config));
}
bool ConfigSettings::load() {
    Config epromConfig;
    EEPROM.readBytes(0, &epromConfig, sizeof(Config));
    uint16_t crcv =  modbus_crc16((uint8_t *) &(epromConfig), sizeof(Config)-2);
    if ( crcv == epromConfig.crc ) {
        memcpy(&config, &epromConfig , sizeof(Config));
        Serial.print("CRC Check read:");
        Serial.print(epromConfig.crc);
        Serial.print(" checked:");
        Serial.print(crcv);
        Serial.print(" using:");
        Serial.println(config.crc);
        return true;
    }
    Serial.print("CRC Check read:");
    Serial.print(epromConfig.crc);
    Serial.print(" checked:");
    Serial.println(crcv);
    return false;
}
bool ConfigSettings::save() {
    config.crc =  modbus_crc16((uint8_t *)&config, sizeof(Config)-2);
    EEPROM.writeBytes(0, &config, sizeof(Config));
    EEPROM.commit();

    Config epromConfig;
    EEPROM.readBytes(0, &epromConfig, sizeof(Config));
    uint16_t crcv =  modbus_crc16((uint8_t *) &(epromConfig), sizeof(Config)-2);
    Serial.print("CRC Check wrote:");
    Serial.print(config.crc);
    Serial.print(" read:");
    Serial.print(epromConfig.crc);
    Serial.print(" checked:");
    Serial.println(crcv);
    return (crcv == config.crc);

}

void ConfigSettings::factoryReset() {
    config.turnOnTemperature[0] = 3.0f;
    config.turnOnTemperature[1] = 7.0f;
    config.turnOnTemperature[2] = 10.0f;
    config.turnOffTemperature[0] = -1.0f;
    config.turnOffTemperature[1] = 4.0f;
    config.turnOffTemperature[2] = 8.0f;
    config.compressorRPM[0] = 3500;
    config.compressorRPM[1] = 2000;
    config.compressorRPM[2] = 0;
    config.minimumChargingVoltage = 13200.0;
    config.minimumBatteryVoltage = 12000.0;
    config.dutyOff = 0;
    config.duty2000 = 10;
    config.duty3500 = 1024;
};




/**
 * @brief crc for mdbus, polynomial = 0x8005, reverse in, reverse out, init 0xffff;
 * 
 * @param array 
 * @param length 
 * @return uint16_t 
 */
uint16_t ConfigSettings::modbus_crc16(const uint8_t *array, uint16_t length) {
    uint16_t crc = 0xffff;
    while (length--) {
        if ((length & 0xFF) == 0) yield();  // RTOS
        uint8_t data = *array++;
        data = (((data & 0xAA) >> 1) | ((data & 0x55) << 1));
        data = (((data & 0xCC) >> 2) | ((data & 0x33) << 2));
        data =          ((data >> 4) | (data << 4));
        crc ^= ((uint16_t)data) << 8;
        for (uint8_t i = 8; i; i--) {
        if (crc & (1 << 15)) {
            crc <<= 1;
            crc ^= 0x8005;
        } else {
            crc <<= 1;
        }
        }
    }
    crc = (((crc & 0XAAAA) >> 1) | ((crc & 0X5555) << 1));
    crc = (((crc & 0xCCCC) >> 2) | ((crc & 0X3333) << 2));
    crc = (((crc & 0xF0F0) >> 4) | ((crc & 0X0F0F) << 4));
    //  crc = (( crc >> 8) | (crc << 8));
    //  crc ^= endmask;
    return crc;
};
