//Header
#pragma once
#include <stdint.h>
#include <limits.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <wiring.c>

#define MAX_SUBSCRIBERS_TIMER2 4 //Small due to static memory usage... also divides clock.

//Only one "thread" should access the stepper objects. 
//If other "threads" or ISRs share a stepper object, they must make all accesses inside ATOMIC_BLOCK
class stepper {
	public: 
	void absoluteMove(int newPosition); 		//invalid positions silently ignored...
	void incrementalMove(int changePositionBy); //invalid positions silently ignored...
	void setHome(void); //Sets the current position as home
	void goHome(void); //Sets the target position as home
	void waitUntilStopped(void);//Busy waits until the stepper motor is idle.
	bool isStopped(void);
	void stop(void);
	
	uint8_t speedDivisor = 1;
	volatile int maxPosition = INT_MAX;
	volatile int minPosition = INT_MIN;
	
	stepper(uint8_t stepPin, uint8_t dirPin);
	~stepper();
	
	void tick(void);
	private:
	stepper(); //No default constructor
	volatile int currentPosition = 0; //To be accessed non-atomically by tick(), atomically elsewhere.
	volatile int targetPosition = 0; //To be accessed non-atomically by tick(), atomically elsewhere.
	uint8_t tickCount = 0;
	volatile uint8_t *dirPort, dirMask;
	volatile uint8_t *stepPort, stepMask;
	
	volatile bool stepHigh = false;
};

//Uses TIMER2 (8 bit) to provide timing calls to each stepper.
class _Timer{
	
	public: 
	//Argument is assumed to be a pointer to a class instance.
	void subscribe(void(*wrapper)(void *), void* argument);
	void unsubscribe(void *argument);
	
	_Timer();
	
	void tick(void);
	
	void begin(void);
	
	private:
	
	struct subscriber {
		void(*ftn)(void *);
		void *arg;
	} subscribers[MAX_SUBSCRIBERS_TIMER2];
	
	uint8_t tickCount = 0;
	
} Timer2;

//Implementation - stepper
void stepper::absoluteMove(int newPosition) {
	if( (newPosition > maxPosition) || (newPosition < minPosition) ) return;
	targetPosition = newPosition;
}

void stepper::incrementalMove(int changePositionBy) {
	long temp = targetPosition + changePositionBy;
	if( (temp > maxPosition) || (temp < minPosition) ) return;
	targetPosition = temp;
}

void stepper::setHome(void) {
	waitUntilStopped();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		currentPosition = 0;
	}
}

void stepper::goHome(void) {
	waitUntilStopped();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		targetPosition = 0;
	}
}

void stepper::waitUntilStopped(void) {
	while (!isStopped()){};
}

bool stepper::isStopped(void) {
	bool temp = false;
	
	if(currentPosition == targetPosition) {
		//Catches potentially dirty reads of currentPosition
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
		temp = (currentPosition == targetPosition);
		}
	}
	return temp;
}

void stepper::stop(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		targetPosition = currentPosition;
	}
}

stepper::stepper(uint8_t stepPin, uint8_t dirPin){
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
	
	stepMask = digitalPinToBitMask(stepPin);
	dirMask = digitalPinToBitMask(dirPin);
	
	stepPort = portOutputRegister( digitalPinToPort(stepPin) );
	dirPort = portOutputRegister( digitalPinToPort(dirPin) );
	
	//The lambda function wraps a call to tick() for the correct instance of stepper.
	Timer2.subscribe(
		[](void *target){ ((stepper *)target)->tick(); }	,
		this);
}

stepper::~stepper() {
	Timer2.unsubscribe((void *)this);
}

void stepper::tick() {
	tickCount++;
	uint8_t temp;
	if( !(tickCount % speedDivisor) ) {
		if(stepHigh) {
			*stepPort = *stepPort & ~stepMask;
			tickCount = 0;
			stepHigh = false;
			return;
		}
		
		temp = *dirPort & ~dirMask;
		if(currentPosition > targetPosition) {
			*dirPort = temp | dirMask; 
			*stepPort |= stepMask;
			stepHigh = true;
			currentPosition--;
		}
		
		if(currentPosition < targetPosition) {
			*dirPort = temp;
			*stepPort |= stepMask;
			stepHigh = true;
			currentPosition++;
		}
	tickCount = 0;
	return;
	}
}

//Implementation - Timer2

void _Timer::subscribe(void(*wrapper)(void *), void* argument) {
	int i = 0;
	while(subscribers[i].ftn) i++;
	if(i>=MAX_SUBSCRIBERS_TIMER2) { while(1);} //I mean...
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		subscribers[i].ftn = wrapper;
		subscribers[i].arg = argument;
	}
}

void _Timer::unsubscribe(void *argument) {
	for(int i=0; i < MAX_SUBSCRIBERS_TIMER2; i++) {
		if(subscribers[i].arg = argument) {
			subscribers[i].ftn = 0;
			subscribers[i].arg = 0;
		}
	}
}

void _Timer::tick(void) {
	//One subscriber each tick breaks up the time spent in the ISR over more ticks.
	
	if(tickCount == MAX_SUBSCRIBERS_TIMER2) tickCount = 0;
	
	if(subscribers[tickCount].ftn) (*subscribers[tickCount].ftn)(subscribers[tickCount].arg);
	
	digitalWrite(5, digitalRead(5));
	
	/* //If we wanted every subscriber called every tick.
	for(int i=0; i < MAX_SUBSCRIBERS_TIMER2; i++) {
		if(subscribers[i].ftn) (*subscribers[i].ftn)(subscribers[i].arg);
	}
	*/
}

void _Timer::begin(void) {
	//You'll have to read the datasheet.
	PRR &= ~0b01000000;
	TCCR2A = 0b00000001;
	TCCR2B = 0b00001011;
	OCR2A = 125;
	TIMSK2 = 0b00000001;
}

_Timer::_Timer(){
	begin();
}

ISR(TIMER2_OVF_vect){
	Timer2.tick();
}
