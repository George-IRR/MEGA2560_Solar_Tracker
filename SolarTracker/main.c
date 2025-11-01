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
//#include <stdint.h>
#include <stdbool.h> /* bool */

#include "drivers/adc/ADC.h"
#include "drivers/servo/SERVO.h"
#include "drivers/usart/USART.h"
#include "drivers/twi/TWI.h"
#include "modules/DHT20/DHT20.h"
#include "drivers/usart/usart_buffer.h"
#include "drivers/usart/usart_packet.h"
#include "modules/scheduler/Scheduler.h"

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

/*
 * Layout:
 * ADC12 (Left Top)     |  ADC10 (Right Top)
 * --------------------|--------------------
 * ADC14 (Left Bottom)  |  ADC8 (Right Bottom)
 *
 */
void solarTrack2Axis(void) {	
	const int16_t tolerance = 50; // Deadzone threshold
	
	while (1) {
		// Read LDR values
		uint16_t analogValue0 = ADC_read(8);  
		uint16_t analogValue1 = ADC_read(10); 
		uint16_t analogValue2 = ADC_read(12); 
		uint16_t analogValue3 = ADC_read(14); 

		// Calculate aggregate light for each side
		uint32_t left_light = analogValue2 + analogValue3;
		uint32_t right_light = analogValue0 + analogValue1;
		uint32_t top_light = analogValue1 + analogValue2;
		uint32_t bottom_light = analogValue0 + analogValue3;

		// Calculate differences
		// diff_h > 0	Left is brighter
		// diff_v > 0	Top is brighter
		int32_t diff_h = left_light - right_light;
		int32_t diff_v = top_light - bottom_light;

		uint8_t h_centered = 0;
		uint8_t v_centered = 0;
		
		// angle++ moves Left (towards brighter 'left_light')
		if (diff_h > tolerance) 
		{
			if (PWM4_C_regs.current_angle < PWM4_C_regs.high_limit) 
			{
				PWM4_C_regs.current_angle++;
				sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
			}
		} 
		//angle-- moves Right (towards brighter 'right_light')
		else if (diff_h < -tolerance) 
		{
			if (PWM4_C_regs.current_angle > PWM4_C_regs.low_limit) 
			{
				PWM4_C_regs.current_angle--;
				sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
			}
		} 
		else 
		{
			h_centered = 1; // Horizontal is centered
		}
		
		//angle-- moves Up (towards brighter 'top_light')
		if (diff_v > tolerance) 
		{
			if (PWM4_B_regs.current_angle > PWM4_B_regs.low_limit) 
			{
				PWM4_B_regs.current_angle--;
				sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
			}

		} 
		
		//angle++ moves Down (towards brighter 'bottom_light')
		else if (diff_v < -tolerance) 
		{
			if (PWM4_B_regs.current_angle < PWM4_B_regs.high_limit) 
			{
				PWM4_B_regs.current_angle++;
				sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
			}
		} 
		else 
		{
			v_centered = 1; // Vertical is centered
		}
		
		_delay_ms(17);
	}
}

int main(void)
{
	USART_init(&USART0_regs, 57600);
	USART_init(&USART1_regs, 38400);
	sei();
	TWI_init();
	
	SERVO_init();
	SERVO_Zero(&PWM4_C_regs);
	SERVO_Zero(&PWM4_B_regs);
	printString(&USART1_regs,"Calibration Completed \n");
	solarTrack2Axis();
	while (1) {
// 		goToAngle(&PWM4_C_regs,300);
// 		_delay_ms(1000);
// 
// 		goToAngle(&PWM4_B_regs,130);
// 		_delay_ms(1000);
// 
// 		goToAngle(&PWM4_B_regs,160);
// 		_delay_ms(1000);
// 
// 		goToAngle(&PWM4_B_regs,175);
// 		_delay_ms(1000);
// 
// 
// 		goToAngle(&PWM4_C_regs,160);
// 		_delay_ms(1000);		
// 		
// 		goToAngle(&PWM4_C_regs,0);
// 		_delay_ms(1000);
// 		
// 		goToAngle(&PWM4_C_regs,160);
// 		_delay_ms(1000);
// 		
// 		goToAngle(&PWM4_C_regs,0);
// 		_delay_ms(1000);
				
		process_uart1_bytes(); //example AA 55 01 30 0A 02 1A 2B 82
		process_scheduled_work();
		//blink();
		if (uart1_clear_overflow_flag())
		{
			// handle overflow notification
			blink();
		}
	}
	return 0;
}
