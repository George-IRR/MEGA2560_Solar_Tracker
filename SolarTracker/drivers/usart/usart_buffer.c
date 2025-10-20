/*
  usart_buffer.c
  - Contains the USART1 RX circular buffer and the ISR.
  - Keep ISR here (file scope). Do minimal work in ISR.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdbool.h>

#include "USART.h"

#define RX1_BUFFER_SIZE 128U

static volatile uint8_t rx1_buffer[RX1_BUFFER_SIZE];
static volatile uint8_t rx1_head = 0;
static volatile uint8_t rx1_tail = 0;
static volatile bool rx1_overflow = false;

#define RX1_NEXT_IDX(i) ((uint8_t)((i) + 1U))

// Overwrite oldest when buffer is full
ISR(USART1_RX_vect)
{
    uint8_t data = UDR1;                 // read UDR1 (clears RX flag and any pending errors for this byte)
    uint8_t next = RX1_NEXT_IDX(rx1_head);

    if (next == rx1_tail) {
        // buffer full => drop oldest byte to keep newest
        rx1_tail = RX1_NEXT_IDX(rx1_tail);
        rx1_overflow = true;
    }

    rx1_buffer[rx1_head] = data;
    rx1_head = next;
}



uint8_t uart1_available(void)
{
    return (uint8_t)(rx1_head - rx1_tail);
}

int uart1_read(void)
{
    if (rx1_head == rx1_tail) return -1; //if there is no new byte.
    
	uint8_t c = rx1_buffer[rx1_tail];
    rx1_tail = RX1_NEXT_IDX(rx1_tail);
    return (int)c;
}

void uart1_flush(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rx1_head = rx1_tail;
        rx1_overflow = false;
    }
}

bool uart1_clear_overflow_flag(void)
{
    bool overflow_flag;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        overflow_flag = rx1_overflow;
        rx1_overflow = false;
    }
    return overflow_flag;
}