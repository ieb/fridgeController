#pragma once

#include <Arduino.h>
#include "config.h"




class CommandLine {
    public:
        CommandLine(Stream * io, ConfigSettings * settings, 
                void (*statusCallback)(void), 
                void (*rpmCallback)(int16_t rpm),
                void (*dutyCycleCallback)(int16_t dutyCycle)) : 
            io{io},
            settings{settings},
            statusCallback{statusCallback},
            rpmCallback{rpmCallback},
            dutyCycleCallback{dutyCycleCallback}
             {};
        bool diagnosticsEnabled = false;
        void begin();
        void showHelp();
        bool checkCommand();
    private:
        Stream * io;
        ConfigSettings *settings;

        void (*statusCallback)(void);
        void (*rpmCallback)(int16_t rpm);
        void (*dutyCycleCallback)(int16_t dutyCycle);

        void toggleDiagnostics();
        void loadSettings();
        void printHeader(uint8_t id, char end=':' );
        void printStatus_P(uint8_t id, const char * const lookup[], uint8_t idx );
        void printStatus(uint8_t id, int16_t value);
        void printStatus(uint8_t id, uint16_t value);
        void printStatus(uint8_t id, float value);
        void printAllStatus();
        void doReset();
        void doSetup();
        void doCalibration();
        void doManualControl();
        void showConfig();
        void showStatus();
        bool readLong(long *l);
        void read(uint8_t *v, uint8_t min=0, uint8_t max=255);
        void read(int8_t *v, int8_t min=-127, int8_t max=127);
        void read(int16_t *v, int16_t min=0, int16_t max=0xffff);
        void read(float *v, float min=-1e8, float max=1e8);
};


