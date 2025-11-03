#include <avr/io.h>

/**
 * Uses CTC mode (Mode 4) with a prescaler of 8.
 * Prescaler: 8
 * Ticks per second: 16,000,000 / 8 = 2,000,000 Hz
 * Ticks for 20ms: 2,000,000 * 0.020s = 40,000 ticks
 * Compare Value (OCR1A) = 40,000 - 1 = 39,999
 */
void Timer1_Init_Polling(void)
{
    // 1. Reset all Timer1 registers to a known state
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    // 2. Set the Compare Match Value
    //OCR1A = 23999; // 12ms
	OCR1A = 39999; // 20ms
	
    // 3. Set CTC Mode (Mode 4) + Prescaler to 8
    TCCR1B = (1 << WGM12) | (1 << CS11);

    // 4. Clear the Output Compare A Match Flag (OCF1A)
    TIFR1 = (1 << OCF1A);
}