#ifndef TWI_H_
#define TWI_H_

void TWI_init(void);
void TWI_start(void);
void TWI_stop(void);
void TWI_write(uint8_t data);
uint8_t TWI_read_ack(void);
uint8_t TWI_read_nack(void);

#endif /* TWI_H_ */