#ifndef SERVO_H_
#define SERVO_H_

typedef struct {
	volatile uint16_t *ocrc;
} PWM4_t; // "_t" convention for typedef

extern PWM4_t PWM4_B_regs;
extern PWM4_t PWM4_C_regs;

#define MAX_SERVO_DEGREE 300

void SERVO_init(void); 

void sendAngle(PWM4_t *pwm_pin, uint16_t angle);


#endif /* SERVO_H_ */