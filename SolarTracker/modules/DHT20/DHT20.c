#define F_CPU 16000000UL 
#include <avr/io.h>
#include "../../drivers/twi/TWI.h"
#include <util/delay.h>

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