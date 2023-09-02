#include "battery.h"

float readBatteryVoltageMv(uint8_t pin) {
#if defined(MEGATINYCORE)
    // Nominally the suppy voltage 5V which is applied to the 
    // top R. Using VDD as analog reference avoids having to adjust.
    // for a supply voltage offset. We are not interested in the absolute
    // voltage only the ADC reading realive to the supply voltage.
    analogReference(INTERNAL2V048); // 0.5mV per LSB
    delayMicroseconds(100); // wait at least 60us for the reference change to act
    analogSampleDuration(300);

    // need  A divider using 10+4.7+2.2 measured across the 2.2K
    // Multiple is 0.5*(10+2.2+4.7)/2.2 mv  15*2.2/(12.2+4.7)
#define VOLTAGE_SCALE  3.8409090909f // 0.5*(10+2.2+4.7)/2.2
    int32_t reading = analogReadEnh(pin, 12, 1);
    return VOLTAGE_SCALE*reading;
#elif defined(ESP32)
    // ESP32s are notoriously bad for ADC linearity
    // but are ok between about 300mV and 2.5v 
    // so as long as the divider is setup for that range, it should be good enough.
    // 10+1+2.2 measured over 2.2
    // 15*2.2/(12.2+1) = 2.5v
    // 3.3v == 4096 == 0.8056640625 mv/LSB
#define VOLTAGE_SCALE  4.834f // 0.8056640625*(10+2.2+1)/2.2
    return VOLTAGE_SCALE*analogRead(pin);
#else 
    // assuming 3.3v, at 1024 == 3.24mv/lsb
    // divider 10+2.2 measured across 2.2L
#define VOLTAGE_SCALE  17.96f // 3.24*(10+2.2)/2.2
    return VOLTAGE_SCALE*analogRead(pin);
#endif
}
