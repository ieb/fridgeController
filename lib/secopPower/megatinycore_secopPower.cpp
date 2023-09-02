#if defined(MEGATINYCORE)
#error "Not in use, here for reference"

#include "secoppower.h"


// TODO map WO1 to PA5 ?????

#define DUTY_2000 0
#define DUTY_3500 3200
#define DUTY_OFF 0


#if defined(MILLIS_USE_TIMERA0) || defined(__AVR_ATtinyxy2__)
  #error "This sketch takes over TCA0, don't use for millis here.  Pin mappings on 8-pin parts are different"
#endif


/**
 * Setup PWM to 5KHz.
 * 3v3 16MHz clock
 */ 
void SecopPower::begin() {
  // https://github.com/SpenceKonde/megaTinyCore/blob/c7afbb3161086edb54112005df15e4a1db84bf16/megaavr/extras/TakingOverTCA0.md

  pinMode(pin, OUTPUT); //PB0 - TCA0 WO1, pin7 on 14-pin parts
  byte port = digitalPinToPort(pin);
  PORTMUX.TCAROUTEA = port; // map output to the pin.
  takeOverTCA0();
  //Dual slope PWM mode OVF interrupt at BOTTOM, PWM on WO1
  // Need PWM on WO3, WO4 or WO5 to get PA3, PA4 or PA5
  // WO0, WO1, WO2 are only on PB0, PB1, PB2 or alt PB3, PB4, PB5.
  // PB0 == SCL, PB1 == SDA, PB2==Tx0, PB3==RX0
  // Cant move I2C, Would rather not move Tx0, Rx0
  // To get to W03,4,5 we would have to use split mode and only output one side of the split.
  // see page 229 of https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny3224-3226-3227-Data-Sheet-DS40002345A.pdf
  // for description.
  // Simplest is to use RX1 and TX1
  // Conclusion use WO2 on PB2
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CM25EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc); 
  TCA0.SINGLE.PER = 3200; // 16000/5 = 3200 == 5Khz, assuming a 16MHz clock chip.
  TCA0.SINGLE.CMP1 = 0; // start with a duty cycle of 0
  TCA0.SINGLE.INTCTRL = 0; // no interrupts required
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // enable the timer with no prescaler
}

void SecopPower::setCompressorSpeed(int16_t rpm) {
  if ( rpm <  2000 ) {
    // effectively off
    TCA0.SINGLE.CMP1 = DUTY_OFF;
  } else if (rpm > 3500 ) {
    TCA0.SINGLE.CMP1 = DUTY_3500;
  } else {
    // assuming the relationship is li
    TCA0.SINGLE.CMP1 = map(duty, 2000, 3500, DUTY_2000, DUTY_3500);
  }
}

#endif