#ifndef USART_H_
#define USART_H_

typedef struct {
	volatile uint8_t *ubrrh;
	volatile uint8_t *ubrrl;
	volatile uint8_t *ucsra;
	volatile uint8_t *ucsrb;
	volatile uint8_t *ucsrc;
	volatile uint8_t *udr;
	uint8_t txen;
	uint8_t rxen;
	uint8_t ucsz0;
	uint8_t ucsz1;
	uint8_t udre;
	uint8_t rxc;
	uint8_t rxcie;
} USART_t;


extern USART_t USART0_regs;

extern USART_t USART1_regs;

extern USART_t USART2_regs;

extern USART_t USART3_regs;

#define F_CPU 16000000UL

#define RX_BUFFER_SIZE 64


void USART_init   (USART_t *usart, uint32_t baud);

void USART_sendBtye(USART_t *usart, uint8_t data);

void printString  (USART_t *usart, const char* str);

void printInt     (USART_t *usart, uint32_t value);

unsigned char USART_Receive       (USART_t *usart);

void USART_Receive_String(USART_t *usart, char* string_buffer, uint8_t size);

#endif /* USART_H_ */