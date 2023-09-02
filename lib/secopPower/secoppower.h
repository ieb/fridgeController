#include <Arduino.h>
#include "config.h"
class SecopPower {
public:
    SecopPower(Stream * io, uint8_t pin, Config *config) : io{io}, config{config}  {
        this->pin = pin;
    };
    /*
     * setup PWM output
     */
    void begin();
    /*
     * set compressor Speed in RPM
     */
    void setCompressorSpeed(int16_t rpm);
    void setDutyCycle(int16_t duty);
    int16_t getCompressorRPM();
private:
    Stream * io;
    uint8_t pin;
    Config * config;
    int16_t compressorRPM = 0;
};