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
#include "drivers/twi/TWI.h"


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
	sendAngle(&PWM4_C_regs, angle);
	while(1){
		analogValue0 = ADC_read(0);
		analogValue1 = ADC_read(1);


		if(analogValue0>analogValue1) 
		{
			if(angle<MAX_SERVO_DEGREE) sendAngle(&PWM4_C_regs, angle++);
		
		}
		else if(analogValue0<analogValue1)
		{
			if(angle>0) sendAngle(&PWM4_C_regs, angle--);
			
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
			char *BT_COMMAND = strstr(rx0_buf,"AT"); 
			if(BT_COMMAND != NULL)
			{				
				//send command & message
				printString(&USART1_regs, BT_COMMAND);
				
				printString(&USART0_regs, "\n Command sent: ");
				printString(&USART0_regs, BT_COMMAND);	
				printString(&USART0_regs, "\n");			
				/* //DEBUG, if you leave 19200 baud it will not show the whole UART response from BT module
				printString(&USART0_regs, "\n Command sent: ");
				printString(&USART0_regs, BT_COMMAND);
				printString(&USART0_regs, "\n Found from the array: ");
				printString(&USART0_regs, rx0_buf);
				printString(&USART0_regs, "\n");
				*/
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

void getDHT20_Data(uint8_t data[7])
{
	//Trigger measurement
	TWI_start();
	TWI_write(0x70);  // DHT20 write address (0x38 << 1)
	TWI_write(0xAC);  // Trigger command
	TWI_write(0x33);
	TWI_write(0x00);
	TWI_stop();
	
	_delay_ms(80);    // Wait for measurement (minimum 80ms)
	
	//Read data
	TWI_start();
	TWI_write(0x71);  // DHT20 read address (0x38<<1 | 1)
	
	data[0] = TWI_read_ack();   // Status byte
	data[1] = TWI_read_ack();   // Humidity[19:12]
	data[2] = TWI_read_ack();   // Humidity[11:4]
	data[3] = TWI_read_ack();   // Hum[3:0]|Temp[19:16]
	data[4] = TWI_read_ack();   // Temperature[15:8]
	data[5] = TWI_read_ack();   // Temperature[7:0]
	data[6] = TWI_read_nack();  // CRC-8
	
	TWI_stop();
}

//
int main(void)
{
	USART_init(&USART0_regs, 57600);
	USART_init(&USART1_regs, 38400);
	SERVO_init();
	while(1)
	{
		sendAngle(&PWM4_C_regs,0);
		_delay_ms(200);
		sendAngle(&PWM4_B_regs,130);
		
		_delay_ms(1000);
		
		sendAngle(&PWM4_C_regs,90);
		_delay_ms(200);
		sendAngle(&PWM4_B_regs,140);
		
		_delay_ms(1000);

		sendAngle(&PWM4_C_regs,180);
		_delay_ms(200);
		sendAngle(&PWM4_B_regs,150);
		
		_delay_ms(1000);

		sendAngle(&PWM4_C_regs,300);
		_delay_ms(200);
		sendAngle(&PWM4_B_regs,175);
		
		_delay_ms(1000);
	}
}


// DHT20 temp and moisture data send to web server
// working for dashboard v0.0.1

// int main(void)
// {
// 	USART_init(&USART0_regs, 57600);
// 	USART_init(&USART1_regs, 38400);
// 	
// 	TWI_init();
// 	uint8_t data[7];
// 	_delay_ms(100);  // Wait for DHT20 power-up
// 	
// 	while(1)
// 	{
// 		getDHT20_Data(data);
// // 		
// // 		// Print results
// // 		printString(&USART1_regs, "Status: 0x");
// // 		printHex(&USART1_regs, data[0]);
// // 		printString(&USART1_regs, "\r\n");
// // 		
// 		// For now, print raw bytes
// 		//printString(&USART1_regs, "Data: ");
// 		for(int i = 0; i < 7; i++) {
// 			printHex(&USART1_regs, data[i]);
// 			//printString(&USART1_regs, " ");
// 		}
// 		printString(&USART1_regs, "\r\n");
// 		/*
// 		// Extract 20-bit values
// 		uint32_t raw_humidity = ((uint32_t)data[1] << 12) |
// 								((uint32_t)data[2] << 4) |
// 								((uint32_t)data[3] >> 4);
// 
// 		uint32_t raw_temp = (((uint32_t)data[3] & 0x0F) << 16) |
// 								((uint32_t)data[4] << 8) |
// 								((uint32_t)data[5]);
// 
// 		// Convert to actual values
// 		float humidity = (raw_humidity * 100.0) / 1048576.0;
// 		float temperature = ((raw_temp * 200.0) / 1048576.0) - 50.0;
// 	
// 		
// 
// 		
// 		printString(&USART1_regs, "\n Humidity: \n");
// 		printFloat(&USART1_regs, humidity);
// 		printString(&USART1_regs, "\n Temperature: \n");
// 		printFloat(&USART1_regs, temperature);
// 
// 	
// 		printString(&USART1_regs, "\r\n\r\n");
// 		*/
// 		_delay_ms(2000);  // Read every 2 seconds
// 	}
// }