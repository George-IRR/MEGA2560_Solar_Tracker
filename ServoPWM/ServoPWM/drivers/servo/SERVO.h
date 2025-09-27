#ifndef SERVO_H_
#define SERVO_H_


const uint16_t MAX_SERVO_DEGREE = 300; // type safe

void SERVO_init(void); 

void sendAngle(uint16_t angle);


#endif /* SERVO_H_ */