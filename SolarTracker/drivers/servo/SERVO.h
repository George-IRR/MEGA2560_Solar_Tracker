#ifndef SERVO_H_
#define SERVO_H_

typedef struct {
	volatile uint16_t *ocr;
	const uint16_t max_degree;
	const uint16_t low_limit;
	const uint16_t high_limit;
	uint16_t current_angle;
} PWM4_t;

extern PWM4_t PWM4_B_regs;
extern PWM4_t PWM4_C_regs;



#define MAX_SERVO_DEGREE 300

void SERVO_init(void); 

void SERVO_Zero(PWM4_t *pwm_pin);

void sendAngle(PWM4_t *pwm_pin, uint16_t angle);

void goToAngle(PWM4_t *pwm_pin, uint16_t angle);

#endif /* SERVO_H_ */