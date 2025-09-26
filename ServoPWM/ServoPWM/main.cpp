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

static inline void initServo(void)
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
	
	OCR4C=(angle*13.33)+1000;
}
#define BLINK_DELAY 500

void blink()
{
	DDRB |= (1 << PB7);
	PORTB |= (1 << PB7);   // LED on
	_delay_ms(BLINK_DELAY);    // Delay (if time_ms is constant or small)
	PORTB &= ~(1 << PB7);  // LED off
	_delay_ms(BLINK_DELAY);
}

int main(void)
{
	initServo();
	//sendAngle(0);

	// 	while (1)
	// 	{
	// 	sendAngle(0);
	// 		for(int i=0; i<=360; i+=5)
	// 		{
	// 			_delay_ms(100);
	// 			sendAngle(i);
	// 		}
	//
	//
	// 	}
	while (1)
	{
	
		blink(500);
		sendAngle(300);
		_delay_ms(1000);

		blink(500);
		blink(500);
		sendAngle(310);
		_delay_ms(1000);

		sendAngle(320);
		_delay_ms(1000);

		sendAngle(330);
		_delay_ms(1000);

		sendAngle(340);
		_delay_ms(1000);

		sendAngle(350);
		_delay_ms(1000);
	}
}

