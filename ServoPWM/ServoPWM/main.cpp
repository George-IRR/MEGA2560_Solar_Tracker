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
#include <stdlib.h>     /* abs */

#include "drivers/adc/ADC.h"
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
uint16_t analogValue1 = 0;

uint16_t angle = 180;
int main(void)
{
	SERVO_init();
	ADC_init();
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
	sendAngle(angle);
	while(1){
		
		
		
		analogValue0 = ADC_read(0);
		analogValue1 = ADC_read(1);


		if(analogValue0>analogValue1) //&& angle <= MAX_SERVO_DEGREE && angle >= 0) 
		{
			if(angle<MAX_SERVO_DEGREE) sendAngle(angle++);
		
		}
		else if(analogValue0<analogValue1)// && angle >= 0 && angle <= MAX_SERVO_DEGREE) 
		{
			if(angle>0) sendAngle(angle--);
			
		}
		else if(abs(analogValue0-analogValue1)<400) //tolerance
		{
			blink();
		}

// 		else if (!(angle >= 0 && angle <= MAX_SERVO_DEGREE))
// 		{
// 			blink();
// 		}
		_delay_ms(10);
		
		
// 		do
// 		{
// 			angle++;
// 			sendAngle(angle);
// 		}
// 		while(analogValue0>=analogValue1);
// 		
// 		do
// 		{
// 			angle--;
// 			sendAngle(angle);
// 					analogValue0 = ADC_read(0);
// 					analogValue1 = ADC_read(1);
// 		}
// 		while(analogValue0<=analogValue1);
		
		
		
		

		//for(uint16_t i=200;i>=analogValue1;i++) sendAngle(i);
		//_delay_ms(100);
// 		if (analogValue0>500) sendAngle(300);
// 		else sendAngle(0);



		//uint16_t servoDegrees = (0.29296875) * analogValue0; // (1024/300) * analogValue0
		//uint16_t servoDegrees = 50;
		//sendAngle(servoDegrees);
	}
}

