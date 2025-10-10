#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 16000000UL

void USART_init(uint32_t baud){
	uint16_t ubrr_value = (F_CPU/(16UL*baud))-1;
	
	UBRR0H = (uint8_t)(ubrr_value >> 8);
	UBRR0L = (uint8_t)(ubrr_value);
	
	UCSR0B = (1<<TXEN0); //TX Enable
	UCSR0B |= (1<<RXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); //8 data bits

	
}

void USART_sendBtye(uint8_t data){
	while (!(UCSR0A & (1<<UDRE0))); //if busy
	UDR0 = data; // 8 bit register
}

void printString(const char* str){
	while(*str){
		USART_sendBtye(*str++);
	}
}

void printInt(uint32_t value){
	char buffer[12];
	ultoa(value, buffer, 10);
	printString(buffer);
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) );
	
	/* Get and return received data from buffer */
	return UDR0;
}


/* Function usage:
char rx_buffer[32];
USART_Receive_String(rx_buffer, sizeof(rx_buffer));
*/
void USART_Receive_String( char* string_buffer, uint8_t size)
{
	// Don't use local array
	//char string_buffer[RX_BUFFER_SIZE]; 
	uint8_t i = 0;
	
	while(i < size - 1) //leave the last char for \0 
	{
		unsigned char received_char = USART_Receive() ;
		if(received_char == '\n' || received_char == '\r')
			break;
		string_buffer[i++]= received_char;
	}
	string_buffer[i] = '\0';
}