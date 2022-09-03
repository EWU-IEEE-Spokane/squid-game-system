#include <arduino-timer.h>

#define redLed 2
#define greenLed 3
int state;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, HIGH);
  Serial.begin(9600);
}

void loop() {
  //Generate random delay, switch between states after delay
  int delayTime = random(1000, 3000);
  redLight(delayTime);
  delayTime = random(1000, 3000);
  greenLight(delayTime);
}

//void switchColor (){
//  
//
//


void redLight(delayTime){
  toggle_led();
  int start = millis();
  while (millis() < (start + delayTime){
    //if button is pressed and (digitalRead(redLed), you lose!
  }
}

void greenLight(delayTime){
  toggle_led();
  int start = millis();
  while (millis() < (start + delayTime){
    //if button is pressed, advance stepper
  }  
}

bool toggle_led(void *) {
  //rotate eagle head
  digitalWrite(redLed, !digitalRead(redLed)); // toggle the LED
  digitalWrite(greenLed, !digitalRead(greenLed)); // toggle the LED
  return true; // repeat? true
}
