#include <avr/io.h>

void ADC_init(void){
	ADMUX = (1<<REFS0); // Reference voltage AVCC (5V)
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); // Enable ADC, prescaler 64
	ADCSRB = 0; // Initialize ADCSRB
}

uint16_t ADC_read(uint8_t channel){
	// Mask to keep only the lower 3 bits for ADMUX (0-7)
	ADMUX = (ADMUX & 0xE0) | (channel & 0x07);
	
	// Set or clear MUX5 bit in ADCSRB based on channel
	if (channel >= 8) {
		ADCSRB |= (1 << MUX5);  // Set MUX5 for channels 8-15
		} else {
		ADCSRB &= ~(1 << MUX5); // Clear MUX5 for channels 0-7
	}
	
	// Start conversion
	ADCSRA |= (1<<ADSC);
	
	// Wait until conversion is complete
	while(ADCSRA & (1<<ADSC));
	
	return ADC;
}