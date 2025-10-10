#ifndef SERVO_H_
#define SERVO_H_


#define MAX_SERVO_DEGREE 300

void SERVO_init(void); 

void sendAngle(uint16_t angle);


#endif /* SERVO_H_ */