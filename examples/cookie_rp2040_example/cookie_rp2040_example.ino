/*

Melopero Cookie RP2040  example

Get your Cookie RP2040 here:
www.melopero.com/melopero-cookie-rp2040

Authors: Francesco Marchetti, Luca Davidian
Melopero S.r.l.  - www.melopero.com 
First release March 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Melopero_Cookie_RP2040.hpp"

String message = "HELLO COOKIE";


Melopero_Cookie_RP2040 cookie;

//format a 32bit color for use with neopixels starting from RGB + Brightness values. 
uint32_t color = cookie.formatColor(20,50,40,0.2);

void setup()
{
    
    //Initialize the green LED on GPIO21, same as PinMode(21, OUTPUT);
    cookie.ledInit();

    //Initialize the user buttons A/B on GPIO10/GPIO11, same as PinMode(10, INPUT); and PinMode(11, INPUT);
    cookie.buttonsInit();
    
    //set neopixel #5 to rgb color (255, 255, 0)
    cookie.setPixel(5, 255 , 255,0);

    //write the pixels on screen
    cookie.showPixels();

    delay(2000);
    
    //create a matrix of 25 pixels and assign to them a color
    uint32_t pix[25];
    
    for(int i=0; i<25; i++)
    {
        pix[i]=color;
    }

    //set all the 25 neopixels at once, passing an array of 25 colors. 
    //colors must be formatted as uin32_t, this can be done using formatColor(...) 
    //and passing as parameters r, g, b values along with brightness (optional)
    cookie.setMatrix(pix);

    //write the pixels on screen
    cookie.showPixels();


    delay(2000);
    
    color=cookie.formatColor(0, 40, 100);

    //clear the screen with color, call clearScreen() with no input to turn off all the pixels
    cookie.clearScreen(color);

    delay(2000);

    //By default the message is displayed once. 
    //enable the following line to set a repeated message
    //cookie.setRepeatedStart(true);

    cookie.showMessage(message);
    


    
}

void loop()
{
  //read the value of BUTTON_A (optionally BUTTON_B) and if pressed clear the screen with color
if(cookie.readButton(BUTTON_A))  cookie.clearScreen(color);
else cookie.clearScreen(); //else turn off all the pixels
  

  //toggle the buil-in LED
  cookie.ledToggle();
  delay(100);

}


    


