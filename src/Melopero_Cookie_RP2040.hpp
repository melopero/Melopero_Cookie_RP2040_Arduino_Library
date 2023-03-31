/*

Melopero Cookie RP2040 library

Get your Cookie RP2040 here:
www.melopero.com/melopero-cookie-rp2040

Authors: Francesco Marchetti, Luca Davidian
Melopero S.r.l.  - www.melopero.com 
First release March 2023

*/

#ifndef Melopero_Cookie_RP2040_H
#define Melopero_Cookie_RP2040_H

#include <vector>
#include <string>
#include <pico/time.h>
#include <Arduino.h>


#define WORDS_PER_CHAR 25
#define CHAR_WIDTH    5
#define CHAR_HEIGHT    5

#define WS2812_PIN_BASE         17
#define COOKIE_DEFAULT_LED_PIN  21

#define MAX_MESSAGE_LENGTH  100

#define LED_PIN  21
#define BUTTON_A  10
#define BUTTON_B  11



bool repeating_timer_callback(repeating_timer_t *rt);

enum class Direction : uint8_t
{
    LEFT, RIGHT, UP, DOWN,
};

class Melopero_Cookie_RP2040
{
friend bool repeating_timer_callback(struct repeating_timer *t); 
public: 

    uint32_t backgroundCol = 0x00100000;
    uint32_t charCol = 0x10101000;
    
    
    //constructor
    Melopero_Cookie_RP2040();

    //destrucotr
    ~Melopero_Cookie_RP2040();
    
    
    //Show a message on the matrix display, if not specified, default scroll delay is 200ms.
    //Default scrolling direction is LEFT, to change it use setDirection
    void showMessage(String &message, uint32_t timeMS = 200); 

    //set the text color to RED, GREEN, BLUE + Brightness (optional, default is 0.3)
    void setRgbColor(uint8_t r, uint8_t g, uint8_t b, double brightness = 0.3);

    //set the text color to RED, GREEN, BLUE + Brightness (optional, default is 0.3)
    void setRgbBackground(uint8_t r, uint8_t g, uint8_t b, double brightness = 0.3);
    
    //set the scrolling direction, default is LEFT
    void setDirection(Direction direction);

    //enable or disable auto re-started scrolling text
    void setRepeatedStart(bool enable);

    //stop the current scrolling text without clearing the display
    void stopMessage();

    //clear the display with a color, if not specified, the display matrix is turned off (color 0,0,0)
    void clearScreen(uint32_t background = 0);

    //set a specifi pixel with a color and brightness. Once the desired pixels have been set, call showPixels() to write the values on screen
    void setPixel(int pix, uint8_t r, uint8_t g, uint8_t b, double brightness = 0.3);
    
    //set all the 25 pixels at once passing an array of 32-bit neopixels formatted colors.
    //Use formatColor to format neopixels colors starting from RED, GREEN, BLUE and brightness values
    //call showPixels() to write the values on screen
    void setMatrix(uint32_t pixels[25]) ;

    //show all the previously set pixels on screen (use setMatrix or setPixels to set the desired pixels)
    void showPixels();

    //convert RGB + Brightness values to 32-bit neopixel format
    uint32_t formatColor(uint8_t r, uint8_t g, uint8_t b, double brightness=0.3);

    
    //initialize the built-in LED on pin 21, this function has the same result as pinMode(21, OUTPUT)
    void ledInit();
    
    //turn on the built-in LED on pin 21, this function has the same result as digitalWrite(21, HIGH)
    void ledOn();

    //turn off the built-in LED on pin 21, this function has the same result as digitalWrite(21, LOW)
    void ledOff();

    //change the current value of the built-in LED to the opposite
    void ledToggle();


    //initialize the A B user buttons on pins 10 and 11, this function has the same result as pinMode(10, INPUT) and pinMode(11, INPUT)
    void buttonsInit();

    //read the current value on the specified pin, this function has the same result as digitalRead(10) and digitalRead(11)
    //you can pass BUTTON_A and BUTTON_B constants to read user buttons on pins 10 and 11
    bool readButton(uint8_t button);

    String reverseMessage(String &message) const;

private:
    
    
    static Melopero_Cookie_RP2040* cookie_rp2040;

    uint32_t mBuffer[(MAX_MESSAGE_LENGTH+2)*WORDS_PER_CHAR];
    uint32_t mDataIndex = 0;
    Direction mDirection = Direction::LEFT;
    bool repeatedStart = false;
    bool messageStop = false;
    bool dmaBusy = false;
    
    uint32_t mFrame[WORDS_PER_CHAR];
    uint32_t mFrameRow = 0; 
    uint32_t mPixelBuffer[WORDS_PER_CHAR]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint mDMAChannel;
    repeating_timer_t mTimer;
    char mMessage[MAX_MESSAGE_LENGTH];
    uint8_t messageLength;
    uint sm; 
    uint offset;  

    
    void setupAndStartTimer(uint32_t timeMS);
    void pioInit();
    void dmaInit();
    void remapBuffer();
    
    
};



#endif  // Melopero_Cookie_RP2040_H