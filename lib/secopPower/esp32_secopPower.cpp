#if defined(ESP32)
#include "secoppower.h"

// ESP32s have ledc PWM which allows setting the frequency, so will use that.

void SecopPower::begin() {
    // ledcAttachPin changed  so needed change and remove ledcSetup  JEM 2024-06
    // through out pin is  changed to PWM_PIN as that is what is used in ledcAttach and ledcWrite JEM 2024-06
    // class variable pin is not used Could assign to PWM_PIN and other refernces to pin JEM 2024-06
    ledcAttach(PWM_PIN, 5000, 10);
    //pinMode(pin, OUTPUT); Conflictes with ledcAttachPin, so removed JEM 2024-06
    //ledcAttachPin(pin, 1); Not needed as ledcAttach does this JEM 2024-06
    ledcWrite(PWM_PIN, config->dutyOff);}
int16_t SecopPower::getCompressorRPM() {
  return compressorRPM;
}

void SecopPower::setDutyCycle(int16_t duty) {
    ledcWrite(PWM_PIN, duty);
    if ( duty == 0 ) {
      compressorRPM = 0;
    }
    compressorRPM = map(duty, config->duty2000, config->duty3500, 2000, 3500);
}


void SecopPower::setCompressorSpeed(int16_t rpm) {
 if ( rpm <  2000 ) {
    // effectively off
    compressorRPM = 0;
    ledcWrite(PWM_PIN, config->dutyOff);
  } else if (rpm > 3500 ) {
    compressorRPM = 3500;
    ledcWrite(PWM_PIN, config->duty3500);
  } else {
    // assuming the relationship is linear
    ledcWrite(PWM_PIN, map(rpm, 2000, 3500, config->duty2000, config->duty3500));
    compressorRPM = rpm;
  }
}
#endif