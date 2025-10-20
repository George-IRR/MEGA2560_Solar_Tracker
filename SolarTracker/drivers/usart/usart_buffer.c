/*
  usart_buffer.c
  - Contains the USART0 RX circular buffer and the ISR.
  - Keep ISR here (file scope). Do minimal work in ISR.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdbool.h>

#include "USART.h"

#define RX0_BUFFER_SIZE 128U

static volatile uint8_t rx0_buffer[RX0_BUFFER_SIZE];
static volatile uint8_t rx0_head = 0;
static volatile uint8_t rx0_tail = 0;
static volatile bool rx0_overflow = false;

#define RX0_NEXT_IDX(i) ((uint8_t)((i) + 1U))

// Overwrite oldest when buffer is full
ISR(USART0_RX_vect)
{
    uint8_t data = UDR0;                 // read UDR0 (clears RX flag and any pending errors for this byte)
    uint8_t next = RX0_NEXT_IDX(rx0_head);

    if (next == rx0_tail) {
        // buffer full => drop oldest byte to keep newest
        rx0_tail = RX0_NEXT_IDX(rx0_tail);
        rx0_overflow = true;
    }

    rx0_buffer[rx0_head] = data;
    rx0_head = next;
}



uint8_t uart0_available(void)
{
    return (uint8_t)(rx0_head - rx0_tail);
}

int uart0_read(void)
{
    if (rx0_head == rx0_tail) return -1; //if there is no new byte.
    
	uint8_t c = rx0_buffer[rx0_tail];
    rx0_tail = RX0_NEXT_IDX(rx0_tail);
    return (int)c;
}

void uart0_flush(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rx0_head = rx0_tail;
        rx0_overflow = false;
    }
}

bool uart0_clear_overflow_flag(void)
{
    bool overflow_flag;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        overflow_flag = rx0_overflow;
        rx0_overflow = false;
    }
    return overflow_flag;
}