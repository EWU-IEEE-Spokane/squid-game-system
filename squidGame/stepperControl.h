//Header
#pragma once
#include <stdint.h>
#include <limits.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <wiring.c>

void stepper_error(const char *);

class stepper {
	public: 
	void absoluteMove(int newPosition);			//invalid positions silently ignored...
	void incrementalMove(int changePositionBy); //invalid positions silently ignored...
	void setHome(void); //Sets the current position as home
	void goHome(void); //Sets the target position to home
	void stop(void);
	
	int getCurrentPosition(void) const;
	void waitUntilStopped(void) const;//Busy waits until the stepper motor is idle.
	bool isStopped(void) const;
	
	volatile uint8_t speedDivisor = 1;
	volatile int maxPosition = INT_MAX;
	volatile int minPosition = INT_MIN;
	
	stepper(uint8_t stepPin, uint8_t dirPin);
	~stepper();
	
	void tick(void);
	private:
	stepper(); //No default constructor
	volatile int currentPosition = 0;	//To be accessed non-atomically by tick(), atomically elsewhere.
	volatile int targetPosition = 0;	//To be accessed non-atomically by tick(), atomically elsewhere.
	uint8_t tickCount = 0;
	volatile uint8_t *dirPort;
	uint8_t dirMask;
	volatile uint8_t *stepPort;
	uint8_t stepMask;
	
	bool stepHigh = false;
};

//Uses TIMER2 (8 bit) to provide timing calls to each stepper.
class Timer_{
	
	public: 
	
	//Argument is assumed to be a pointer to a class instance.
	void subscribe(void(*wrapper)(stepper *), stepper* argument);
	void unsubscribe(stepper *argument);
	
	Timer_();
	
	void tick(void);
	void begin(void) const;
	
	static const uint8_t maxSubscribers = 4;
	
	private:
	
	struct subscriber {
		void(*ftn)(stepper *) = 0;
		stepper *arg = 0;
	} subscribers[maxSubscribers];
	
	uint8_t tickCount = 0;
	
} Timer2;

//End of what would be the header

//Implementation - stepper
void stepper::absoluteMove(int newPosition) {
	if( (newPosition > maxPosition) || (newPosition < minPosition) ) return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		targetPosition = newPosition;
	}
}

void stepper::incrementalMove(int changePositionBy) {
	long temp = targetPosition + changePositionBy;
	if( (temp > maxPosition) || (temp < minPosition) ) return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		targetPosition = temp;
	}
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

void stepper::stop(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		targetPosition = currentPosition;
	}
}

int stepper::getCurrentPosition(void) const{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return currentPosition;
	}
	__builtin_unreachable(); //Compiler hint to quiet warning.
}

void stepper::waitUntilStopped(void) const{
	while (!isStopped()){};
}

bool stepper::isStopped(void) const{
	bool temp = false;
	
	if(currentPosition == targetPosition) {
		//Catches potentially dirty reads of currentPosition
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
		temp = (currentPosition == targetPosition);
		}
	}
	return temp;
}

stepper::stepper(uint8_t stepPin, uint8_t dirPin){
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
	
	stepMask = digitalPinToBitMask(stepPin);
	dirMask = digitalPinToBitMask(dirPin);
	
	stepPort = portOutputRegister( digitalPinToPort(stepPin) );
	dirPort = portOutputRegister( digitalPinToPort(dirPin) );
	
	//The lambda function wraps a call to tick() for the correct instance of stepper.
	Timer2.subscribe([](stepper *target){ (target)->tick();},this);
}

stepper::~stepper() {
	Timer2.unsubscribe(this);
}

//Per DRV8825 datasheet: SLVSA73F – APRIL 2010 – REVISED JULY 2014
//See sec 7.6: Timing requirements, Item 4 on figure 1 & associated table
//Minimum time between changing DIR and changing STEP is 650ns. 
//11 clock cycles here is ~687 ns
/*	The the placement of the useful work of updating stepHigh and currentPosition between the
	writes to dirPort and stepPort causes ~15 asm instructions and ~26 clock cycles
	of delay. Compiler optomization could reduce this to ~15 clock cycles if addresses are all
	known at compile time. The use of barriers the compiler can't reorder around  
	for good measure should be good enough, but don't modify this section without 
	examining the generated assembly and verifying that there is sufficient time inbetween 
	* the st (store) instructions associated with the port writes. */
//Also verified the times between writing dir & writing step with oscilloscope: ~1.62us
	
void stepper::tick(void) {
#define REORDERING_BARRIER() asm volatile ("" ::: "memory")

	tickCount++;
	if( !(tickCount % speedDivisor) ) {
		if(stepHigh) {
			*stepPort = *stepPort & ~stepMask;
			tickCount = 0;
			stepHigh = false;
			return;
		}
		
		uint8_t temp = *dirPort & ~dirMask;
		if(currentPosition > targetPosition) {
			*dirPort = temp | dirMask; 
			REORDERING_BARRIER();
			stepHigh = true;
			currentPosition--;
			REORDERING_BARRIER();
			*stepPort |= stepMask;
			return;
		}
		
		if(currentPosition < targetPosition) {
			*dirPort = temp;
			REORDERING_BARRIER();
			stepHigh = true;
			currentPosition++;
			REORDERING_BARRIER();
			*stepPort |= stepMask;
			return;
		}
	tickCount = 0;
	}
#undef REORDERING_BARRIER
}


//Implementation - Timer2

//"Why aren't you using std::function and std::array?"
//The STL is not available on the platform.

void Timer_::subscribe(void(*wrapper)(stepper *), stepper* argument) {
	//Find an available slot in subscribers[]
	int i = 0;
	while(subscribers[i].ftn && i<maxSubscribers) i++;
	if(i==maxSubscribers) stepper_error("Exceeded maximum controllable stepper outputs.");
	
	//Place the new subscriber in that slot
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		subscribers[i].ftn = wrapper;
		subscribers[i].arg = argument;
	}
}

void Timer_::unsubscribe(stepper *argument) {
	for(int i=0; i < maxSubscribers; i++) {
		if(subscribers[i].arg == argument) {
			subscribers[i].ftn = 0;
			subscribers[i].arg = 0;
		}
	}
}

void Timer_::tick(void) {
	//One subscriber each tick breaks up the time spent in the ISR over more ticks.
	tickCount++;
	
	if(tickCount == maxSubscribers) tickCount = 0;
	
	if(subscribers[tickCount].ftn) (*subscribers[tickCount].ftn)(subscribers[tickCount].arg);
}

void Timer_::begin(void) const{
	//Configures timer2 to trigger the TIMER2_OVF_vect ISR at a rate of ~exactly 64khz. 
	//Ultimately this results in each stepper::tick() being called at a rate of 16khz
	
	//https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf 
	//rev:		7810D-AVR-01/15
	PRR &= ~0b01000000; // Sec 17.2, 9.10, 9.11.3  | Enable power to timer2
	TCCR2A = 0b00000011; // Sec 17.11.1, 17.11.2 | WGM20, WGM21, WGM22 bits, between TCCR2A, TCCR2B, set waveform generation mode to
	//Fast PWM Mode with OCRA as timer TOP value. Counts up to TOP, then triggers TIMER2_OVF_vect and restarts
	//Other bits not used, as we're not using the PWM outputs
	TCCR2B = 0b00001010; // Sec 17.11.2  | WGM22, Also divide clock by 32 (see table 17-9)
	OCR2A = 31; // TOP Value
	TIMSK2 = 0b00000001; // Sec 17.11.6 | Enables TIMER2_OVF_vect
}

Timer_::Timer_(){
	begin();
}

ISR(TIMER2_OVF_vect){
	Timer2.tick();
}

void stepper_error(const char *errMsg) {
	Serial.begin(9600);
	while(1) {
		Serial.println(errMsg);
		Serial.flush();
	}
}
