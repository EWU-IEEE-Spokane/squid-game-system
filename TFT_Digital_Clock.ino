/***************************************************
  Written by Justin Liebert for IEEE EWU chapter Sept 8, 2022.
 ****************************************************/

#include<SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_DC 9
#define TFT_CS 10
#define counting_pin 8
#define out_of_time_pin 7 

uint16_t time_seconds = 15;
boolean counting = false;
boolean out_of_time = false;
uint16_t topOffset = 65;

const uint16_t t1_load = 0;
const uint16_t t1_comp = 62500;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/*
pinout      nano      LCD
            13        CLK
            12        MISO
            11        MOSI
            10        CS      
            9         D/C
            nano      master control
            8         enables countdown
            7         out of time indication
            RST       send a low pulse to reset the nano
*/

void setup() {
  pinMode(counting_pin, INPUT);
  pinMode(out_of_time_pin, OUTPUT);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  // Reset timer1 control register A
  TCCR1A = 0;

  // Set the prescaler to 256
  TCCR1B |= (1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);

  // Reset timer1 compare value
  TCNT1 = t1_load;
  OCR1A = t1_comp;

  // Enable timer1 compare interrupt
  TIMSK1 = (1 << OCIE1A);

  // Enable global interrupts
  sei();
}

void loop() {
  delay(1000);
}

ISR(TIMER1_COMPA_vect){
  TCNT1 = t1_load;
  counting = digitalRead(counting_pin);
  if(out_of_time == false){
    if(time_seconds == 1){
      out_of_time = true;
    }
    if(counting){
      time_seconds--;
      blankClock(10, topOffset, 3, ILI9341_BLACK);
    }
    drawClock(10, topOffset, time_seconds, 3, ILI9341_RED);
  }
  digitalWrite(out_of_time_pin, out_of_time);
}

void drawVerticalSegment(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color){
  // dots to callibrate placement
  /*tft.drawPixel(x, y, color);
  tft.drawPixel(x+width-1, y, color);
  tft.drawPixel(x, y+height+width-1, color);
  tft.drawPixel(x+width-1, y+height+width-1, color);*/
  tft.fillRect(x, y+(width/2), width, height, color);
  for(int i = 0; i < (width/2); i++){
    tft.drawLine(x+i, y+(width/2)-i, x+width-1-i, y+(width/2)-i, color);
    tft.drawLine(x+i, y+(width/2)+height+i, x+width-1-i, y+(width/2)+height+i, color);
  }
}

void drawHorizontalSegment(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color){
  // dots to callibrate placement
  /*tft.drawPixel(x-(height/2)+1, y, color);
  tft.drawPixel(x-(height/2)+1, y+height-1, color);
  tft.drawPixel(x-(height/2)+width+height-1, y, color);
  tft.drawPixel(x-(height/2)+width+height-1, y+height-1, color);*/
  tft.fillRect(x, y, width, height, color);
  for(int i = 0; i < (height/2); i++){
    tft.drawLine(x-i, y+i, x-i, y+height-1-i, color);
    tft.drawLine(x+width+i, y+i, x+width+i, y+height-1-i, color);
    }
}

void drawDigit(uint16_t x, uint16_t y, uint16_t number, uint8_t scale, boolean decimal,  uint16_t color){
  // dot to callibrate placement
  /*tft.drawPixel(x, y+1, color);*/
  uint16_t width = 0;
  uint16_t height = 0;
  uint8_t decimal_radius = 0;
  int offsets[10];
  switch(scale){
    case 1:
      width = 3;
      height = 8;
      decimal_radius = 1;
      offsets[0] = 3; offsets[1]= 12; offsets[2] = 0; offsets[3] = 1; offsets[4]= 3;
      offsets[5] = 16; offsets[6] = 26; offsets[7] = 13; offsets[8] = 18; offsets[9] = 27;
      break;
    case 2:
      width = 5;
      height = 13;
      decimal_radius = 2;
      offsets[0] = 6; offsets[1]= 21; offsets[2] = 0; offsets[3] = 1; offsets[4]= 5;
      offsets[5] = 26; offsets[6] = 43; offsets[7] = 22; offsets[8] = 30; offsets[9] = 45;
      break;
    case 3:
      width = 7;
      height = 18;
      decimal_radius = 3;
      offsets[0] = 9; offsets[1]= 30; offsets[2] = 0; offsets[3] = 1; offsets[4]= 7;
      offsets[5] = 37; offsets[6] = 61; offsets[7] = 31; offsets[8] = 43; offsets[9] = 64;
      break;
    case 4:
      width = 8;
      height = 22;      
      decimal_radius = 4;
      offsets[0] = 10; offsets[1]= 35; offsets[2] = 0; offsets[3] = 1; offsets[4]= 8;
      offsets[5] = 44; offsets[6] = 73; offsets[7] = 37; offsets[8] = 50; offsets[9] = 76;
      break;
    case 5:
      width = 10;
      height = 30;      
      decimal_radius = 5;
      offsets[0] = 13; offsets[1]= 47; offsets[2] = 0; offsets[3] = 1; offsets[4]= 10;
      offsets[5] = 58; offsets[6] = 97; offsets[7] = 49; offsets[8] = 63; offsets[9] = 101;
      break;
  }
      
  if(decimal){
    tft.fillCircle(x+offsets[8], y+offsets[9], decimal_radius, color);                  // seg dp
  }
  
  switch(number){
    case 0:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[5], width, height, color);            // seg e
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      break;
    case 1:
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      break;
    case 2:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[5], width, height, color);            // seg e
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;  
    case 3:    
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;
    case 4:
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;
    case 5:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;
    case 6:    
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[5], width, height, color);            // seg e
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;
    case 7:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      break; 
    case 8:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[5], width, height, color);            // seg e
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;
    case 9:
      drawHorizontalSegment(x+offsets[0], y+offsets[3], height, width, color);          // seg a
      drawVerticalSegment(x+offsets[1], y+offsets[4], width, height, color);            // seg b
      drawVerticalSegment(x+offsets[1], y+offsets[5], width, height, color);            // seg c
      drawHorizontalSegment(x+offsets[0], y+offsets[6], height, width, color);          // seg d
      drawVerticalSegment(x+offsets[2], y+offsets[4], width, height, color);            // seg f
      drawHorizontalSegment(x+offsets[0], y+offsets[7], height, width, color);          // seg g
      break;               
  }
}

void drawClock(uint16_t x, uint16_t y, int time_seconds, uint8_t scale, uint16_t color){
  uint8_t colon_radius = 0;
  int offsets[4];
  switch(scale){
    case 1:
      colon_radius = 1;
      offsets[0] = 22; offsets[1]= 40; offsets[2] = 8; offsets[3] = 21;
      break;
    case 2:
      colon_radius = 2;
      offsets[0] = 35; offsets[1]= 65; offsets[2] = 13; offsets[3] = 35;
      break;
    case 3:
      colon_radius = 3;
      offsets[0] = 50; offsets[1]= 93; offsets[2] = 18; offsets[3] = 48;
      break;
    case 4:
      colon_radius = 4;
      offsets[0] = 60; offsets[1]= 111; offsets[2] = 23; offsets[3] = 58;
      break;
    case 5:
      colon_radius = 5;
      offsets[0] = 80; offsets[1]= 149; offsets[2] = 35; offsets[3] = 75;
      break;
  }
  
  int time_minutes = time_seconds/60;
  time_seconds = time_seconds - (time_minutes*60);
  drawDigit(x, y, time_minutes/10, scale, false, color);                                  // 4th digit
  drawDigit(x+offsets[0], y, time_minutes%10, scale, false, color);                       // 3rd digit
  drawDigit(x+2*offsets[0], y, time_seconds/10, scale, false, color);                     // 2nd digit
  drawDigit(x+3*offsets[0], y, time_seconds%10, scale, false, color);                     // 1st digit

  tft.fillCircle(x+offsets[1], y+offsets[2], colon_radius, color);                          // top middle dot
  tft.fillCircle(x+offsets[1], y+offsets[3], colon_radius, color);                          // bottom middle dot
}

void blankClock(uint16_t x, uint16_t y,  uint8_t scale, uint16_t color){
  uint8_t colon_radius = 0;
  int offsets[4];
  switch(scale){
    case 1:
      colon_radius = 1;
      offsets[0] = 22; offsets[1]= 40; offsets[2] = 8; offsets[3] = 21;
      break;
    case 2:
      colon_radius = 2;
      offsets[0] = 35; offsets[1]= 65; offsets[2] = 13; offsets[3] = 35;
      break;
    case 3:
      colon_radius = 3;
      offsets[0] = 50; offsets[1]= 93; offsets[2] = 18; offsets[3] = 48;
      break;
    case 4:
      colon_radius = 4;
      offsets[0] = 60; offsets[1]= 111; offsets[2] = 23; offsets[3] = 58;
      break;
    case 5:
      colon_radius = 5;
      offsets[0] = 80; offsets[1]= 149; offsets[2] = 35; offsets[3] = 75;
      break;
  }
  drawDigit(x, y, 8, scale, false, color);                                                // 4th digit
  drawDigit(x+offsets[0], y, 8, scale, false, color);                                     // 3rd digit
  drawDigit(x+2*offsets[0], y, 8, scale, false, color);                                   // 2nd digit
  drawDigit(x+3*offsets[0], y, 8, scale, false, color);                                   // 1st digit
  tft.fillCircle(x+offsets[1], y+offsets[2], colon_radius, color);                        // top middle dot
  tft.fillCircle(x+offsets[1], y+offsets[3], colon_radius, color);                        // bottom middle dot
}
