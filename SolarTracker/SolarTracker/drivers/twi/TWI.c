#include <avr/io.h>


void TWI_init(void)
{
	TWSR = 0x00;  // No prescaler (bits 1-0 = 00)
	
	/*
	SCL_frequency = F_CPU / (16 + 2 * TWBR * Prescaler)
	400000 = 16000000 / (16 + 2 * TWBR * 1)
	TWBR = 12 Fast
	
	100000 = 16000000 / (16 + 2 * TWBR * 1)
	TWBR = 72 Standard
	 */
	TWBR = 72;
	
	TWCR = (1 << TWEN);  // Enable TWI hardware
}

void TWI_start(void)
{
	// Set: TWINT (clear flag), TWSTA (start), TWEN (enable)
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	
	// Wait until START is sent (TWINT goes back to 1)
	while (!(TWCR & (1 << TWINT)));
}

void TWI_stop(void)
{
	// Set: TWINT (clear flag), TWSTO (stop), TWEN (enable)
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	
	// STOP executes immediately, no need to wait
}

void TWI_write(uint8_t data)
{
	TWDR = data;  // Load data into data register
	
	// Set: TWINT (start transmission), TWEN (enable)
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	// Wait until byte is sent (TWINT goes back to 1)
	while (!(TWCR & (1 << TWINT)));
}

uint8_t TWI_read_ack(void)
{
	// Set: TWINT (start receive), TWEA (send ACK), TWEN (enable)
	TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
	
	// Wait until byte is received
	while (!(TWCR & (1 << TWINT)));
	
	return TWDR;  // Return received data
}

uint8_t TWI_read_nack(void)
{
	// Set: TWINT (start receive), TWEN (enable)
	// Note: NO TWEA = sends NACK
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	// Wait until byte is received
	while (!(TWCR & (1 << TWINT)));
	
	return TWDR;  // Return received data
}