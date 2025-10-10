#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 16000000UL

void UART_init(uint32_t baud){
	uint16_t ubrr_value = (F_CPU/(16UL*baud))-1;
	
	UBRR0H = (uint8_t)(ubrr_value >> 8);
	UBRR0L = (uint8_t)(ubrr_value);
	
	UCSR0B = (1<<TXEN0); //TX Enable
	UCSR0B |= (1<<RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); //8 data bits

	
}

void UART_sendBtye(uint8_t data){
	while (!(UCSR0A & (1<<UDRE0))); //if busy
	UDR0 = data;
}

void printString(const char* str){
	while(*str){
		UART_sendBtye(*str++);
	}
}

void printInt(uint32_t value){
	char buffer[12];
	ultoa(value, buffer, 10);
	printString(buffer);
}
