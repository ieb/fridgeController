
#if defined(ONEWIRE_SENSOR_PIN)

#include <OneWire.h>
#include <DallasTemperature.h>


float readCelciusFromSensor(uint8_t channel) {
    static bool tempSensor_init = false;
    static OneWire oneWire(ONEWIRE_SENSOR_PIN);
    static DallasTemperature DS18B20(&oneWire);
    if ( !tempSensor_init ) {
        DS18B20.begin();    // initialize the DS18B20 sensor
        tempSensor_init = true;
    }
    DS18B20.requestTemperatures();       // send the command to get temperatures
    return DS18B20.getTempCByIndex(channel);  // read temperature in °C
}

#endif