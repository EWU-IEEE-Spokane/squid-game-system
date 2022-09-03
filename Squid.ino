#include <arduino-timer.h>

#define redLed 2
#define greenLed 3
int state;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  //Generate random delay, switch between states after delay
  int delayTime = random(2);
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  delay(1000*delayTime);
  //redLight(delayTime);
  delayTime = random(2);
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, HIGH);
  delay(1000*delayTime);
  Serial.println(millis());

  //greenLight(delayTime);
  
}

//void switchColor (){
//  
//}
//
//void redLight(delayTime){
//  
//}
//
//void greenLight(delayTime){
//  
//}
