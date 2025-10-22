#include <stdint.h>
#include <stdbool.h>

#include "USART.h"
#include "../modules/DHT20.h"

#define RX1_PAYLOAD_MAX 128

typedef enum {
	WAIT_PREAMBLE_1,
	WAIT_PREAMBLE_2,
	READ_VERSION,
	READ_TYPE,
	READ_ID,
	READ_LEN,
	READ_PAYLOAD,
	READ_CHECKSUM
} parser_state_t;

static parser_state_t pstate = WAIT_PREAMBLE_1;
static uint8_t packet_version;
static uint8_t packet_type;
static uint8_t packet_id;
static uint8_t packet_len;
static uint8_t payload_buf[RX1_PAYLOAD_MAX];
static uint8_t payload_pos;
static uint16_t checksum_acc; // use 16-bit to accumulate safely

// forward declarations of the usart_buffer API from usart_buffer.c
uint8_t uart1_available(void);
int uart1_read(void);
bool uart1_clear_overflow_flag(void);

void handle_packet(uint8_t version, uint8_t type, uint8_t packet_id, uint8_t *payload_buf, uint8_t packet_len)
{
	uint8_t data[7];
	switch (type) 
	{
		case 0x10: // CMD_REQ
		getDHT20_Data(data); // example param for now AA 55 01 10 00 00 11
		for(int i = 0; i < 7; i++)
		 	printHex(&USART0_regs, data[i]);
		break;
	
		case 0x20: // TELEMETRY
		printHex(&USART1_regs,0XFF);
		break;
		
		default:
		// unknown type — log or ignore
		printString(&USART0_regs, "Unknown packet type\r\n");
		break;
	}
}

void process_uart1_bytes(void)
{
	while (uart1_available())
	{
		int ci = uart1_read();
		if (ci < 0) break;
		uint8_t b = (uint8_t)ci;

		switch (pstate)
		{
			case WAIT_PREAMBLE_1:
			if (b == 0xAA) pstate = WAIT_PREAMBLE_2;
			break;

			case WAIT_PREAMBLE_2:
			if (b == 0x55)
			{
				pstate = READ_VERSION;
			} else
			{
				// handle overlapping: if this byte is 0xAA, stay in state2 (start again),
				// else go back to searching
				pstate = (b == 0xAA) ? WAIT_PREAMBLE_2 : WAIT_PREAMBLE_1;
			}
			break;

			case READ_VERSION:
			packet_version = b;
			checksum_acc = packet_version; // start checksum from version
			pstate = READ_TYPE;
			break;

			case READ_TYPE:
			packet_type = b;
			checksum_acc += packet_type;
			pstate = READ_ID;
			break;

			case READ_ID:
			packet_id = b;
			checksum_acc += packet_id;
			pstate = READ_LEN;
			break;

			case READ_LEN:
			packet_len = b;
			checksum_acc += packet_len;
			if (packet_len == 0)
			{
				// no payload, next read checksum
				pstate = READ_CHECKSUM;
			} else if (packet_len > RX1_PAYLOAD_MAX)
			{
				// payload too big -> reject and resync
				pstate = WAIT_PREAMBLE_1;
			} else
			{
				payload_pos = 0;
				pstate = READ_PAYLOAD;
			}
			break;

			case READ_PAYLOAD:
			payload_buf[payload_pos++] = b;
			checksum_acc += b;
			if (payload_pos >= packet_len)
			{
				pstate = READ_CHECKSUM;
			}
			break;

			case READ_CHECKSUM:
			{
				uint8_t chk = b;
				uint8_t calc = (uint8_t)(checksum_acc & 0xFF);
				if (chk == calc)
				{
					// packet valid -> process it
					handle_packet(packet_version, packet_type, packet_id, payload_buf, packet_len);
					printString(&USART0_regs, "\r\n");
					printString(&USART0_regs, "Packet OK: type=");
					printHex(&USART0_regs, packet_type);
					printString(&USART0_regs, " len=");
					printInt(&USART0_regs, packet_len);
					printString(&USART0_regs, "\r\n");
				} else
				{
					// bad checksum: ignore or log
					printString(&USART0_regs, "Bad checksum\r\n");
				}
				
				// Look for next packet
				pstate = WAIT_PREAMBLE_1;
				break;
			}

			default:
			pstate = WAIT_PREAMBLE_1;
			break;
		} // switch
	} // while available
}
