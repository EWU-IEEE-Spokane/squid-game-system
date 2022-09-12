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

uint16_t time_seconds = 90;
boolean counting = false;
boolean out_of_time = false;
uint16_t topOffset = 65;

const uint16_t t1_load = 0;
const uint16_t t1_comp = 62500;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  pinMode(counting_pin, INPUT);
  pinMode(out_of_time_pin, OUTPUT);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  drawClock(10, topOffset, time_seconds, ILI9341_RED);

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
      blankClock(10, topOffset, ILI9341_BLACK);
    }
    drawClock(10, topOffset, time_seconds, ILI9341_RED);
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

void drawDigit(uint16_t x, uint16_t y, uint16_t number, boolean decimal,  uint16_t color){
  // dot to callibrate placement
  /*tft.drawPixel(x, y+1, color);*/
  if(decimal){
    tft.fillCircle(x+63, y+101, 5, color);                             // seg dp
  }
  switch(number){
    case 0:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+58, 10, 30, color);                   // seg e
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      break;
    case 1:
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      break;
    case 2:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+58, 10, 30, color);                   // seg e
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;  
    case 3:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;
    case 4:
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;
    case 5:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;
    case 6:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+58, 10, 30, color);                   // seg e
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;
    case 7:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      break; 
    case 8:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+58, 10, 30, color);                   // seg e
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;
    case 9:
      drawHorizontalSegment(x+13, y+1, 30, 10, color);                 // seg a
      drawVerticalSegment(x+47, y+10, 10, 30, color);                  // seg b
      drawVerticalSegment(x+47, y+58, 10, 30, color);                  // seg c
      drawHorizontalSegment(x+13, y+97, 30, 10, color);                // seg d
      drawVerticalSegment(x+0, y+10, 10, 30, color);                   // seg f
      drawHorizontalSegment(x+13, y+49, 30, 10, color);                // seg g
      break;               
  }
}

void drawClock(uint16_t x, uint16_t y, int time_seconds, uint16_t color){
  int time_minutes = time_seconds/60;
  time_seconds = time_seconds - (time_minutes*60);
  drawDigit(x, y, time_minutes/10, false, color);                      // 4th digit
  drawDigit(x+80, y, time_minutes%10, false, color);                   // 3rd digit
  drawDigit(x+160, y, time_seconds/10, false, color);                  // 2nd digit
  drawDigit(x+240, y, time_seconds%10, false, color);                  // 1st digit
  tft.fillCircle(x+149, 100, 5, color);                                // top middle dot
  tft.fillCircle(x+149, 140, 5, color);                                // bottom middle dot
}

void blankClock(uint16_t x, uint16_t y,  uint16_t color){
  drawDigit(x, y, 8, false, color);                                    // 4th digit
  drawDigit(x+80, y, 8, false, color);                                 // 3rd digit
  drawDigit(x+160, y, 8, false, color);                                // 2nd digit
  drawDigit(x+240, y, 8, false, color);                                // 1st digit
  tft.fillCircle(x+149, 100, 5, color);                                // top middle dot
  tft.fillCircle(x+149, 140, 5, color);                                // bottom middle dot
}
