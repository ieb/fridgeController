#include <Arduino.h>
#include "secoppower.h"
#include "tempsensor.h"
#include "battery.h"
#include "config.h"
#include "commandline.h"
#include "ssd1306.h"

ConfigSettings settings;
extern void showStatus(void);
extern void setRPM(int16_t rpm);
extern void setDutyCycle(int16_t dutyCycle);
CommandLine commandLine(&Serial, &settings, &showStatus, &setRPM, &setDutyCycle);
TemperatureSensors temperatureSensors;

SecopPower secopPower(&Serial, PWM_PIN, &(settings.config));

LCDDisplay lcdDisplay(&settings);



#define POWER_STATE_CHARGING 0
#define POWER_STATE_ON_BATTERY 1
#define POWER_STATE_LOW_BATTERY 2
const char * stateDisplay[3] = {
    "oncharge",
    "discharging",
    "low"
};


uint8_t state = POWER_STATE_ON_BATTERY;
bool compressorOn = false;
bool diagnosticsEnabled = true;
bool manualControl = false;
unsigned long compressorTurnedOnAt = 0;
unsigned long compressorTurnedOffAt = 0;

/**
 * Verifying to myself that the correct way to test for a time
 * difference in millis is (now - last ) > period
 * last is always before now, so that calculation will always
 * result in a positive difference when the unsigned long wraps
 * Other forms dont work
 *     now > (last + period) 
 * will fail when last approaches max long, since last + period wraps
 * now is always greater than the result.
 * Leaving here to remind myself.
 */
void testMillisDifference() {
    unsigned long t1 =  10;
    unsigned long t2 =  t1+9990;
    for (int i = 0; i < 100; i++) {
        t2++;
        Serial.printf("Low %lu %lu %lu  %s \n", t1, t2, (t2-t1), ((t2-t1)>10000UL)?"after":"before");
    }

    t1 =  4294967255;
    t2 =  t1+9990;
    for (int i = 0; i < 100; i++) {
        t2++;
        Serial.printf("High %lu %lu %lu %s \n", t1, t2, (t2-t1), ((t2-t1)>10000UL)?"after":"before");
    }

}

void setup() {
    Serial.begin(115200);
    // wait a second so that its possible to flash if the 
    // program is in a crashloop
    delay(1000);
    Serial.println("Secop Compressor Controller v1.0");
    temperatureSensors.begin();
    commandLine.begin();
    secopPower.begin();
    lcdDisplay.begin();
    Serial.println("press h for help");


}
void setDutyCycle(int16_t dutyCycle) {
    if ( dutyCycle == -1 ) {
        if ( manualControl ) {
            Serial.println("Switching to auto");
            manualControl = false;
            secopPower.setDutyCycle(0);
        }
    } else {
        if ( !manualControl ) {
            manualControl = true;
            Serial.println("Switching to manual duty cycle");
        }
        secopPower.setDutyCycle(dutyCycle);
        Serial.print("rpm set to ");Serial.println(secopPower.getCompressorRPM());
    }
}

void setRPM(int16_t rpm) {
    if ( rpm == -1 ) {
        if ( manualControl ) {
            Serial.println("Switching to auto");
            manualControl = false;
            secopPower.setCompressorSpeed(0);
        }
    } else {
        if ( !manualControl ) {
            manualControl = true;
            Serial.println("Switching to manual rpm");
        }
        secopPower.setCompressorSpeed(rpm);
        Serial.print("rpm set to ");Serial.println(secopPower.getCompressorRPM());
    }
}

void showStatus() {
    Serial.print("State                    : "); Serial.println(stateDisplay[state]);
    Serial.print("Control                  : "); Serial.println(manualControl?"manual":"auto");
    Serial.print("Voltage              (mV): "); Serial.println(readBatteryVoltageMv(BAT_ADC_PIN));
    Serial.print("Temperature Evaporator(C): "); Serial.println(temperatureSensors.readCelciusFromSensor(EVAPORATOR_SENSOR));
    Serial.print("Temperature Discharge (C): "); Serial.println(temperatureSensors.readCelciusFromSensor(DISCHARGE_SENSOR));
    Serial.print("Temperature Box       (C): "); Serial.println(temperatureSensors.readCelciusFromSensor(BOX_SENSOR));
    Serial.print("Compressor          (rpm): "); Serial.println(secopPower.getCompressorRPM());
    Serial.print("Temp High             (C): "); Serial.println(settings.config.turnOnTemperature[state]);
    Serial.print("Temp Low              (C): "); Serial.println(settings.config.turnOffTemperature[state]);
    Serial.print("Min Charging         (mV): "); Serial.println(settings.config.minimumChargingVoltage);
    Serial.print("Min Battery          (mV): "); Serial.println(settings.config.minimumBatteryVoltage);
}

/*
 * Aim is to return a  RPM based on the measured temperatures.
 * sensors are BOX_SENSOR, EVAPORATOR_SENSOR and DISCHARGE_SENSOR.
 * The fridge is a fridge not a freezer, so the simplest form of control
 * is to reduce the RPM of the fridge when the evaporator sensor says the 
 * evaporator is down to temperature.
 * 
 * ALso to stop cycling delay the turn on of the motor for 120s, and 
 * delay turning back on for 240s after a turn off.

    -D EVAPORATOR_SENSOR=0 
   -D DISCHARGE_SENSOR=1 
   -D BOX_SENSOR=2 

 * 
 */

int16_t calculateRPM(float *temperatures) {
    unsigned long now = millis();
    if ( compressorOn 
        && ((now - compressorTurnedOnAt) > settings.config.compressorStartDelay*1000)
        && ((now - compressorTurnedOffAt) > settings.config.compressorStartDelay*2000)  ) {
        if (temperatures[EVAPORATOR_SENSOR] <= settings.config.minEvaporatorTemperature
            || temperatures[DISCHARGE_SENSOR] <= settings.config.minDischargeTemperature ) {
            // not a freezer
            return 0;
        } else if (temperatures[EVAPORATOR_SENSOR] <= settings.config.targetEvaporatorTemperature ) {
            return 2000;
        } else {
            return map(temperatures[EVAPORATOR_SENSOR], settings.config.targetEvaporatorTemperature, 
                settings.config.highEvaporatorTemperature, 2000, 3500); 
        }
    }
    return 0;
}

void checkStatus() {
    static unsigned long last = millis();
    static unsigned long readSent = false;
    unsigned long now = millis();
    if ( !readSent && (now - last) > 7000UL ) {
        readSent = true;
        temperatureSensors.getTemperatures();
    } 
    if ( readSent ) {
        temperatureSensors.checkStatus();
    }
    if ( (now - last) > 10000UL ) {
        if ( !temperatureSensors.checkStatus() ) {
            Serial.println("Onewire too slow");
        }
        last = now;
        readSent = false;
        float  voltage = readBatteryVoltageMv(BAT_ADC_PIN);
        // determine any state change.
        switch(state) {
            case POWER_STATE_CHARGING:
                // can go into on battery.
                if ( voltage < settings.config.minimumChargingVoltage ) {
                    state = POWER_STATE_ON_BATTERY;
                    Serial.print("Below minimum charging voltage  bv:");
                    Serial.print(voltage);
                    Serial.print(" <  min:");
                    Serial.println(settings.config.minimumChargingVoltage);
                }
            break;
            case POWER_STATE_ON_BATTERY:
                // can go into low power or charging.
                if ( voltage > settings.config.minimumChargingVoltage ) {
                    state = POWER_STATE_CHARGING;
                    Serial.print("Above minimum charging voltage  bv:");
                    Serial.print(voltage);
                    Serial.print(" >  min:");
                    Serial.println(settings.config.minimumChargingVoltage);
                } else if ( voltage < settings.config.minimumBatteryVoltage ) {
                    state = POWER_STATE_LOW_BATTERY;
                    Serial.print("Below minimum battery voltage  bv:");
                    Serial.print(voltage);
                    Serial.print(" <  min:");
                    Serial.println(settings.config.minimumBatteryVoltage);
                }
            break;
            case POWER_STATE_LOW_BATTERY:
                // in low battery mode, only way out is to to be charged.
                // the battery voltage will recover, but the fridge should 
                // reduce its drain once this happens.
                if ( voltage > settings.config.minimumChargingVoltage ) {
                    state = POWER_STATE_CHARGING;
                    Serial.print("Above minimum charging voltage  bv:");
                    Serial.print(voltage);
                    Serial.print(" >  min:");
                    Serial.println(settings.config.minimumChargingVoltage);
                }
            break;
        }
        float temperatures[3];
        for (int i = 0; i < 3; i++) {
            temperatures[i] = temperatureSensors.readCelciusFromSensor(i);

        }
        int16_t rpm = 0;
        if ( state == POWER_STATE_LOW_BATTERY ) {
                secopPower.setCompressorSpeed(0);
                compressorOn = false;
        } else {
            if ( temperatures[BOX_SENSOR] > settings.config.turnOnTemperature[state] ) {
                if ( !compressorOn ) {
                    compressorTurnedOnAt = millis();
                    compressorOn = true;
                }
                rpm = calculateRPM(temperatures);
                secopPower.setCompressorSpeed(rpm);
            } else if ( temperatures[BOX_SENSOR] < settings.config.turnOffTemperature[state]) {
                if ( compressorOn ) {
                    compressorTurnedOffAt = millis();
                    compressorOn = false;
                }
                secopPower.setCompressorSpeed(0);
            }        
        }
        lcdDisplay.update(voltage,  temperatures, rpm, state);

    }
}

void flashLed() {
    static unsigned long last = millis();
    static unsigned long cycle = 0;
    unsigned long now = millis();
    if ( (now-last) > last ) {
        last = now;
        int8_t ledOn = (cycle%2);
        switch(state) {
            case POWER_STATE_CHARGING:
                // 3 flashes
                if ( cycle > 5 ) {
                    ledOn = 0;
                }
                break;
            case POWER_STATE_ON_BATTERY:
                // 2 flashes
                if ( cycle > 3 ) {
                    ledOn = 0;
                }
                break;
            case POWER_STATE_LOW_BATTERY:
                // flash 1x
                if (cycle > 1 ) {
                    ledOn = 0;
                }
                break;

        }
        digitalWrite(LED_PIN, (ledOn==1)?HIGH:LOW);
        cycle++;
        if ( cycle > 10 ) {
            cycle = 0;
        }
    }

}

void loop() {
    checkStatus();
    flashLed();
    lcdDisplay.update();
    diagnosticsEnabled = commandLine.checkCommand();
    temperatureSensors.diagnosticsEnabled = diagnosticsEnabled;
    settings.diagnosticsEnabled = diagnosticsEnabled;
}


