#include <avr/io.h>
#include "SERVO.h"

PWM4_t PWM4_C_regs = {
	&OCR4C //PH5
};
PWM4_t PWM4_B_regs = {
	&OCR4B //PH4
};

void SERVO_init(void)
{
	//sets Timer4 registers
	TCCR4A |= (1<<WGM41);
	TCCR4B |= (1<<WGM42) | (1<<WGM43);
	
	/*
	Set clock for Timer4 2MHz
	Prescaler clk/8 
	Prescaler clk = 16MHZ
	
	each timer tick = 1/2MHz = 0.5 us
	*/
	TCCR4B |= (1<<CS41);
	
	/*
	sets timer ticks to 40.000
	40.000 * 0.5us = 20ms (50Hz) -> 
	20ms the frequency the servo works
	*/
	ICR4=40000;
	
	TCCR4A |= (1<<COM4C1);
	DDRH |= (1<<PH5);
}

void sendAngle(PWM4_t *pwm_pin, uint16_t angle)
{
	//clamp values
	if(angle < 0) angle = 0;
	if(angle > MAX_SERVO_DEGREE) angle = MAX_SERVO_DEGREE;
	
	//0.5ms = 1000 ticks, 2.5ms = 5000 ticks
	*(pwm_pin->ocrc) = (angle * ((5000-1000)/MAX_SERVO_DEGREE)) + 1000;
}