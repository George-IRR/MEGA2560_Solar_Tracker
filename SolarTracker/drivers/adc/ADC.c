/*
 * ADC.cpp
 *
 * Created: 9/26/2025 9:23:13 PM
 *  Author: George
 */ 

#include <avr/io.h>

void ADC_init(void){
	ADMUX = (1<<REFS0); //Reference voltage 5V, ADC channel 0
	
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1); //Enable ADC
}

uint16_t ADC_read(uint8_t channel)
{
	ADMUX = (ADMUX & 0b11110000) | (channel & 0b00001111);
	ADCSRA |= (1<<ADSC); //ADC Start Conversion
	while(ADCSRA & (1<<ADSC)); //wait until ADC reading is complete to perform the next one
	return ADC;
}