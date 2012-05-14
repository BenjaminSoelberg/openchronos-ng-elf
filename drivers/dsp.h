/*
 * dsp.h
 *
 *  Created on: Aug 5, 2010
 *      Author: nlv13513
 */

// *************************************************************************************************
#ifndef DSP_H_
#define DSP_H_

// Include section
#include <openchronos.h>

// *************************************************************************************************
// Prototypes section
extern int16_t mult_scale16(int16_t a, int16_t b); // returns (int16_t)((int32_t)a*b + 0x8000) >> 16
extern int16_t mult_scale15(int16_t a, int16_t b); // returns (int16_t)(((int32_t)a*b << 1) + 0x8000) >> 16

#endif /*DSP_H_*/
