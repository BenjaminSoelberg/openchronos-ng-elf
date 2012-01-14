/*
 * dsp.c
 *
 * Some basic DSP routines that make efficient use of the MSP430 hardware multiplier.
 * For some reason inline combination of this code tends to be messed up by the
 * msp430-gcc compiler, so we stick with the call overhead.
 *
 *  Created on: Aug 5, 2010
 *      Author: Niek Lambert
 */

// *************************************************************************************************
// Include section

// logic
#include "dsp.h"

// *************************************************************************************************
// @fn          mult_scale16
// @brief       Multiply and scale rounded by 16 bits
// @param       a multiply operand 1
// @param       b multiply operand 2
// @return      (int16_t)((int32_t)a*b + 0x8000) >> 16
// *************************************************************************************************
int16_t mult_scale16(int16_t a, int16_t b)
{
#define HALF ((int32_t)0x8000)
	return (int16_t)(((int32_t)a * b + HALF) >> 16);
}

// *************************************************************************************************
// @fn          mult_scale15
// @brief       Multiply and scale rounded by 15 bits
// @param       a multiply operand 1
// @param       b multiply operand 2
// @return      (int16_t)(((int32_t)a*b << 1) + 0x8000) >> 16
// *************************************************************************************************
int16_t mult_scale15(int16_t a, int16_t b)
{
#define HALF ((int32_t)0x8000)
	int32_t ff;
	ff = ((int32_t)a * b);
	// Note 1: The sequence of a separate << 1 and >>16 operation produces
	//         far more efficient compiled code than >> 15.
	// Note 2: Combining the shift(s) with previous statement is not accepted by the compiler.
	ff <<= 1;
	return (int16_t)((ff + HALF) >> 16);
}
