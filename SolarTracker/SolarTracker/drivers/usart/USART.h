#ifndef USART_H_
#define USART_H_

#define F_CPU 16000000UL

#define RX_BUFFER_SIZE 64

void USART_init(uint32_t baud);

void USART_sendBtye(uint8_t data);

void printString(const char* str);

void printInt(uint32_t value);

unsigned char USART_Receive( void );

void USART_Receive_String( char* string_buffer, uint8_t size);

#endif /* USART_H_ */