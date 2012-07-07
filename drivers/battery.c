/*
    battery.c: battery voltage measurement driver

    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//	  Redistribution and use in source and binary forms, with or without
//	  modification, are permitted provided that the following conditions
//	  are met:
//
//	    Redistributions of source code must retain the above copyright
//	    notice, this list of conditions and the following disclaimer.
//
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the
//	    documentation and/or other materials provided with the
//	    distribution.
//
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Battery voltage measurement functions.
// *************************************************************************************************


// *************************************************************************************************
// Include section
#include <cc430x613x.h>

// driver
#include "display.h"
#include "ports.h"
#include "adc12.h"
#include "battery.h"


// *************************************************************************************************
// Prototypes section
void reset_batt_measurement(void);
void battery_measurement(void);


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section
struct batt sBatt;


// *************************************************************************************************
// Extern section


// *************************************************************************************************
// @fn          reset_temp_measurement
// @brief       Reset temperature measurement module.
// @param       none
// @return      none
// *************************************************************************************************
void reset_batt_measurement(void)
{
	// Start with battery voltage of 3.00V
	sBatt.voltage = 400; //TODO: Switch back to 300!

	//No measurement to start
	sBatt.has_update = FALSE;
}


// *************************************************************************************************
// @fn          battery_measurement
// @brief       Init ADC12. Do single conversion of AVCC voltage. Turn off ADC12.
// @param       none
// @return      none
// *************************************************************************************************
void battery_measurement(void)
{
	uint16_t voltage;

	// Convert external battery voltage (ADC12INCH_11=AVCC-AVSS/2)
	//voltage = adc12_single_conversion(REFVSEL_2, ADC12SHT0_10, ADC12SSEL_0, ADC12SREF_1, ADC12INCH_11, ADC12_BATT_CONVERSION_TIME_USEC);
	voltage = adc12_single_conversion(REFVSEL_1, ADC12SHT0_10, ADC12INCH_11);

	// Convert ADC value to "x.xx V"
	// Ideally we have A11=0->AVCC=0V ... A11=4095(2^12-1)->AVCC=4V
	// --> (A11/4095)*4V=AVCC --> AVCC=(A11*4)/4095
	voltage = (voltage * 2 * 2) / 41;

	// Correct measured voltage with calibration value
	voltage += sBatt.offset;

	// Discard values that are clearly outside the measurement range
	if (voltage > BATTERY_HIGH_THRESHOLD) {
		voltage = sBatt.voltage;
	}

#ifndef CONFIG_BATTERYMON_DISABLE_BATTERY_FILTER
	// Filter battery voltage
	sBatt.voltage = ((voltage * 2) + (sBatt.voltage * 8)) / 10;
#else
	//Get it raw instead for testing
	sBatt.voltage = voltage;
#endif

	// If battery voltage falls below low battery threshold, set system flag and modify LINE2 display function pointer
	if (sBatt.voltage < BATTERY_LOW_THRESHOLD) {
		sBatt.low_battery = TRUE;

		// Set sticky battery icon
		//display_symbol(NULL,LCD_SYMB_BATTERY, SEG_ON);
	} else {
		sBatt.low_battery = FALSE;

		// Clear sticky battery icon
		//display_symbol(NULL,LCD_SYMB_BATTERY, SEG_OFF);
	}

	sBatt.has_update = TRUE;
}

