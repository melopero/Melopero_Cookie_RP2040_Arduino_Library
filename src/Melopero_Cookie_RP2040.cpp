/*

Melopero Cookie RP2040 library

Product page:
www.melopero.com/melopero-cookie-rp2040

Authors: Francesco Marchetti, Luca Davidian
Melopero S.r.l.  - www.melopero.com 
First release March 2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "melopero_cookie_rp2040.hpp"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include <pico/time.h>
#include "font.h"
#include <cstring>





Melopero_Cookie_RP2040* Melopero_Cookie_RP2040::cookie_rp2040 = nullptr;



Melopero_Cookie_RP2040::Melopero_Cookie_RP2040()
{
    
    pioInit();
    dmaInit();
    
    cookie_rp2040=this;
    

}

 Melopero_Cookie_RP2040::~Melopero_Cookie_RP2040()
 {
    // if(cookie_rp2040!=nullptr)
    // {
        clearScreen();
        //stop the bitstream sm
        pio_sm_set_enabled(pio0, sm, false);
        //abort any in-progress DMA transfer
        dma_channel_abort(mDMAChannel);
        //Free dma channel
        dma_channel_unclaim(mDMAChannel);
        //free sm
        pio_sm_unclaim(pio0, sm);
        //remove program
        pio_remove_program(pio0, &ws2812_program, offset);
        pio_clear_instruction_memory(pio0);	
        
        cookie_rp2040=nullptr;
    // }
   
 }

void Melopero_Cookie_RP2040::pioInit()
{
    
    PIO pio = pio0;
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN_BASE, 800000);

}



void Melopero_Cookie_RP2040::dmaInit()
{
   
        mDMAChannel = dma_claim_unused_channel(true);
        dma_channel_config channel_config = dma_channel_get_default_config(mDMAChannel);
        channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
        channel_config_set_read_increment(&channel_config, true);
        channel_config_set_write_increment(&channel_config, false);
        channel_config_set_dreq(&channel_config, DREQ_PIO0_TX0);
        
        uint sm = 0;
       
        dma_channel_configure(
        mDMAChannel,
        &channel_config,
        &pio0_hw->txf[sm],
        NULL,
        25,
        false);

}



void Melopero_Cookie_RP2040::showMessage(String &message, uint32_t timeMS)
    {
        
        while(dmaBusy);
        dmaBusy=true;

        if(messageStop) messageStop=false;

        if(message.length()>MAX_MESSAGE_LENGTH)
        {   
            message.toCharArray(mMessage, MAX_MESSAGE_LENGTH);
            messageLength = MAX_MESSAGE_LENGTH;
            
        }
        else
        {
            message.toCharArray(mMessage, message.length());
            messageLength=message.length();
        }

        
        
        
 
        switch (mDirection)
        {
            case Direction::UP:  

                for (int i = 0; i < (int) message.length(); i++)
                {
                    const uint8_t (&character)[CHAR_WIDTH] = glyphs[message[i] - 32];
                    

                    for (int row = 0; row < CHAR_HEIGHT; row++)
                        for (int col = 0; col < CHAR_WIDTH; col++)
                        {
                            uint8_t characterCol = character[col]; 
                            uint32_t color = characterCol & 1 << ((CHAR_HEIGHT - 1) - row) ? charCol : backgroundCol;
                            mBuffer[row * CHAR_WIDTH + col + (i + 1) * WORDS_PER_CHAR] = color;
                        }      
                }

                mDataIndex = 0;

                break;

            case Direction::DOWN:
            {
                String reversedMessage = reverseMessage(message);
                
                for (int i = 0; i < (int)reversedMessage.length(); i++)
                {
                    const uint8_t (&character)[CHAR_WIDTH] = glyphs[reversedMessage[i] - 32];

                    for (int row = 0; row < CHAR_HEIGHT; row++)
                        for (int col = 0; col < CHAR_WIDTH; col++)
                        {
                            uint8_t characterCol = character[col]; 
                            uint32_t color = characterCol & 1 << ((CHAR_HEIGHT - 1) - row) ? charCol : backgroundCol;
                            mBuffer[row * CHAR_WIDTH + col + (i + 1) * WORDS_PER_CHAR] = color;
                        } 
                }
                
                mDataIndex = (reversedMessage.length() + 1) * WORDS_PER_CHAR;

                break;
            }    
              
            case Direction::LEFT:
                                
                for (int i = 0; i < (int) messageLength; i++)
                {
                    const uint8_t (&character)[CHAR_WIDTH] = glyphs[message[i] - 32];

                    for (int row = 0; row < CHAR_HEIGHT; row++)
                        for (int col = 0; col < CHAR_WIDTH; col++)
                        {
                            uint8_t characterCol = character[row]; 
                            uint32_t color = characterCol & 1 << col ? charCol : backgroundCol;
                            mBuffer[row * CHAR_WIDTH + col + (i + 1) * WORDS_PER_CHAR] = color;
                            
                        } 
                }

                mFrameRow = 0;

                break;

            case Direction::RIGHT:
            {
                String reversedMessage = reverseMessage(message);

                for (int i = 0; i < (int) messageLength; i++)
                {
                    const uint8_t (&character)[CHAR_WIDTH] = glyphs[reversedMessage[i] - 32];

                    for (int row = 0; row < CHAR_HEIGHT; row++)
                        for (int col = 0; col < CHAR_WIDTH; col++)
                        {
                            uint8_t characterCol = character[row]; 
                            uint32_t color = characterCol & 1 << col ? charCol : backgroundCol;
                            mBuffer[row * CHAR_WIDTH + col + (i + 1) * WORDS_PER_CHAR] = color;
                        } 
                }

                mFrameRow = (messageLength + 1) * CHAR_WIDTH; 

                break;
            }
        }

        // fill first and last empty character
        for (int row = 0; row < CHAR_HEIGHT; row++)
            for (int col = 0; col < CHAR_WIDTH; col++)
            {
                mBuffer[row * CHAR_WIDTH + col] = backgroundCol;
                mBuffer[row * CHAR_WIDTH + col + (messageLength + 1) * WORDS_PER_CHAR] = backgroundCol;
            } 
 
        setupAndStartTimer(timeMS);
    }

void Melopero_Cookie_RP2040::setupAndStartTimer(uint32_t timeMS)
    {
        add_repeating_timer_ms(timeMS, repeating_timer_callback, this, &mTimer);
    }


uint32_t Melopero_Cookie_RP2040::formatColor(uint8_t r, uint8_t g, uint8_t b, double brightness)
{
   
       r *= brightness;
       g *= brightness;
       b *= brightness;

    return ((uint32_t)g << 24 | (uint32_t)r << 16 | b << 8);
}



void Melopero_Cookie_RP2040::setRgbColor(uint8_t r, uint8_t g, uint8_t b, double brightness)
{
    if (brightness >1) {brightness=1;}
    if (brightness <0) {brightness =0.1;}
    charCol = formatColor(r, g, b, brightness);
    
}

void Melopero_Cookie_RP2040::setRgbBackground(uint8_t r, uint8_t g, uint8_t b, double brightness)
{   
    if (brightness >1) {brightness=1;}
    if (brightness <0) {brightness =0.1;}
    backgroundCol = formatColor(r, g, b, brightness);
}


void Melopero_Cookie_RP2040::setDirection(Direction direction)
{
    mDirection = direction;
}

void Melopero_Cookie_RP2040::remapBuffer()
    {
        for (uint row = 0; row < CHAR_HEIGHT; row++)
            for (uint col = 0; col < CHAR_WIDTH; col++)
                mFrame[row * CHAR_WIDTH + col] = mBuffer[(mFrameRow + col) * CHAR_HEIGHT + (CHAR_HEIGHT - 1) - row];
    }
    

String Melopero_Cookie_RP2040::reverseMessage(String &message) const 
    {
        String reversed = message;
        //reversed.reserve(messageLength);

        char* end = &message[messageLength-1];

        for (int i=0; i<messageLength; i++)
        {
            reversed[i] = *end;
            end--;
        }
           
        return reversed;
    }


void Melopero_Cookie_RP2040::setRepeatedStart(bool enable)
{
    repeatedStart=enable;
}


void Melopero_Cookie_RP2040::stopMessage()
{
    messageStop=true;
    
}


void Melopero_Cookie_RP2040::clearScreen(uint32_t background)
{
    if(!messageStop) stopMessage();

    while(dmaBusy);
    dmaBusy=true;
    
    for (int i = 0; i<25; i++)
    {
            mFrame[i] = background; 
            mPixelBuffer[i]=0;
            
    }
    
    dma_channel_set_read_addr(mDMAChannel,mFrame, true);
    sleep_ms(1);
    dmaBusy=false;

}


void Melopero_Cookie_RP2040::setPixel(int pix, uint8_t r, uint8_t g, uint8_t b, double brightness)
{   
    if (brightness >1) {brightness=1;}
    if (brightness <0) {brightness =0.1;}
    uint32_t color = formatColor(r,g,b,brightness);
    mPixelBuffer[pix]=color;
}



void Melopero_Cookie_RP2040::setMatrix(uint32_t pixels[25])
{
   
    for(int i=0; i<25; i++)
    {
        mPixelBuffer[i]=pixels[i];

    }
}


void Melopero_Cookie_RP2040::showPixels()
{
    while(dmaBusy);

    dmaBusy=true;

    dma_channel_set_read_addr(mDMAChannel,mPixelBuffer, true);
    
    sleep_ms(1);
    
    dmaBusy=false;

}


bool repeating_timer_callback(repeating_timer_t *rt)
{   


    switch (((Melopero_Cookie_RP2040*)(rt->user_data))->mDirection)
    {
        case Direction::UP:

            if (((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex >= (((Melopero_Cookie_RP2040 *)(rt->user_data))->messageLength + 1) * WORDS_PER_CHAR)  
            {
                ((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex = 0;

                 //check if repeated start is enables, if not cancel timer
                    if(!((Melopero_Cookie_RP2040*)(rt->user_data))->repeatedStart)
                    {
                        ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;
                        return false;
                    } 
            }

             //check if stop command has been received    
            if(((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop)
            {
                //disable stop trigger and cancel timer 
                ((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop=false;
                ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;

                return false;
            }
                

            dma_channel_set_read_addr(((Melopero_Cookie_RP2040*)(rt->user_data))->mDMAChannel, ((Melopero_Cookie_RP2040*)(rt->user_data))->mBuffer + ((Melopero_Cookie_RP2040*)(rt->user_data))->mDataIndex, true);
            
            ((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex += CHAR_WIDTH;

            break;

        case Direction::DOWN:

            if (((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex <= 0)
            {
                ((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex = (((Melopero_Cookie_RP2040 *)(rt->user_data))->messageLength + 1) * WORDS_PER_CHAR; 

                //check if repeated start is enables, if not cancel timer
                    if(!((Melopero_Cookie_RP2040*)(rt->user_data))->repeatedStart)
                    {
                        ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;
                        return false;
                    } 
            }
                
            //check if stop command has been received    
            if(((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop)
            {
                //disable stop trigger and cancel timer 
                ((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop=false;
                ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;

                return false;
            }

            dma_channel_set_read_addr(((Melopero_Cookie_RP2040*)(rt->user_data))->mDMAChannel, ((Melopero_Cookie_RP2040*)(rt->user_data))->mBuffer + ((Melopero_Cookie_RP2040*)(rt->user_data))->mDataIndex, true);
            
            ((Melopero_Cookie_RP2040 *)(rt->user_data))->mDataIndex -= CHAR_WIDTH;
            
            break;

        case Direction::LEFT:

            
            if (((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow >= (((Melopero_Cookie_RP2040*)(rt->user_data))->messageLength + 1) * CHAR_WIDTH)
                {
                    ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow = 0;

                    //check if repeated start is enables, if not cancel timer
                    if(!((Melopero_Cookie_RP2040*)(rt->user_data))->repeatedStart)
                    {
                        ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;
                        return false;

                    } 
                }

            //check if stop command has been received    
            if(((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop)
            {
                //disable stop trigger and cancel timer 
                ((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop=false;
                ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;

                return false;
            }

            ((Melopero_Cookie_RP2040*)(rt->user_data))->remapBuffer();
            dma_channel_set_read_addr(((Melopero_Cookie_RP2040*)(rt->user_data))->mDMAChannel, ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrame, true);
            
            ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow++;
            
            break;

        case Direction::RIGHT:

    


             if (((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow <= 0)
            {
                ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow = (((Melopero_Cookie_RP2040*)(rt->user_data))->messageLength + 1) * CHAR_WIDTH;

                //check if repeated start is enables, if not cancel timer
                if(!((Melopero_Cookie_RP2040*)(rt->user_data))->repeatedStart)
                {
                    ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;
                    return false;

                } 

            }

            //check if stop command has been received    
            if(((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop)
            {
                //disable stop trigger and cancel timer 
                ((Melopero_Cookie_RP2040*)(rt->user_data))->messageStop=false;
                ((Melopero_Cookie_RP2040*)(rt->user_data))->dmaBusy=false;

                return false;
            }  

            ((Melopero_Cookie_RP2040*)(rt->user_data))->remapBuffer();
            dma_channel_set_read_addr(((Melopero_Cookie_RP2040*)(rt->user_data))->mDMAChannel, ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrame, true);
            
            ((Melopero_Cookie_RP2040*)(rt->user_data))->mFrameRow--;
            
            break;




    }

    return true;
}



void Melopero_Cookie_RP2040::ledInit()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

}


void Melopero_Cookie_RP2040::ledOn()
{
    gpio_put(LED_PIN, true);
}


void Melopero_Cookie_RP2040::ledOff()
{
    gpio_put(LED_PIN, false);
}


void Melopero_Cookie_RP2040::ledToggle()
{
    uint32_t mask = (1UL << LED_PIN);
    gpio_xor_mask(mask);
}


void Melopero_Cookie_RP2040::buttonsInit()
{
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);

    
}


bool Melopero_Cookie_RP2040::readButton(uint8_t button)
{
    return (gpio_get(button));
}
