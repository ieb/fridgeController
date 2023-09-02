#pragma once

#include <Arduino.h>

typedef struct Config {
    float turnOnTemperature[3]  = {  3, 7, 10};
    float turnOffTemperature[3] = { -1, 4,  8};
    int16_t compressorRPM[3] = { 3500, 2000, 0};
    float minimumChargingVoltage = 13200.0;
    float minimumBatteryVoltage = 12000.0;
    int16_t dutyOff = 0;
    int16_t duty2000 = 10;
    int16_t duty3500 = 1024;
    uint16_t crc;
} _Config;



class ConfigSettings {
    public:
        ConfigSettings();
        bool begin();
        bool load();
        bool save();
        void factoryReset();
        uint16_t modbus_crc16(const uint8_t *array, uint16_t length);
        Config config;
};