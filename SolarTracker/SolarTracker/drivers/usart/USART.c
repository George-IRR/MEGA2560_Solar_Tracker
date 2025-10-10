#include <avr/io.h>
#include <stdlib.h>
#include "USART.h"
#define F_CPU 16000000UL

USART_t USART0_regs = {
	&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0,
	TXEN0, RXEN0, UCSZ00, UCSZ01, UDRE0, RXC0
};
USART_t USART1_regs = {
	&UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1,
	TXEN1, RXEN1, UCSZ10, UCSZ11, UDRE1, RXC1
};
USART_t USART2_regs = {
	&UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2,
	TXEN2, RXEN2, UCSZ20, UCSZ21, UDRE2, RXC2
};
USART_t USART3_regs = {
	&UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3,
	TXEN3, RXEN3, UCSZ30, UCSZ31, UDRE3, RXC3
};

void USART_init(USART_t *usart, uint32_t baud){
	uint16_t ubrr_value = (F_CPU/(16UL*baud))-1;
	
	*(usart->ubrrh) = (uint8_t)(ubrr_value >> 8);
	*(usart->ubrrl) = (uint8_t)(ubrr_value);
	
	*(usart->ucsrb)  = (1<<usart->txen);  //TX Enable
	*(usart->ucsrb) |= (1<<usart->rxen);  //RX Enable
	*(usart->ucsrc) = (1<<usart->ucsz1) | (1<<usart->ucsz0); //8 data bits

	
}

void USART_sendBtye(USART_t *usart, uint8_t data){
	while (!( *(usart->ucsra) & (1<< usart->udre) )); //if busy
	*(usart->udr)= data; // 8 bit register
}

void printString(USART_t *usart, const char* str){
	while(*str){
		USART_sendBtye(usart, *str++);
	}
}

void printInt(USART_t *usart, uint32_t value){
	char buffer[12];
	ultoa(value, buffer, 10);
	printString(usart, buffer);
}

unsigned char USART_Receive(USART_t *usart)
{
	/* Wait for data to be received */
	while ( !( *(usart->ucsra) & (1 << usart->rxc) ));
	
	/* Get and return received data from buffer */
	return *(usart->udr);
}


/* Function usage:
char rx_buffer[32];
USART_Receive_String(USART0_regs, rx_buffer, sizeof(rx_buffer));
*/
void USART_Receive_String(USART_t *usart, char* string_buffer, uint8_t size)
{
	// Don't use local array
	//char string_buffer[RX_BUFFER_SIZE]; 
	uint8_t i = 0;
	
	while(i < size - 1) //leave the last char for \0 
	{
		unsigned char received_char = USART_Receive(usart) ;
		if(received_char == '\n' || received_char == '\r')
			break;
		string_buffer[i++]= received_char;
	}
	string_buffer[i] = '\0';
}