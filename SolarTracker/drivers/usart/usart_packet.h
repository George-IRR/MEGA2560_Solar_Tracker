#ifndef USART_PACKET_H
#define USART_PACKET_H

#include <stdint.h>
#include <stdbool.h>

#include "USART.h"
#define RX1_PAYLOAD_MAX 128

// Command types
typedef enum {
	CMD_DHT20 = 0x10,
	CMD_SERVO = 0x11,
	CMD_OVERRIDE = 0x12,
	CMD_SOLARTRACK = 0x13
} cmd_type_t;

// Response types
typedef enum {
	RESP_DHT20 = 0x21,
	RESP_SERVO = 0x22,
	RESP_STATUS = 0x23,
	RESP_OVERRIDE = 0x24,
	RESP_SOLARTRACK = 0x25
} resp_type_t;

// Status codes
typedef enum {
	STATUS_OK = 0xFF,
	STATUS_BUSY = 0xFE,
	STATUS_INVALID_CMD = 0xFD,
	STATUS_SERVO_INVALID_ID = 0xFC,
	STATUS_SERVO_ANGLE_OOR = 0xFB,
	STATUS_CHECKSUM_ERR = 0xFA
} status_code_t;

// Payload data types
typedef enum{
	DATA_STATUS    = 0x40,
	DATA_ALIVE     = 0x41,
	DATA_TELEMETRY = 0x42,
	DATA_OVERRIDE_CTRL = 0x43   // payload[0] = override control tag
} payload_data_type_t;

// payload[0] == DATA_OVERRIDE_CTRL
typedef enum {
	OV_CTRL_TOGGLE   = 0x00, // flip current state
	OV_CTRL_SET_ON   = 0x01, // force manual block = true
	OV_CTRL_SET_OFF  = 0x02  // force manual block = false
} override_ctrl_t;

extern bool track_manual_block; //manually block solar tracking

extern uint8_t task_pending_id;
extern uint8_t task_pending_type;
extern uint8_t packet_len;
extern uint8_t payload_buf[RX1_PAYLOAD_MAX];

// Public API functions
void process_uart1_bytes(void);
void handle_packet(uint8_t version, uint8_t type, uint8_t packet_id, uint8_t *payload_buf, uint8_t packet_len);
void send_packet(USART_t *usart, uint8_t type, uint8_t id, uint8_t *payload, uint8_t len);

#endif /* USART_PACKET_H */