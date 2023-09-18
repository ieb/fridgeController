
#if defined( OLED_SCREEN_ADDRESS )

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "ssd1306.h"
#include "splash.h"

extern const char * stateDisplay[3];


void i2cScan() {
    for(uint8_t address = 0; address < 255; address++ ) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
          if (address<16) {
            Serial.print(" 0");
          } else {
            Serial.print("  ");
          }
          Serial.print(address,HEX);
        } else {
          Serial.print(" --");
        }
        if ( address%32 == 31 ) {
            Serial.println("");
        }
    }
    Serial.println("");
}


void LCDDisplay::begin() {
    pinMode(BTN_PIN, INPUT);  
  Wire.begin(21,22);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    i2cScan();
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }
  initialised = true;
  Serial.print(F("SSD1306 started"));Serial.println(OLED_SCREEN_ADDRESS,HEX);
  staticPagePress = millis();
  lastDisplay = staticPagePress;
  lastDim = staticPagePress;
  displayState = AWAKE;

  logoDisplayPage.setNextPage(&statsDisplayPage);
  logoDisplayPage.setHold5sPage(&configSettingsPage);
  statsDisplayPage.setNextPage(&logoDisplayPage);  // back to logo
  statsDisplayPage.setHold5sPage(&configSettingsPage);
  configSettingsPage.setNextPage(&logoDisplayPage); // back to logo
  currentPage = &logoDisplayPage;

  currentPage->update( FORCE_UPDATE, &display);
}



void LCDDisplay::update(float voltage, float *temperatures, int16_t rpm, int8_t state) {
    statsDisplayPage.update(voltage, temperatures, rpm, state);
    update();
}

ButtonPress LCDDisplay::updateButtonPress() {
    unsigned long now = millis();
    ButtonPress ret = NOPRESS;
    if ( now-lastCheckButton > 50 ) {
        lastCheckButton = now;
        if ( digitalRead(BTN_PIN) == LOW ) {
            buttonPressed++;
            if ( buttonPressed == 20 ) {
                Serial.println("1s");
                ret = HOLD1s;
            } else if ( buttonPressed == 60 ) {
                Serial.println("3s");
                ret = HOLD3s;
            } else if ( buttonPressed == 100 ) {
                Serial.println("5s");
                ret = HOLD5s;
            }
        } else {
            if ( buttonPressed > 1 ) {
                if (buttonPressed < 20) {
                    Serial.println("p");
                    ret = PRESS;
                } else {
                    Serial.print("^");
                    ret = RELEASE;
                }
            }
            buttonPressed = 0;
        }

    }
    return ret;
}


void LCDDisplay::update() {
    ButtonPress bp = updateButtonPress();
    DisplayPage * next = currentPage->processPress(bp, &display);
    if (next != NULL ) {
        currentPage = next;
    } else {
        currentPage = currentPage->update(bp, &display);
    }
}



void LCDDisplay::dim() {
    if (dimmer == 0) {
        dimmer = 10;
    }
    dimmer--;
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(dimmer*25);
    display.clearDisplay();
    display.setCursor(52,20);              // Start at top-left corne
    display.setTextSize(3);   // 18x24, 4 rows, 10.6 chars
    display.printf("%d",dimmer); // ADC0 is service battery 
    display.display();

}

DisplayPage * DisplayPage::processPress(ButtonPress bp, Adafruit_SSD1306 * display) {
  if ( bp == PRESS ) {
    subPage++;
    if ( subPage >= maxSubPages ) {
        subPage = 0;
        Serial.println("display -> next");
        nextPage->update(FORCE_UPDATE, display);
        return nextPage;
    }
  } else if ( bp == HOLD5s && hold5sPage != NULL ) {
        subPage = 0;
        Serial.println("display -> hold5s");
        hold5sPage->update(FORCE_UPDATE, display);
        return hold5sPage;
  }
  return NULL;

}

DisplayPage * LogoDisplayPage::update(ButtonPress bp, Adafruit_SSD1306 * display) {
    unsigned long now = millis();
    if ( bp == FORCE_UPDATE ||  now - last > displayUpdate ) {
        last = now;
        Serial.println("Logo display");
        display->clearDisplay();
        display->drawBitmap((display->width() - spash12864_width) / 2, (display->height() - spash12864_height) / 2,
                   spash12864_data, spash12864_width, spash12864_height, 1);
        display->display();
    }
    return this;    
}



DisplayPage * StatsDisplayPage::update(ButtonPress bp, Adafruit_SSD1306 * display) {
    unsigned long now = millis();
    if ( bp == FORCE_UPDATE || bp == PRESS || now - last > displayUpdate ) {
        last = now;
        Serial.println("State display");
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE); // Draw white text
        switch(subPage) {
            case 0:
              display->setFont(&FreeSans24pt7b);
              display->setTextWrap(false);
              display->setCursor(0,48);
              display->printf("%4.1fc", temperatures[BOX_SENSOR]);
              display->setFont();
              break;
            case 1:
              display->setFont(&FreeSans9pt7b);
              display->setTextWrap(false);
              display->setCursor(0,16-3);
              display->printf("tNow %4.1fC", settings->config.turnOffTemperature[state] );  
              display->setCursor(0,32-3);              
              display->printf("tOn %4.1fC", settings->config.turnOnTemperature[state]); 
              display->setCursor(0,48-3);
              display->printf("tOff %4.1fC", settings->config.turnOffTemperature[state] );  
              display->setCursor(0,64-3);
              display->printf("%s", stateDisplay[state] );  
              display->setFont();
              break;
            case 2:
              display->setFont(&FreeSans9pt7b);
              display->setTextWrap(false);
              display->setCursor(0,16-3);
              display->printf("%5.2fv", voltage/1000.0); 
              display->setCursor(0,32-3);              
              display->printf("%4d rev", rpm ); 
              display->setCursor(0,48-3);
              display->printf("tEva %4.1fc", temperatures[EVAPORATOR_SENSOR] );//voltage/1000.0); 
              display->setCursor(0,64-3);
              display->printf("tDis %4.1fc", temperatures[DISCHARGE_SENSOR] );//voltage/1000.0); 
              display->setFont();
              break;

        }
        display->display();
    }
    return this;    
}

void StatsDisplayPage::update(float voltage, float * temperatures, int16_t rpm, int8_t state) {
    this->voltage = voltage;
    this->temperatures[EVAPORATOR_SENSOR] = temperatures[EVAPORATOR_SENSOR];
    this->temperatures[DISCHARGE_SENSOR] = temperatures[DISCHARGE_SENSOR];
    this->temperatures[BOX_SENSOR] = temperatures[BOX_SENSOR];
    this->rpm = rpm;
    this->state = state;
}

DisplayPage * ConfigSettingsPage::update(ButtonPress bp, Adafruit_SSD1306 * display) {
    unsigned long now = millis();
    if ( bp == FORCE_UPDATE || now - last > displayUpdate ) {
        last = now;
        Serial.println("ConifgSettings display");
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE); // Draw white text
        switch(subPage) {
            case 0:
              display->setFont(&FreeSans12pt7b);
              display->setTextWrap(false);
              display->setCursor(0,62);
              display->printf("Press for 5s\n");
              display->printf("for config\n");
              display->setFont();
              break;
        }
        display->display();
    }
    return this;    
}


#endif