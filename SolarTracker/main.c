/*
 * ServoPWM.cpp
 *
 * Created: 9/18/2025 6:39:15 PM
 * Author : George
 *
 * avrdude -c wiring -p atmega2560 -P COM4 -b 115200 -D -U flash:w:"MEGA2560_Baremetal\SolarTracker\Debug\SolarTracker.hex":i
 *
 */ 

#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>     /* abs */
#include <string.h>		/* strcmp */
#include <avr/interrupt.h> /* sreg reg*/
#include <stdint.h>
#include <stdbool.h> /* bool */

#include "drivers/adc/ADC.h"
#include "drivers/servo/SERVO.h"
#include "drivers/usart/USART.h"
#include "drivers/twi/TWI.h"
#include "modules/DHT20/DHT20.h"
#include "drivers/usart/usart_buffer.h"
#include "drivers/usart/usart_packet.h"
#include "modules/scheduler/Scheduler.h"
#include "modules/solar_track/solar_track.h"
#include "drivers/timer/timer.h"

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

//TEMPORARY COMMENT
// uint8_t rx1_index=0;
// char rx1_buf[RX_BUF_SIZE];
// 
// ISR(USART1_RX_vect) //Interrupt Bluetooth receive
// {
// 	uint8_t data = UDR1;  // Read to clear the RXC1 flag
// 
// 	// Handle incoming data
// 	if (rx1_index < RX_BUF_SIZE - 1) 
// 	{
// 		if (data == '\r' || data == '\n') 
// 		{
// 			if( strncmp(rx1_buf, "+CONNECTING<<", 13) == 0 )
// 			{
// 				printString(&USART0_regs, "Bluetooth Connected to Device \n");
// 				//Later add the password and maybe connect to just one address
// 				printString(&USART0_regs, rx1_buf+13);
// 				// OR
// 				//char *Connected_BT_Address = rx1_buf+13;
// 				//printString(&USART0_regs, rx1_buf+13);
// 				printString(&USART0_regs, "\n");
// 				
// 				// Clear buffer and reset index
// 				memset(rx1_buf, 0, sizeof(rx1_buf));
// 				rx1_index = 0;
// 			}
// 			else if ( data == '\r' || data == '\n' )
// 			{
// 				// Process the received line
// 				printString(&USART0_regs, "Received: ");
// 				printString(&USART0_regs, rx1_buf);
// 				printString(&USART0_regs, "\n");
// 
// 				// Clear buffer and reset index
// 				memset(rx1_buf, 0, sizeof(rx1_buf));
// 				rx1_index = 0;
// 			}
// 		} 
// 		else 
// 		{
// 			// Store data in buffer
// 			rx1_buf[rx1_index++] = data;
// 		}
// 	} 
// 	else 
// 	{
// 		// Buffer overflow handling (optional)
// 		printString(&USART0_regs, "Buffer overflow\n");
// 		memset(rx1_buf, 0, sizeof(rx1_buf));
// 		rx1_index = 0;
// 	}
// }

int main(void)
{
	USART_init(&USART0_regs, 57600);
	USART_init(&USART1_regs, 38400);
	sei();
	TWI_init();
	
	ADC_init();
	SERVO_init();
	SERVO_Zero(&PWM4_C_regs);
	SERVO_Zero(&PWM4_B_regs);
	printString(&USART1_regs,"Calibration Completed \n");
	Timer1_Init_Polling();
	
    while (1) 
    {
        if ( ( TIFR1 & (1 << OCF1A) ) && !track_manual_block)
        {
            TIFR1 = (1 << OCF1A);
            solarTrack2Axis_step();
        }
        
        process_uart1_bytes(); //example AA 55 01 30 0A 02 1A 2B 82	
		process_scheduled_work();
		if (uart1_clear_overflow_flag())
		{
			// handle overflow notification
			blink();
		}
	}
}

