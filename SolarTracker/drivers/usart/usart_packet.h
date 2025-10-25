#ifndef USART_PACKET_H
#define USART_PACKET_H

#include <stdint.h>
#include <stdbool.h>

extern volatile bool dht_request_pending;

// Public API for the UART packet parser
void process_uart1_bytes(void);

void process_scheduled_work(void);

// Called by the parser when a valid packet is received.
void handle_packet(uint8_t version, uint8_t type, uint8_t packet_id, uint8_t *payload_buf, uint8_t packet_len);

void send_packet(USART_t *usart, uint8_t type, uint8_t id, const uint8_t *payload, uint8_t len);

#endif /* USART_PACKET_H */