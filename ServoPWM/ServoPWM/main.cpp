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
	OCR4C=(angle*22.22)+1000;
}
int main(void)
{
	initServo();
	sendAngle(0);
	
	while (1)
	{
	sendAngle(0);
		for(int i=0; i<=200; i+=10)
		{
			_delay_ms(500);
			sendAngle(i);
		}


	}

}

