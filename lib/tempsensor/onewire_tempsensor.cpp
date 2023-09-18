
#if defined(ONEWIRE_SENSOR_PIN)

#include "tempsensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

void printDeviceAddress(DeviceAddress *deviceAddress) {
    uint8_t *b = (uint8_t *)deviceAddress; 
    for (int i = 0; i < 8; i++) {
        if ( b[i] < 16) {
            Serial.print(" 0x0");
        } else {
            Serial.print(" 0x");
        }
        Serial.print(b[i],HEX);
    }
}


void TemperatureSensors::begin() {
    ds18B20.begin();    // initialize the DS18B20 sensor
    for(int i = 0; i < 3; i++) {
        Serial.print("Device ");
        Serial.print(i);
        Serial.print(" getAddress:");
        Serial.print(ds18B20.getAddress(sensors[i],i));
        Serial.print(" Address:");
        printDeviceAddress(&sensors[i]);
        Serial.println("");

    }
    ds18B20.setWaitForConversion(false);
    ds18B20.requestTemperatures();

    lastRequest = millis();
}


void TemperatureSensors::getTemperatures() {
    Serial.print("getTemperatures: ");
    /*
    for (int i = 0; i < 3; i++) {

        Serial.print(ds18B20.getTempC(sensors[i]));
        Serial.print(" ");
    }
    Serial.print(" tr:");            
    Serial.print(millis()-now);
    now = millis();
    */

    lastRequest = millis();
    conversionCompleteAt = lastRequest;
    ds18B20.requestTemperatures();
    Serial.print(" tp:");            
    Serial.println(millis()-lastRequest);
}

bool TemperatureSensors::checkStatus() {
    if ( ds18B20.isConversionComplete() ) {
        if ( conversionCompleteAt == lastRequest ) {
            conversionCompleteAt = millis();
            Serial.print("Temp ready ms:");
            Serial.println(conversionCompleteAt-lastRequest);
        }
        return true;
    }
    return false;
}


                    
float TemperatureSensors::readCelciusFromSensor(uint8_t channel) {
    float t = ds18B20.getTempC(sensors[channel]);
    Serial.print("c:");
    Serial.print(channel);
    Serial.print(" ms:");
    Serial.print(millis()-lastRequest);
    Serial.print(" t:");
    Serial.println(t);
    return t;  // read temperature in °C

}

#endif