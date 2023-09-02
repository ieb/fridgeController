#include <Arduino.h>
#include "secoppower.h"
#include "tempsensor.h"
#include "battery.h"
#include "config.h"
#include "commandline.h"

ConfigSettings settings;
extern void showStatus(void);
extern void setRPM(int16_t rpm);
extern void setDutyCycle(int16_t dutyCycle);
CommandLine commandLine(&Serial, &settings, &showStatus, &setRPM, &setDutyCycle);


SecopPower secopPower(&Serial, PWM_PIN, &(settings.config));

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



void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println("Secop Compressor Controller v1.0");
    commandLine.begin();
    secopPower.begin();
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
    Serial.print("State            : "); Serial.println(stateDisplay[state]);
    Serial.print("Control          : "); Serial.println(manualControl?"manual":"auto");
    float  voltage = readBatteryVoltageMv(BAT_ADC_PIN);
    Serial.print("Voltage      (mV): "); Serial.println(voltage);
    float temperature = readCelciusFromSensor(0);
    Serial.print("Temperature   (C): "); Serial.println(temperature);
    int16_t rpm = secopPower.getCompressorRPM();
    Serial.print("Compressor  (rpm): "); Serial.println(rpm);
    Serial.print("Temp High     (C): "); Serial.println(settings.config.turnOnTemperature[state]);
    Serial.print("Temp Low      (C): "); Serial.println(settings.config.turnOffTemperature[state]);
    Serial.print("Min Charging (mV): "); Serial.println(settings.config.minimumChargingVoltage);
    Serial.print("Min Battery  (mV): "); Serial.println(settings.config.minimumBatteryVoltage);
}


void checkStatus() {
    static unsigned long last = millis();
    unsigned long now = millis();
    if ( now > last+10000UL ) {
        last = now;
        if (now > last+10000UL) {
            // overflow, reset required.
            last = 0;
        }
        if ( manualControl ) {
            return;
        }
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
        float temperature = readCelciusFromSensor(0);
        if ( state == POWER_STATE_LOW_BATTERY ) {
                secopPower.setCompressorSpeed(0);
                compressorOn = false;
        } else {
            if ( temperature > settings.config.turnOnTemperature[state] ) {
                compressorOn = true;
                secopPower.setCompressorSpeed(settings.config.compressorRPM[state]);
            } else if ( temperature < settings.config.turnOffTemperature[state]) {
                compressorOn = false;
                secopPower.setCompressorSpeed(0);
            }        
        }
    }
}

void flashLed() {
    static unsigned long last = millis();
    static unsigned long cycle = 0;
    unsigned long now = millis();
    if ( now > last+200UL ) {
        last = now;
        if (now > last+200UL) {
            // overflow, reset required.
            last = 0;
        }
        int8_t ledOn = (cycle+1)%2;
        switch(state) {
            case POWER_STATE_CHARGING:
                // 3 flashes
                if ( cycle > 5 ) {
                    ledOn = 0;
                }
            case POWER_STATE_ON_BATTERY:
                // 2 flashes
                if ( cycle > 3 ) {
                    ledOn = 0;
                }
            case POWER_STATE_LOW_BATTERY:
                // flash 1x
                if (cycle > 1 ) {
                    ledOn = 0;
                }

        }
        digitalWrite(LED_PIN, ledOn);
        cycle++;
        if ( cycle > 10 ) {
            cycle = 0;
        }
    }

}

void loop() {
    checkStatus();
    flashLed();
    diagnosticsEnabled = commandLine.checkCommand();
}


