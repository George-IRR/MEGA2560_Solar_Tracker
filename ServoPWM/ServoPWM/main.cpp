/*
 * ServoPWM.cpp
 *
 * Created: 9/18/2025 6:39:15 PM
 * Author : George
 * 
 */ 

#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h>

#include "ADC.h"

const uint16_t MAX_SERVO_DEGREE = 300; // type safe

static inline void SERVO_init(void)
{
	//sets Timer4 registers
	TCCR4A |= (1<<WGM41);
	TCCR4B |= (1<<WGM42) | (1<<WGM43);
	
	/*
	Set clock for Timer4 2MHz
	Prescaler clk/8 
	Prescaler clk = 16MHZ
	
	each timer tick = 1/2MHz = 0.5 us
	*/
	TCCR4B |= (1<<CS41);
	
	/*
	sets timer ticks to 40.000
	40.000 * 0.5us = 20ms (50Hz) -> 
	20ms the frequency the servo works
	*/
	ICR4=40000;
	
	TCCR4A |= (1<<COM4C1);
	DDRH |= (1<<PH5);
}

void sendAngle(uint16_t angle)
{
	//clamp values
	if(angle < 0) angle = 0;
	if(angle > MAX_SERVO_DEGREE) angle = MAX_SERVO_DEGREE;
	
	//0.5ms = 1000 ticks, 2.5ms = 5000 ticks
	OCR4C = (angle * ((5000-1000)/MAX_SERVO_DEGREE)) + 1000;
}

#define BLINK_DELAY 500
void blink()
{
	DDRB |= (1 << PB7);
	PORTB |= (1 << PB7);	// LED on
	_delay_ms(BLINK_DELAY);    // Delay (BLINK_DELAY is constant or small)
	PORTB &= ~(1 << PB7);	// LED off
	_delay_ms(BLINK_DELAY);
}

uint16_t analogValue0 = 0;

int main(void)
{
// 	while(1){
// 		SERVO_init();
// 		sendAngle(0);
// 		_delay_ms(2500);
// 		blink();
// 
// 		sendAngle(250);
// 		_delay_ms(2500);
// 		blink();
// 		
// 		sendAngle(300);
// 		_delay_ms(2500);
// 		blink();		
// 	}
	sendAngle(0);
	while(1){
		SERVO_init();
		ADC_init();
		
		analogValue0 = ADC_read();
		//_delay_ms(5);
// 		if (analogValue0>500) sendAngle(300);
// 		else sendAngle(0);

		uint16_t servoDegrees = (0.29296875) * analogValue0;
		//uint16_t servoDegrees = 50;
		sendAngle(servoDegrees);
	}
}

