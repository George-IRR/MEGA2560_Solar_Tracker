#ifndef USART_BUFFER_H_
#define USART_BUFFER_H_



uint8_t uart1_available(void);

int uart1_read(void);

void uart1_flush(void);

bool uart1_clear_overflow_flag(void);



#endif /* USART_BUFFER_H_ */