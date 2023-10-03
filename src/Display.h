#ifndef MyDisplay_h
#define MyDisplay_h

#include <string>

//Für Display
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

using namespace std;

class Display {
    const int SCREEN_WIDTH  = 128;
    const int SCREEN_HEIGHT = 64;
    const int OLED_RESET    =  -1;
    const uint8_t SCREEN_ADDRESS = 0x3C;

    private:
        Adafruit_SSD1306 *display;
        bool displayIsInit = false;

    public:
        Display();
        void setup();
        void update();
        String ip;
        bool wifiStatus = false;
        String wifiName = "";
        int work = 0;
        String currentjob = "";
        float temp1 = 0;
        float temp2 = 0;
        String time = "";
        String updateTime = "";
        bool heating = true;
        bool pumping = false;
        bool light = false;
        float volt = 0;
        String tpStatus = "no status";
};

#endif