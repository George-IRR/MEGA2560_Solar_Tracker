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
#include <avr/interrupt.h> /* sreg reg*/

#include "drivers/adc/ADC.h"
#include "drivers/servo/SERVO.h"
#include "drivers/usart/USART.h"


#define BLINK_DELAY 500
#define BLINK_DELAY_short 50
void blink()
{
	DDRB |= (1 << PB7);
	PORTB |= (1 << PB7);	// LED on
	_delay_ms(BLINK_DELAY);    // Delay (BLINK_DELAY is constant or small)
	PORTB &= ~(1 << PB7);	// LED off
	_delay_ms(BLINK_DELAY);
}
void blink_short()
{
	DDRB |= (1 << PB7);
	PORTB |= (1 << PB7);	// LED on
	_delay_ms(BLINK_DELAY_short);    // Delay (BLINK_DELAY is constant or small)
	PORTB &= ~(1 << PB7);	// LED off
	_delay_ms(BLINK_DELAY_short);
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

#define RX_BUF_SIZE 64
uint8_t rx0_index=0;
char rx0_buf[RX_BUF_SIZE];

ISR(USART0_RX_vect) //Interrupt PC Serial receive
{
	uint8_t data = UDR0;  // Read to clear the RXC1 flag

	// Send commands to Bluetooth module
	if (rx0_index < RX_BUF_SIZE - 1) 
	{
		if (data == '\r' || data == '\n') 
		{
			if(rx0_buf[0]=='A' && rx0_buf[1]=='T' && rx0_buf[2]=='+')
			{				
				//send command & message
				printString(&USART1_regs, rx0_buf);
				
				// Clear buffer and reset index
				memset(rx0_buf, 0, sizeof(rx0_buf));
				rx0_index = 0;
			}
			
		}	
		else 
		{
			// Store data in buffer
			rx0_buf[rx0_index++] = data;
		}
	} 
	else 
	{
		// Buffer overflow handling (optional)
		printString(&USART0_regs, "Buffer overflow\n");
		memset(rx0_buf, 0, sizeof(rx0_buf));
		rx0_index = 0;
	}
}

uint8_t rx1_index=0;
char rx1_buf[RX_BUF_SIZE];

ISR(USART1_RX_vect) //Interrupt Bluetooth receive
{
	uint8_t data = UDR1;  // Read to clear the RXC1 flag

	// Handle incoming data
	if (rx1_index < RX_BUF_SIZE - 1) 
	{
		if (data == '\r' || data == '\n') 
		{
// 			size_t len = strlen(rx0_buf);
// 			if (len > 0 && (rx0_buf[len - 1] == '\n' || rx0_buf[len - 1] == '\r')) {
// 				rx0_buf[len - 1] = '\0';
// 			}

			if( strncmp(rx1_buf, "+CONNECTING<<", 13) == 0 )
			{
				printString(&USART0_regs, "Bluetooth Connected to Device \n");
				//Later add the password and maybe connect to just one address
				printString(&USART0_regs, rx1_buf+13);
				// OR
				//char *Connected_BT_Address = rx1_buf+13;
				//printString(&USART0_regs, rx1_buf+13);
				printString(&USART0_regs, "\n");
				
				// Clear buffer and reset index
				memset(rx1_buf, 0, sizeof(rx1_buf));
				rx1_index = 0;
			}
			else if ( data == '\r' || data == '\n' )
			{
				// Process the received line
				printString(&USART0_regs, "Received: ");
				printString(&USART0_regs, rx1_buf);
				printString(&USART0_regs, "\n");

				// Clear buffer and reset index
				memset(rx1_buf, 0, sizeof(rx1_buf));
				rx1_index = 0;
			}
		} 
		else 
		{
			// Store data in buffer
			rx1_buf[rx1_index++] = data;
		}
	} 
	else 
	{
		// Buffer overflow handling (optional)
		printString(&USART0_regs, "Buffer overflow\n");
		memset(rx1_buf, 0, sizeof(rx1_buf));
		rx1_index = 0;
	}
}

int main(void)
{
	USART_init(&USART0_regs, 19200);
	USART_init(&USART1_regs, 9600);

	while(1)
	{
		
	}
}