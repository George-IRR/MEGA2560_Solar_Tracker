#define F_CPU 16000000UL
#include <stdint.h>
#include <util/delay.h>
#include "../../drivers/servo/SERVO.h"
#include "../../drivers/adc/ADC.h"

/*
 * Layout:
 * ADC14 (Left Top)     |  ADC13 (Right Top)
 * ---------------------|--------------------
 * ADC15 (Left Bottom)  |  ADC12 (Right Bottom)
 *
 */
void solarTrack2Axis_loop(void) 
{
	const int16_t tolerance = 50; // Deadzone threshold
	
	// Use #define for array size instead of const
	#define AVG_SAMPLES 4
	
	// Circular buffers for averaging
	int32_t diff_h_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	int32_t diff_v_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	uint8_t buffer_index = 0;
	uint8_t samples_collected = 0;
	
	while (1) 
	{
		// Read LDR values
		uint16_t analogValue0 = ADC_read(12);
		uint16_t analogValue1 = ADC_read(13);
		uint16_t analogValue2 = ADC_read(14);
		uint16_t analogValue3 = ADC_read(15);

		// Calculate aggregate light for each side
		uint32_t left_light = analogValue2 + analogValue3;
		uint32_t right_light = analogValue0 + analogValue1;
		uint32_t top_light = analogValue1 + analogValue2;
		uint32_t bottom_light = analogValue0 + analogValue3;

		// Calculate differences
		// diff_h > 0	Left is brighter
		// diff_v > 0	Top is brighter
		int32_t diff_h_raw = left_light - right_light;
		int32_t diff_v_raw = top_light - bottom_light;
		
		// Store in circular buffer
		diff_h_buffer[buffer_index] = diff_h_raw;
		diff_v_buffer[buffer_index] = diff_v_raw;
		buffer_index = (buffer_index + 1) % AVG_SAMPLES;
		
		// Track how many samples we've collected (up to AVG_SAMPLES)
		if (samples_collected < AVG_SAMPLES) 
		{
			samples_collected++;
		}
		
		// Calculate averages
		int32_t diff_h_sum = 0;
		int32_t diff_v_sum = 0;
		for (uint8_t i = 0; i < samples_collected; i++) 
		{
			diff_h_sum += diff_h_buffer[i];
			diff_v_sum += diff_v_buffer[i];
		}
		int32_t diff_h = diff_h_sum / samples_collected;
		int32_t diff_v = diff_v_sum / samples_collected;
		
		// Only make movements after we have enough samples
		if (samples_collected >= AVG_SAMPLES) 
		{
			// angle++ moves Left (towards brighter 'left_light')
			if (diff_h > tolerance)
			{
				if (PWM4_C_regs.current_angle < PWM4_C_regs.high_limit)
				{
					PWM4_C_regs.current_angle++;
					sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
				}
			}
			//angle-- moves Right (towards brighter 'right_light')
			else if (diff_h < -tolerance)
			{
				if (PWM4_C_regs.current_angle > PWM4_C_regs.low_limit)
				{
					PWM4_C_regs.current_angle--;
					sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
				}
			}
			
			//angle-- moves Up (towards brighter 'top_light')
			if (diff_v > tolerance)
			{
				if (PWM4_B_regs.current_angle > PWM4_B_regs.low_limit)
				{
					PWM4_B_regs.current_angle--;
					sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
				}

			}
			
			//angle++ moves Down (towards brighter 'bottom_light')
			else if (diff_v < -tolerance)
			{
				if (PWM4_B_regs.current_angle < PWM4_B_regs.high_limit)
				{
					PWM4_B_regs.current_angle++;
					sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
				}
			}
		}
		
		_delay_ms(12); //17 -> slow  12 -> med  7 ->fast
	}
	
	#undef AVG_SAMPLES
}

void solarTrack2Axis_step(void)
{
	const int16_t tolerance = 50; // Deadzone threshold
	
	// Use #define for array size instead of const
	#define AVG_SAMPLES 4
	
	// Circular buffers for averaging
	static int32_t diff_h_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	static int32_t diff_v_buffer[AVG_SAMPLES] = {0, 0, 0, 0};
	static uint8_t buffer_index = 0;
	static uint8_t samples_collected = 0;

	// Read LDR values
	uint16_t analogValue0 = ADC_read(12);
	uint16_t analogValue1 = ADC_read(13);
	uint16_t analogValue2 = ADC_read(14);
	uint16_t analogValue3 = ADC_read(15);

	// Calculate aggregate light for each side
	uint32_t left_light = analogValue2 + analogValue3;
	uint32_t right_light = analogValue0 + analogValue1;
	uint32_t top_light = analogValue1 + analogValue2;
	uint32_t bottom_light = analogValue0 + analogValue3;

	// Calculate differences
	// diff_h > 0	Left is brighter
	// diff_v > 0	Top is brighter
	int32_t diff_h_raw = left_light - right_light;
	int32_t diff_v_raw = top_light - bottom_light;
	
	// Store in circular buffer
	diff_h_buffer[buffer_index] = diff_h_raw;
	diff_v_buffer[buffer_index] = diff_v_raw;
	buffer_index = (buffer_index + 1) % AVG_SAMPLES;
	
	// Track how many samples we've collected (up to AVG_SAMPLES)
	if (samples_collected < AVG_SAMPLES)
	{
		samples_collected++;
	}
	
	// Calculate averages
	int32_t diff_h_sum = 0;
	int32_t diff_v_sum = 0;
	for (uint8_t i = 0; i < samples_collected; i++)
	{
		diff_h_sum += diff_h_buffer[i];
		diff_v_sum += diff_v_buffer[i];
	}
	int32_t diff_h = diff_h_sum / samples_collected;
	int32_t diff_v = diff_v_sum / samples_collected;
	
	// Only make movements after we have enough samples
	if (samples_collected >= AVG_SAMPLES)
	{
		// angle++ moves Left (towards brighter 'left_light')
		if (diff_h > tolerance)
		{
			if (PWM4_C_regs.current_angle < PWM4_C_regs.high_limit)
			{
				PWM4_C_regs.current_angle++;
				sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
			}
		}
		//angle-- moves Right (towards brighter 'right_light')
		else if (diff_h < -tolerance)
		{
			if (PWM4_C_regs.current_angle > PWM4_C_regs.low_limit)
			{
				PWM4_C_regs.current_angle--;
				sendAngle(&PWM4_C_regs, PWM4_C_regs.current_angle);
			}
		}
		
		//angle-- moves Up (towards brighter 'top_light')
		if (diff_v > tolerance)
		{
			if (PWM4_B_regs.current_angle > PWM4_B_regs.low_limit)
			{
				PWM4_B_regs.current_angle--;
				sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
			}

		}
		
		//angle++ moves Down (towards brighter 'bottom_light')
		else if (diff_v < -tolerance)
		{
			if (PWM4_B_regs.current_angle < PWM4_B_regs.high_limit)
			{
				PWM4_B_regs.current_angle++;
				sendAngle(&PWM4_B_regs, PWM4_B_regs.current_angle);
			}
		}
	}
		
	_delay_ms(12); //17 -> slow  12 -> med  7 ->fast
	
	#undef AVG_SAMPLES
}