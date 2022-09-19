#include "stepperControl.h"
#include <stdint.h>

#define RED_LED 3 // check for the correct pin

stepper stepper0(15, 14); // PC1, PC0
stepper stepper1(17, 16);  // PC3, PC2
stepper headStepper(6, 7); // PD6, PD7

// timing variables 
unsigned long start_millis = millis();
unsigned long current_millis = millis();
unsigned long period = 1000;

uint16_t red_light_time = 0;
uint16_t green_light_time = 0;
uint16_t red_time_max = 5; // seconds
uint16_t red_time_min = 1; // seconds
uint16_t red_flash_time = 125; // milliseconds
uint16_t green_time_max = 7; // seconds
uint16_t green_time_min = 3; // seconds



class counter_ {
	public:
	const int8_t enable_pin = 8;	//PB0
	const int8_t status_pin = 9;	//PB1
	const int8_t reset_pin	= 10;	//PB2
	
	bool getStatus(void) {
		return digitalRead(status_pin);
	}
	
	void reset(void) {
		digitalWrite(enable_pin, LOW);
		digitalWrite(reset_pin, HIGH);
		delay(10);
		digitalWrite(reset_pin, LOW);
	}
	
	void start(void) {
		digitalWrite(enable_pin, HIGH);
	}
	
	counter_() {
		pinMode(enable_pin, OUTPUT);
		pinMode(status_pin, INPUT);
		pinMode(reset_pin, OUTPUT);
		
		digitalWrite(reset_pin, LOW);
		digitalWrite(enable_pin, LOW);
	}
} counter;

class player_ {
	public:
	uint8_t pin;
	bool lastState;
	bool playing;
	bool hasChanged(void){
		if (digitalRead(pin) != lastState) {
			lastState != lastState;
			return true;
		}
		return false;
	}
	
	bool state(void){
		return digitalRead(pin);
	}
	player_(uint8_t digitalPin){
		lastState =0;
		pin=digitalPin;
	}
};
player_ player0(2); //PD2
player_ player1(3); //PD3


void setup(){
	Timer2.begin();
	Serial.begin(115200);
}

void loop(){ 
	player0.playing = true; 
	player1.playing = true;
	
	//wait for start
	while(!player1.state() || !player0.state()){};
	
	//startup
	Serial.write("play rules.wav");
	Serial.flush();
	delay(15000);
	
	while (player0.playing || player1.playing){
		// begin green light
		// play green light sound
  		// rotate head 180 degrees
  		green_light_time = random(green_time_min, green_time_max+1);
  		period = green_light_time*1000;
  		current_millis = millis();
  		while (current_millis - start_millis <= period){
  			current_millis = millis();
			// read button state and check players
			// increment players who are pressing their buttons
  		}
  		start_millis = current_millis;
		// end green light
		
		// begin red light
		// play red light sound
  		// rotate head 180 degrees
 	 	period = red_flash_time;
  		start_millis = current_millis;
  		current_millis = millis();  
  		for(int x = 0; x < 11; x++){
  			while (current_millis - start_millis <= period){
  		  		current_millis = millis();
  			}
  			start_millis = current_millis;
  			digitalWrite(RED_LED, !digitalRead(RED_LED));
  		}
  
  		// RED LIGHT SOLID
  		red_light_time = random(red_time_min, red_time_max+1);
  		period = red_light_time*1000;
  		current_millis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  		while (current_millis - start_millis <= period){
  			//test whether the period has elapsed
			current_millis = millis();
			// read button state and check players
  			digitalWrite(RED_LED, HIGH);
  		}
  		start_millis = current_millis;  //IMPORTANT to save the start time of the current count state.
  		digitalWrite(RED_LED, LOW);
		// end red light
	}	
}
