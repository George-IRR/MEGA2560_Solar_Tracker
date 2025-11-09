#include "../../drivers/usart/usart_packet.h"
#include "../../drivers/usart/USART.h"
#include "../DHT20/DHT20.h"
#include "../../drivers/servo/SERVO.h"


volatile bool task_pending_usart = false;
bool track_manual_block = true; // default state is to track

void process_scheduled_work(void)
{
	if (task_pending_usart)
	{
		const uint8_t id = task_pending_id;
		uint8_t resp_type;
		uint8_t data[7];
		uint8_t len;
		uint8_t status = STATUS_OK;
		
		switch (task_pending_type)
		{
			case CMD_DHT20:
				resp_type = RESP_DHT20;
				len = 0x07;
				getDHT20_Data(data);
			break;

			case CMD_SERVO:
				resp_type = RESP_STATUS;
				len = 0x01;
			
				if (packet_len < 3)
				{
					status = STATUS_INVALID_CMD;
					data[0] = status;
					break;
				}
			
				uint8_t servo_id = payload_buf[0];
				uint16_t angle = (payload_buf[1] << 8) | payload_buf[2];
			
				if (servo_id > 1)
				{
					status = STATUS_SERVO_INVALID_ID;
				} else if (servo_id == 0 && angle > PWM4_C_regs.max_degree)
				{
					status = STATUS_SERVO_ANGLE_OOR;
				} else if (servo_id == 1 && angle > PWM4_B_regs.max_degree)
				{
					status = STATUS_SERVO_ANGLE_OOR;
				} else
				{
					if (servo_id == 0)
					goToAngle(&PWM4_C_regs, angle);
					else
					goToAngle(&PWM4_B_regs, angle);
					status = STATUS_OK;
				}
			
				data[0] = status;
			break;
			
			case CMD_OVERRIDE:
				resp_type = RESP_OVERRIDE;
				status = STATUS_OK;

				if (packet_len >= 1 && payload_buf[0] == DATA_OVERRIDE_CTRL) 
				{
					// expect payload_len >= 2 for explicit command
					if (packet_len >= 2) 
					{
						uint8_t ctrl = payload_buf[1];
						if (ctrl == OV_CTRL_TOGGLE) 
						{
							track_manual_block = !track_manual_block;
						} else if (ctrl == OV_CTRL_SET_ON) 
						{
							track_manual_block = true;
						} else if (ctrl == OV_CTRL_SET_OFF) 
						{
							track_manual_block = false;
						} else 
						{
							status = STATUS_INVALID_CMD;
						}
					} else 
					{
					status = STATUS_INVALID_CMD; // malformed payload
					}
				} else 
				{
				// no payload tag — toggle
				track_manual_block = !track_manual_block;
				}

				// reply with current state (0 = tracking allowed, 1 = manual block)
				len = 1;
				data[0] = track_manual_block ? 1 : 0;

				task_pending_type = CMD_OVERRIDE;
				task_pending_id = id;
			break;
			
			default:
			task_pending_usart = false;
			return;
		}
		
		send_packet(&USART1_regs, resp_type, id, data, len);
		task_pending_usart = false;
	}
}
