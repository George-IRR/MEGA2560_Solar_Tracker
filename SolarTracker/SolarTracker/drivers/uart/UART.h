#ifndef UART_H_
#define UART_H_

#define F_CPU 16000000UL

void UART_init(uint32_t baud);

void UART_sendBtye(uint8_t data);

void printString(const char* str);

void printInt(uint32_t value);

#endif /* UART_H_ */