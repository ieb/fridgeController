#include <config.h>
#include <Preferences.h>

void dumpBytes(Config *configBlock) {
    uint8_t *b = (uint8_t *) &configBlock;
    for (int i = 0; i < sizeof(Config); i++) {
        if ( b[i] < 16) {
            Serial.print("0");
        }
        Serial.print(b[i], HEX);
    }
    Serial.println();
} 
void dumpText(Config *config) {
    Serial.print("config->turnOnTemperature[0] "); Serial.println(config->turnOnTemperature[0]);
    Serial.print("config->turnOnTemperature[1] "); Serial.println(config->turnOnTemperature[1]);
    Serial.print("config->turnOnTemperature[2] "); Serial.println(config->turnOnTemperature[2]);
    Serial.print("config->turnOffTemperature[0] "); Serial.println(config->turnOffTemperature[0]);
    Serial.print("config->turnOffTemperature[1] "); Serial.println(config->turnOffTemperature[1]);
    Serial.print("config->turnOffTemperature[2] "); Serial.println(config->turnOffTemperature[2]);
    Serial.print("config->compressorRPM[0] "); Serial.println(config->compressorRPM[0]);
    Serial.print("config->compressorRPM[1] "); Serial.println(config->compressorRPM[1]);
    Serial.print("config->compressorRPM[2] "); Serial.println(config->compressorRPM[2]);
    Serial.print("config->minimumChargingVoltage "); Serial.println(config->minimumChargingVoltage);
    Serial.print("config->minimumBatteryVoltage "); Serial.println(config->minimumBatteryVoltage);
    Serial.print("config->dutyOff "); Serial.println(config->dutyOff);
    Serial.print("config->duty2000 "); Serial.println(config->duty2000);
    Serial.print("config->duty3500 "); Serial.println(config->duty3500);
    Serial.print("config->compressorStartDelay "); Serial.println(config->compressorStartDelay);
    Serial.print("config->targetEvaporatorTemperature "); Serial.println(config->targetEvaporatorTemperature);
    Serial.print("config->highEvaporatorTemperature "); Serial.println(config->highEvaporatorTemperature);
    Serial.print("config->minEvaporatorTemperature "); Serial.println(config->minEvaporatorTemperature);
    Serial.print("config->minDischargeTemperature "); Serial.println(config->minDischargeTemperature);
}

void zero(Config *config) {
    config->turnOnTemperature[0] = 0.0f;
    config->turnOnTemperature[1] = 0.0f;
    config->turnOnTemperature[2] = 0.0f;
    config->turnOffTemperature[0] = 0.0f;
    config->turnOffTemperature[1] = 0.0f;
    config->turnOffTemperature[2] = 0.0f;
    config->compressorRPM[0] = 0;
    config->compressorRPM[1] = 0;
    config->compressorRPM[2] = 0;
    config->minimumChargingVoltage = 0.0;
    config->minimumBatteryVoltage = 0.0;
    config->dutyOff = 0;
    config->duty2000 = 0;
    config->duty3500 = 0;
    config->compressorStartDelay = 0;
    config->targetEvaporatorTemperature = 0;
    config->highEvaporatorTemperature = 0;
    config->minEvaporatorTemperature = 0.0;
    config->minDischargeTemperature  = 0.0;
}


ConfigSettings::ConfigSettings() {
    factoryReset();
}

bool ConfigSettings::begin() {
    return true; 
}
bool ConfigSettings::load() {
    preferences.begin("fridge", true);
    size_t configLen = preferences.getBytesLength("config");
    if ( configLen == sizeof(Config) ) {
        Config prefsConfig;

        preferences.getBytes("config", &prefsConfig, configLen);
        uint16_t crcv =  modbus_crc16((uint8_t *) &(prefsConfig), sizeof(Config)-2);
        if ( crcv == prefsConfig.crc ) {
            memcpy(&config, &prefsConfig , sizeof(Config));
            Serial.print("CRC Check read:");
            Serial.print(prefsConfig.crc);
            Serial.print(" checked:");
            Serial.print(crcv);
            Serial.print(" using:");
            Serial.println(config.crc);
            preferences.end();
            return true;
        }
        Serial.print("CRC Check read:");
        Serial.print(prefsConfig.crc);
        Serial.print(" checked:");
        Serial.println(crcv);
        Serial.println("Stored preferences crc mismatch");
    } else {
        Serial.println("Stored preferences size mismatch");
    }
    preferences.end();
    return false;
}
bool ConfigSettings::save() {
    preferences.begin("fridge", false);

    uint8_t epromSize = sizeof(Config);
    if ( diagnosticsEnabled ) {
        Serial.print("Eprom Size:");Serial.println(epromSize);
        Serial.print("Before Write:");
        dumpText(&config);
        dumpBytes(&config);        
    }
    config.crc =  modbus_crc16((uint8_t *)&config, epromSize-2);
    preferences.putBytes("config", (uint8_t *)&config, epromSize);
    if ( diagnosticsEnabled ) {
        Serial.print("After Write :");
        dumpBytes(&config);
    }


    Config epromConfig;
    if ( diagnosticsEnabled ) {
        zero(&epromConfig);
        dumpText(&epromConfig);
        Serial.print("Test (0) :");
        Serial.println(epromConfig.compressorStartDelay);
        Serial.print("Before Read :");
        dumpBytes(&epromConfig);
    }
    preferences.getBytes("config", (uint8_t *)&epromConfig, epromSize);
    if ( diagnosticsEnabled ) {
    Serial.print("After Read  :");
        dumpBytes(&epromConfig);
        dumpText(&epromConfig);

    }
    uint16_t crcv =  modbus_crc16((uint8_t *) &(epromConfig), epromSize-2);
    if ( diagnosticsEnabled ) {
        Serial.print("CRC Check wrote:");
        Serial.print(config.crc);
        Serial.print(" read:");
        Serial.print(epromConfig.crc);
        Serial.print(" checked:");
        Serial.println(crcv);
        
    }
    preferences.end();
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
    config.compressorStartDelay = 240;
    config.targetEvaporatorTemperature = 1.0;
    config.highEvaporatorTemperature = 8.0;
    config.minEvaporatorTemperature = 0.0;
    config.minDischargeTemperature  = 0.0;

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
