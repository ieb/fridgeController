#if defined(ESP32)
#include "secoppower.h"

// ESP32s have ledc PWM which allows setting the frequency, so will use that.

void SecopPower::begin() {
    ledcSetup(1, 5000, 10);
    pinMode(pin, OUTPUT);
    ledcAttachPin(pin, 1);
    ledcWrite(1, config->dutyOff);
}
int16_t SecopPower::getCompressorRPM() {
  return compressorRPM;
}

void SecopPower::setDutyCycle(int16_t duty) {
    ledcWrite(1, duty);
    if ( duty == 0 ) {
      compressorRPM = 0;
    }
    compressorRPM = map(duty, config->duty2000, config->duty3500, 2000, 3500);
}


void SecopPower::setCompressorSpeed(int16_t rpm) {
 if ( rpm <  2000 ) {
    // effectively off
    compressorRPM = 0;
    ledcWrite(1, config->dutyOff);
  } else if (rpm > 3500 ) {
    compressorRPM = 3500;
    ledcWrite(1, config->duty3500);
  } else {
    // assuming the relationship is linear
    ledcWrite(1, map(rpm, 2000, 3500, config->duty2000, config->duty3500));
    compressorRPM = rpm;
  }
}
#endif