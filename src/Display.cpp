
//Für Display
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <Display.h>

using namespace std;


Display::Display () {
    this->ip = String("no value");
}

void Display::setup () {
    Serial.println(F("Display: setup"));
    delay(3000);
    this->display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    if(!this->display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        this->displayIsInit = false;
        return;
    }
    this->displayIsInit = true;
    Serial.println(F("Display: ready"));
    this->display->clearDisplay();
    this->display->setTextSize(1);
    this->display->setTextColor(SSD1306_WHITE); 
    this->display->setCursor(0, 0);
    this->display->println(F("Display ready!"));
    this->display->display();
    delay(3000);
}

void Display::update() {
    //Serial.println(F("Display: update"));
    this->display->clearDisplay();


    this->display->setTextSize(0.5);
    //this->display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    this->display->setTextColor(SSD1306_WHITE);
    this->display->setCursor(0, 0);

    int einzug = 35;
    int space = 1;

    this->display->print(F("Wifi: "));
    this->display->setCursor(einzug, this->display->getCursorY());
    if(this->wifiStatus) {
        if(this->wifiName != "") {
            int len = 15;
            char char_array[len];
            this->wifiName.toCharArray(char_array, len);
            this->display->write(char_array);
            this->display->println(F(""));
        }else {
            this->display->println(F("connected"));
        }
        
    }else {
        this->display->println(F("disconnected"));
    }

    this->display->setCursor(this->display->getCursorX(), this->display->getCursorY()+space);
    this->display->print(F("IP: "));
    this->display->setCursor(einzug, this->display->getCursorY());
    this->display->println(this->ip);

    this->display->setCursor(this->display->getCursorX(), this->display->getCursorY()+space);
    this->display->print(F("TP: "));
    this->display->setCursor(einzug, this->display->getCursorY());
    this->display->println(String(this->temp1) + String(" / ") + String(this->temp2) );

    this->display->setCursor(this->display->getCursorX(), this->display->getCursorY()+space);
    this->display->print(F("TPS: "));
    this->display->setCursor(einzug, this->display->getCursorY());
    this->display->println(this->tpStatus);

    this->display->setCursor(this->display->getCursorX(), this->display->getCursorY()+space);
    this->display->print(F("Time:"));
    this->display->setCursor(einzug, this->display->getCursorY());
    this->display->println(this->time);

    this->display->setCursor(this->display->getCursorX(), this->display->getCursorY()+space);
    this->display->print(F("Update:"));
    this->display->setCursor(einzug*1.2, this->display->getCursorY());
    this->display->println(this->updateTime);




    this->display->setCursor(120, 55);
    char a[ ] = "/-\\|";
    this->display->write(a[this->work++]);
    if(this->work+1 > strlen(a)) {
        this->work = 0;
    }



    if(this->currentjob != "") {
        this->display->setCursor(0, 55);
        this->display->println(this->currentjob);
    }

    this->display->display();
}