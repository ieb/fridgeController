#if defined(__AVR_ATmega328P__)
#error "Not in use, here for reference"

#include "secoppower.h"


#define DUTY_2000 0
#define DUTY_3500 49
#define DUTY_OFF 0

/**
 * Setup PWM to 5KHz.
 * ATmega328p
 */ 
void SecopPower::begin() {
  pinMode(11, OUTPUT);
  // https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
  // page 127
  // COM == Compare (match) output mode
  // WGM == Waveform Generation mode
  // 
  // fast mode with OCRA top Mode 7, WGM = 111, set WRM20,21,22
  // Channel A Pin 3 not used, so disconnect, COM2A0 and COM2A1 not set.
  // Channel B Connect pin 11 non inverting, COM2B1 set, COM2B0 not set.
  // Frequencies
  // Channel A = 16000/(64*(49+1))/2 == 2.5KHz, 50% duty cycle.
  // Precalar == 65 = CS22 set, CS21 not set, CSS20 not set.
  // Output B = 16000/(64*(49+1)) = 5Khz
  // Duty cycle = (22+1) / (49+1) = 46%
  // or ORC2B = (dutyCycle * (49+1))-1;
  // 
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22) ;
  OCR2A = 49;
  OCR2B = DUTY_OFF;
}

/**
 * 49 == 100%
 * Atmega328p
 */ 
void SecopPower::setPower(uint8_t powerLevel) {
  if ( rpm <  2000 ) {
    // effectively off
    TOCR2B = DUTY_OFF;
  } else if (rpm > 3500 ) {
    OCR2B = DUTY_3500;
  } else {
    // assuming the relationship is li
    OCR2B = map(duty, 2000, 3500, DUTY_2000, DUTY_3500);
  }
}

#endif