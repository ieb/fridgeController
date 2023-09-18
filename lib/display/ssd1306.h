#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

/**
 *  A SD1306 64x128 OLED display connected over I2C
 * Displays a number of simple pages of information.
 */ 

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define MAX_DISPLAYPAGES 10

typedef enum ButtonPress {
    NOPRESS,
    PRESS,
    HOLD1s,
    HOLD3s,
    HOLD5s,
    RELEASE,
    FORCE_UPDATE
} ButtonPress;



typedef enum DisplayState {
    AWAKE,
    START_SLEEP,
    WAIT_SLEEP_END,
    SLEEPING,
    START_WAKE
} DisplayState;



class DisplayPage {
    public:
        DisplayPage() {
            nextPage = this;
        };
        void setNextPage(DisplayPage *nextPage) {
            this->nextPage = nextPage;
        };
        void setHold5sPage(DisplayPage *hold5sPage) {
            this->hold5sPage = hold5sPage;
        };
        DisplayPage * processPress(ButtonPress bp, Adafruit_SSD1306 * display);
        virtual DisplayPage * update(ButtonPress press, Adafruit_SSD1306 * display);

    protected:
        unsigned long displayUpdate = 1000;
        unsigned long last = 0;
        int subPage = 0;
        int maxSubPages = 0;
        DisplayPage *nextPage;
        DisplayPage *hold5sPage;
};

class LogoDisplayPage: public DisplayPage {
    public:
        LogoDisplayPage(){
            displayUpdate = 60000;
        };
        DisplayPage * update(ButtonPress press, Adafruit_SSD1306 * display);
};


class StatsDisplayPage: public DisplayPage {
    public:
        StatsDisplayPage() {
            displayUpdate = 1000;
            maxSubPages = 3;
        };
        void setConfigSettings(ConfigSettings * settings) {
            this->settings = settings;
        }
        DisplayPage * update(ButtonPress press, Adafruit_SSD1306 * display);
        void update(float voltage, float *temperatures, int16_t rpm, int8_t state);
    private:
        ConfigSettings *settings;
        float voltage = 0.0;
        float temperatures[3] = { 0.0, 0.0, 0.0 };
        int16_t rpm = 0;
        int8_t state = 0;
};


class ConfigSettingsPage: public DisplayPage {
    public:
        ConfigSettingsPage() {
            displayUpdate = 1000;
        };
        void setConfigSettings(ConfigSettings * settings) {
            this->settings = settings;
        }
        DisplayPage * update(ButtonPress press, Adafruit_SSD1306 * display);
    private:
        ConfigSettings *settings;
};



class LCDDisplay  {
    public:
        LCDDisplay(ConfigSettings * settings)  {
            statsDisplayPage.setConfigSettings(settings);
            configSettingsPage.setConfigSettings(settings);
        };
        void begin();
        void update();
        void update(float voltage, float *temperatures, int16_t rpm, int8_t state);
        void nextPage();
        void startDim();
        void endDim();
        void sleep();
        void wake();
    private:
        bool drawDisplay();
        void dim();

        ButtonPress updateButtonPress();

        Adafruit_SSD1306 display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT);
        unsigned long lastDisplay = 0;
        unsigned long displayPeriod = 4000;
        unsigned long lastDim = 0;
        unsigned long dimPeriod = 1000;
        unsigned long staticPagePress = 0;
        unsigned long sleepStart = 0;
        bool dimming = false;
        DisplayState displayState = AWAKE;
        uint8_t dimmer = 9;
        int8_t page = 0;
        int8_t lastPage = 9;
        bool initialised = false;
        uint16_t buttonPressed;
        unsigned long lastCheckButton;
        DisplayPage *currentPage;

        LogoDisplayPage logoDisplayPage;
        StatsDisplayPage statsDisplayPage;
        ConfigSettingsPage configSettingsPage;



};


