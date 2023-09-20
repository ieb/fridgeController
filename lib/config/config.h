#pragma once

#include <Arduino.h>
#include <Preferences.h>

/*
 * At over about 40 bytes its not possible to cast a sting to byte[] and read or write
 * it correcty, unless the struct is packed by the compiler. Failing to do this
 * will cause the layout to be all over the place and the byte[] wont be the same lenght
 * as sizeof(Config), hence the __attibute__ packed is required. Filing to do this
 * results on the crcs being incorrect.
 */
typedef struct __attribute__ ((packed)) Config {
    float turnOnTemperature[3]  = {  3, 7, 10};
    float turnOffTemperature[3] = { -1, 4,  8};
    float minimumChargingVoltage = 13200.0;
    float minimumBatteryVoltage = 12000.0;
    float targetEvaporatorTemperature = 1.0;
    float highEvaporatorTemperature = 8.0;
    float minEvaporatorTemperature = 0.0;
    float minDischargeTemperature  = 0.0;
    int16_t compressorRPM[3] = { 3500, 2000, 0};
    int16_t dutyOff = 0;
    int16_t duty2000 = 10;
    int16_t duty3500 = 1024;
    int16_t compressorStartDelay = 240;
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
        bool diagnosticsEnabled = false;
        Config config;
        Preferences preferences;
};