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
#include "drivers/servo/SERVO.h"

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

		uint16_t servoDegrees = (0.29296875) * analogValue0; // (1024/300) * analogValue0
		//uint16_t servoDegrees = 50;
		sendAngle(servoDegrees);
	}
}

