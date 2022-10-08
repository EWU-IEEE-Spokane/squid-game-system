#include "stepperControl.h"
#include <stdint.h>

#define RED_LED 3 // check for the correct pin

stepper stepper0(15, 14); // PC1, PC0
stepper stepper1(17, 16);  // PC3, PC2
stepper headStepper(6, 7); // PD6, PD7

const uint16_t player_step = 10;

// timing variables 
unsigned long start_millis;
unsigned long current_millis;
unsigned long period;

const uint16_t red_time_max = 5; // seconds
const uint16_t red_time_min = 1; // seconds
const uint16_t red_flash_time = 125; // milliseconds
const uint16_t green_time_max = 7; // seconds
const uint16_t green_time_min = 3; // seconds



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

counter_ counter0();


void setup(){
	Timer2.begin();
	Serial.begin(115200);
	stepper0.speedDivisor = 2;
	stepper1.speedDivisor = 2;
	headStepper.speedDivisor = 2;
}

void loop(){ 
	player0.playing = true; 
	player1.playing = true;
	
	counter0.reset();
	delay(3000);
	
	//wait for start
	while(!player1.state() || !player0.state()){};
	
	//startup
	Serial.write("play rules.wav");
	Serial.flush();
	delay(15000);
	
	while (player0.playing || player1.playing){
		// begin green light
		Serial.write("play green_light.wav\n");  // play green light sound
		Serial.flush();
		headStepper.incrementalMove(100);  // rotate head 180 degrees
		headStepper.waitUntilStopped();
		setMillis(random(green_time_min, green_time_max+1)*1000);
		while (current_millis - start_millis <= period){
			current_millis = millis();
			// checking if the players ran out of time
			if (counter0.getStatus();){
				player0.playing = false;
				player1.playing = false;
			}
			// read button state and move players
			if(player0.state() && player0.playing){
				stepper0.incrementalMove(player_step);  // move player forward need to dial in the right value 
				stepper0.waitUntilStopped();
			}
			if(player1.state() && player1.playing){
				stepper1.incrementalMove(player_step);  // move player forward need to dial in the right value 
				stepper1.waitUntilStopped();
			}
		}
		// end green light
		
		// begin red light
		Serial.write("play red_light_quick.wav\n");  // play red light sound
		Serial.flush();
		headStepper.incrementalMove(-100);  // rotate head -180 degrees
		headStepper.waitUntilStopped();
		setMillis(red_flash_time);  
		for(int x = 0; x < 11; x++){
			while (current_millis - start_millis <= period){
		  		current_millis = millis();
				if (counter0.getStatus();){
					player0.playing = false;
					player1.playing = false;
				}
			}
			digitalWrite(RED_LED, !digitalRead(RED_LED));
			setMillis();
		}
  
		// RED LIGHT SOLID
		setMillis(random(red_time_min, red_time_max+1)*1000);  
		while (current_millis - start_millis <= period){
			//test whether the period has elapsed
			current_millis = millis();
			if (counter0.getStatus()){
				player0.playing = false;
				player1.playing = false;
			}
			// read button state and check players
			if(player0.state() && player0.playing){
				player0.playing = false;
			}
			if(player1.state() && player1.playing){
				player1.playing = false;
			}
			digitalWrite(RED_LED, HIGH);
		}
		digitalWrite(RED_LED, LOW);
		// end red light
	}
}

void setMillis(uint16_t millis_period){
	start_millis = millis(); //IMPORTANT to save the start time of the current count state.
	current_millis = millis(); //get the current "time" (actually the number of milliseconds since the program started)
	period = millis_period;
}
