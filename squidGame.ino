#include "stepperControl.h"
#include <stdint.h>

stepper stepper0(15, 14); // PC1, PC0
stepper stepper1(17, 16);  // PC3, PC2
stepper headStepper(6, 7); // PD6, PD7



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
		//green light
		
		//red light
	}
}
