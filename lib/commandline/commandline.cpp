#include "commandline.h"


const char menuOptions_0[] = "Charging - Temp On";
const char menuOptions_1[] = "Charging - Temp Off";
const char menuOptions_2[] = "Charging - Compressor RPM";
const char menuOptions_3[] = "Normal   - Temp On";
const char menuOptions_4[] = "Normal   - Temp Off";
const char menuOptions_5[] = "Normal   - Compressor RPM";
const char menuOptions_6[] = "Low Bat  - Temp On";
const char menuOptions_7[] = "Low Bat  - Temp Off";
const char menuOptions_8[] = "Low Bat  - Compressor RPM";
const char menuOptions_9[] = "Charging min V";
const char menuOptions_10[] = "Normal min V";
const char menuOptions_11[] = "Duty Cycle - Off";
const char menuOptions_12[] = "Duty Cycle - 2000";
const char menuOptions_13[] = "Duty Cycle - 3500";
const char menuOptions_14[] = "Save settings and restart";
const char menuOptions_15[] = "Quit and restart";

const char * const menuOptions[] = {
    menuOptions_0,
    menuOptions_1,
    menuOptions_2,
    menuOptions_3,
    menuOptions_4,
    menuOptions_5,
    menuOptions_6,
    menuOptions_7,
    menuOptions_8,
    menuOptions_9,
    menuOptions_10,
    menuOptions_11,
    menuOptions_12,
    menuOptions_13,
    menuOptions_14,
    menuOptions_15
};
const char menuKey[] = "0123456789abcdSQ";


void CommandLine::begin() {
    if ( !settings->begin() ) {
        Serial.println("Failed to initialize EEPROM");
        delay(1000);
        ESP.restart();
    } else {
        Serial.println("EEPROM initialized");
    }
    loadSettings();
};

void CommandLine::showHelp() {
    io->println(F("Secop Fridge Controller - key presses"));
    io->println(F("  - 'h' or '?' to show this message"));
    io->println(F("  - 's' show status"));
    io->println(F("  - 'c' show config"));
    io->println(F("  - 'm' manual control"));
    io->println(F("  - 'C' calibrate compressor"));
    io->println(F("  - 'd' toggle diagnostics"));
    io->println(F("  - 'S' setup"));
    io->println(F("  - 'R' reboot"));
    io->println(F("  - 'F' factory reset"));
};

bool CommandLine::checkCommand() {
    if (io->available()) {
        int chr = io->read();
        switch ( chr ) {
            case 'h': showHelp(); return true;
            case 's': showStatus(); return true;
            case 'c': showConfig(); return true;
            case 'C': doCalibration(); return true;
            case 'm': doManualControl(); return true;
            case 'S': doSetup(); return true;
            case 'R': doReset(); return true;
            case 'F': 
                settings->factoryReset(); 
                settings->save();
                io->println("Reset to Factory Settings");
                showConfig();
                return true;
            case 'd': toggleDiagnostics(); return true;
        }
    }
    return diagnosticsEnabled;
};


void CommandLine::doReset() {
   ESP.restart();
}

void CommandLine::toggleDiagnostics() {
    diagnosticsEnabled = !diagnosticsEnabled;
};



void CommandLine::loadSettings() {
    if ( settings->load() ) {
        io->println(F("Eprom Valid"));
    } else {
        io->println(F("Eprom Not Valid, saving current settings"));
        settings->save();
    }
    showConfig();

};


void CommandLine::printHeader(uint8_t id, char end ) {
    io->print(menuKey[id]);
    io->print(". ");
    io->print(menuOptions[id]);
    if ( end != 0) {
        for (int8_t i = strlen(menuOptions[id]); i< 35;i++) {
            io->print(' ');
        }
        io->print(end);
        io->print(' ');
    }
}

void CommandLine::printStatus_P(uint8_t id, const char * const lookup[], uint8_t idx ) {
    printHeader(id);
    io->println(lookup[idx]);
}

void CommandLine::printStatus(uint8_t id, int16_t value) {
    printHeader(id);
    io->println(value);
}
void CommandLine::printStatus(uint8_t id, uint16_t value) {
    printHeader(id);
    io->println(value);
}
void CommandLine::printStatus(uint8_t id, float value) {
    printHeader(id);
    io->println(value);
}




void CommandLine::printAllStatus() {
    printStatus(0,settings->config.turnOnTemperature[0]);
    printStatus(1,settings->config.turnOffTemperature[0]);
    printStatus(2,settings->config.compressorRPM[0]);
    printStatus(3,settings->config.turnOnTemperature[1]);
    printStatus(4,settings->config.turnOffTemperature[1]);
    printStatus(5,settings->config.compressorRPM[1]);
    printStatus(6,settings->config.turnOnTemperature[2]);
    printStatus(7,settings->config.turnOffTemperature[2]);
    printStatus(8,settings->config.compressorRPM[2]);
    printStatus(9,settings->config.minimumChargingVoltage);
    printStatus(10,settings->config.minimumBatteryVoltage);
    printStatus(11,settings->config.dutyOff);
    printStatus(12,settings->config.duty2000);
    printStatus(13,settings->config.duty3500);
}

void CommandLine::doCalibration() {
    int16_t dutyCycle = 0;
    bool calibrate = true;
    io->println(F("Calibrate u=up, d=down, x=exit"));
    Serial.print("DC:");Serial.print(dutyCycle);
    dutyCycleCallback(dutyCycle);
    while(calibrate) {
        if (io->available()) {
            int chr = io->read();
            switch ( chr ) {
                case 'u': dutyCycle++; break;
                case 'd': dutyCycle--; break;
                case 'x': calibrate = false; break;
            }
            if ( dutyCycle > 1024 ) {
                dutyCycle = 1024;
            } else if ( dutyCycle < 0) {
                dutyCycle = 0;
            }
            Serial.print("DC:");Serial.print(dutyCycle);
            dutyCycleCallback(dutyCycle);
        }
    }
    dutyCycleCallback(-1);
}

void CommandLine::doManualControl() {
    io->print("New Rpm:");
    io->setTimeout(30000);
    int16_t rpm = -2;
    read(&(rpm), -1, 3500); 
    io->println("");
    io->print("Setting rpm :");
    io->println(rpm);

    rpmCallback(rpm);
    io->setTimeout(0);
}

void CommandLine::doSetup() {
    showConfig();
    io->setTimeout(30000);
    bool performingSetup = true;
    while(performingSetup) {
        while(io->available()) {
            io->read();
        }
        io->println(F("Setup Menu"));
        printAllStatus();

        printHeader(14,0);io->println("");
        printHeader(15,0);io->println("");

        int option = io->read();


        while (option == -1 ) {
            delay(10);
            option = io->read();
        }
        uint8_t idx = 255;
        for ( int i = 0; i < strlen(menuKey); i++) {
            if ( option == menuKey[i]) {
                idx = i;
            }
        }
        if ( idx == 255 ) {
            Serial.print(F(" not recognised "));
            Serial.println(option,DEC);
            continue;
        }
        printHeader(idx,0);io->print(" ");
        io->print(F(" >"));

        switch(idx) {
            case 0: read(&(settings->config.turnOnTemperature[0])); break;
            case 1: read(&(settings->config.turnOffTemperature[0])); break;
            case 2: read(&(settings->config.compressorRPM[0]), 0, 3500); break;
            case 3: read(&(settings->config.turnOnTemperature[1])); break;
            case 4: read(&(settings->config.turnOffTemperature[1])); break;
            case 5: read(&(settings->config.compressorRPM[1]), 0, 3500); break;
            case 6: read(&(settings->config.turnOnTemperature[2])); break;
            case 7: read(&(settings->config.turnOffTemperature[2])); break;
            case 8: read(&(settings->config.compressorRPM[2]), 0, 3500); break;
            case 9: read(&(settings->config.minimumChargingVoltage)); break;
            case 10: read(&(settings->config.minimumBatteryVoltage)); break;
            case 11: read(&(settings->config.dutyOff),0,1024); break;
            case 12: read(&(settings->config.duty2000),0,1024); break;
            case 13: read(&(settings->config.duty3500),0,1024); break;
            case 14:
                if ( settings->save() ) {
                    io->println("Settings saved - rebooting");
                    doReset();
                } else {
                    io->println("Settings save - failed");
                }

                break;
            case 15:
                io->println("Exit setup");
                performingSetup = false;
                break;
        }
    }
    io->setTimeout(0);
};

void CommandLine::showConfig() {
    io->println(F("Config"));
    printAllStatus();
}



void CommandLine::showStatus() {
    statusCallback();
};




bool CommandLine::readLong(long *l) {
    String line = io->readStringUntil('\r');
    line.trim();
    if ( line.length() > 0 ) {
        *l = line.toInt();
        return true;
    }
    return false;
}

void CommandLine::read(uint8_t *v, uint8_t min, uint8_t max) {
    long l;
    if ( readLong(&l) ) {
        if ( l >= min && l <= max ) {
            *v = l;
        }
    }
}
void CommandLine::read(int8_t *v, int8_t min, int8_t max) {
    long l;
    if ( readLong(&l) ) {
        if ( l >= min && l <= max ) {
            *v = l;
        }
    }
}
void CommandLine::read(int16_t *v, int16_t min, int16_t max) {
    long l;
    if ( readLong(&l) ) {
        if ( l >= min && l <= max ) {
            *v = l;
        }
    }
}
void CommandLine::read(float *v, float min, float max) {
    String line = io->readStringUntil('\r');
    line.trim();
    if ( line.length() > 0 ) {
        double d = line.toDouble();
        if ( d >= min && d <= max ) {
            *v = d;
        }
    }
}
