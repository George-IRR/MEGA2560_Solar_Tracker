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
void solarTrack2Axis(void) 
{
	const int16_t tolerance = 50; // Deadzone threshold
	
	// Use #define for array size instead of const
	#define AVG_SAMPLES 4
	
	// Circular buffers for averaging
	int32_t diff_h_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	int32_t diff_v_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	uint8_t buffer_index = 0;
	uint8_t samples_collected = 0;
	
	while (1) 
	{
		// Read LDR values
		uint16_t analogValue0 = ADC_read(12);
		uint16_t analogValue1 = ADC_read(13);
		uint16_t analogValue2 = ADC_read(14);
		uint16_t analogValue3 = ADC_read(15);

		// Calculate aggregate light for each side
		uint32_t left_light = analogValue2 + analogValue3;
		uint32_t right_light = analogValue0 + analogValue1;
		uint32_t top_light = analogValue1 + analogValue2;
		uint32_t bottom_light = analogValue0 + analogValue3;

		// Calculate differences
		// diff_h > 0	Left is brighter
		// diff_v > 0	Top is brighter
		int32_t diff_h_raw = left_light - right_light;
		int32_t diff_v_raw = top_light - bottom_light;
		
		// Store in circular buffer
		diff_h_buffer[buffer_index] = diff_h_raw;
		diff_v_buffer[buffer_index] = diff_v_raw;
		buffer_index = (buffer_index + 1) % AVG_SAMPLES;
		
		// Track how many samples we've collected (up to AVG_SAMPLES)
		if (samples_collected < AVG_SAMPLES) 
		{
			samples_collected++;
		}
		
		// Calculate averages
		int32_t diff_h_sum = 0;
		int32_t diff_v_sum = 0;
		for (uint8_t i = 0; i < samples_collected; i++) 
		{
			diff_h_sum += diff_h_buffer[i];
			diff_v_sum += diff_v_buffer[i];
		}
		int32_t diff_h = diff_h_sum / samples_collected;
		int32_t diff_v = diff_v_sum / samples_collected;
		
		// Only make movements after we have enough samples
		if (samples_collected >= AVG_SAMPLES) 
		{
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
		}
		
		_delay_ms(12); //17 -> slow  12 -> med  7 ->fast
	}
	
	#undef AVG_SAMPLES
}

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
	
	while (1) {
		solarTrack2Axis();
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
