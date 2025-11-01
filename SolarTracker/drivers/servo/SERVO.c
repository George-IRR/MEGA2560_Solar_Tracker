#define F_CPU 16000000UL 
#include <avr/io.h>
#include "SERVO.h"
#include <util/delay.h>

PWM4_t PWM4_C_regs = {
	&OCR4C, //PH5
	300,
	0,
	300,
	0		// starting angle
};
PWM4_t PWM4_B_regs = {
	&OCR4B, //PH4
	180,
	130,	// Safest max degrees
	175,	// It cannot go lower or higher because it will break the 3D print holder
	150		// starting angle
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
	
	TCCR4A |= (1<<COM4C1) | (1<<COM4B1);
	DDRH |= (1<<PH5) | (1<<PH4);
}

void SERVO_Zero(PWM4_t *pwm_pin)
{
	sendAngle(pwm_pin, pwm_pin->current_angle);
}

#include "../usart/USART.h"
void sendAngle(PWM4_t *pwm_pin, uint16_t angle)
{
	//clamp values
	if(angle < pwm_pin->low_limit) angle = 0;
	if(angle > (pwm_pin->high_limit) ) angle = (pwm_pin->high_limit);
	
	//0.5ms = 1000 ticks, 2.5ms = 5000 ticks
	*(pwm_pin->ocr) = (angle * ((5000-1000)/ (pwm_pin->max_degree) )) + 1000;
}

void goToAngle(PWM4_t *pwm_pin, uint16_t angle)
{
	// Clamp angle to limits before looping
	if (angle < pwm_pin->low_limit) angle = pwm_pin->low_limit;
	if (angle > pwm_pin->high_limit) angle = pwm_pin->high_limit;

	if (angle < pwm_pin->current_angle)
	{
		for (int16_t i = (int16_t)pwm_pin->current_angle; i >= (int16_t)angle; i--)
		{
			printInt(&USART1_regs, (uint16_t)i);  // Print the actual angle being sent
			printString(&USART1_regs, "\n");
			sendAngle(pwm_pin, (uint16_t)i);
			_delay_ms(20);
		}
	}
	else if (angle > pwm_pin->current_angle)
	{
		for (int16_t i = (int16_t)pwm_pin->current_angle; i <= (int16_t)angle; i++)
		{
			printInt(&USART1_regs, (uint16_t)i);  // Print the actual angle being sent
			printString(&USART1_regs, "\n");
			sendAngle(pwm_pin, (uint16_t)i);
			_delay_ms(20);
		}
	}

	pwm_pin->current_angle = angle;
}