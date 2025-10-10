/*
 * ServoPWM.cpp
 *
 * Created: 9/18/2025 6:39:15 PM
 * Author : George
 * 
 * avrdude -c wiring -p atmega2560 -P COM4 -b 115200 -D -U flash:w:"MEGA2560_Baremetal\SolarTracker\SolarTracker\Debug\SolarTracker.hex":i
 *
 */ 

#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>     /* abs */
#include <string.h>		/* strcmp */

#include "drivers/adc/ADC.h"
#include "drivers/servo/SERVO.h"
#include "drivers/usart/USART.h"

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

void solarTrack(){
	SERVO_init();
	ADC_init();
	sendAngle(angle);
	while(1){
		analogValue0 = ADC_read(0);
		analogValue1 = ADC_read(1);


		if(analogValue0>analogValue1) 
		{
			if(angle<MAX_SERVO_DEGREE) sendAngle(angle++);
		
		}
		else if(analogValue0<analogValue1)
		{
			if(angle>0) sendAngle(angle--);
			
		}
		else if(abs(analogValue0-analogValue1)<400) //tolerance
		{
			blink();
		}

		_delay_ms(10);
		
	}
}


int main(void)
{
	USART_init(9600);
	char rx_buffer[32];

	while (1)
	{
		USART_Receive_String(rx_buffer, sizeof(rx_buffer));
		
		//both NL & CR
		if (strcmp(rx_buffer, "/blink")  == 0) 
			blink();
	}
}