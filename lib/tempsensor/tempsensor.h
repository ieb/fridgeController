#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>



class TemperatureSensors {
public:
    TemperatureSensors() : oneWire{ONEWIRE_SENSOR_PIN}, ds18B20{&oneWire} {

    }
    void begin();
    void getTemperatures();
    float readCelciusFromSensor(uint8_t channel);
    bool checkStatus();
    bool diagnosticsEnabled = false;
private:
    OneWire oneWire;
    DallasTemperature ds18B20;
    unsigned long lastRequest = 0;
    unsigned long conversionCompleteAt = 0;
    DeviceAddress sensors[3];
};