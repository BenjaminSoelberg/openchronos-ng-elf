/*
    temperature.c: Temperature driver

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
/*
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the
      distribution.

      Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* driver */
#include "temperature.h"
#include "ports.h"
#include "display.h"
#include "adc12.h"
#include "timer.h"

/* TODO: This must go away */
struct temp sTemp;


void temperature_init(void)
{
	/* Perform one temperature measurement with disabled filter */
	temperature_measurement(0);
}


void temperature_measurement(uint8_t filter)
{
	uint16_t adc_result;
	int32_t temperature;

	/* Convert internal temperature diode voltage */
	adc_result = adc12_single_conversion(REFVSEL_0, ADC12SHT0_8,
								ADC12INCH_10);

	/* Convert ADC value to "xx.x C"
	Temperature in Celsius
	  ((A10/4096*1500mV) - 680mV)*(1/2.25mV) = (A10/4096*667) - 302
	  = (A10 - 1855) * (667 / 4096) */
	temperature = (((int32_t)((int32_t)adc_result - 1855)) * 667 * 10)
								/ 4096;

	/* FIXME: Wrong, offset should be compensated in the driver */
	/* Add temperature offset - we do this at display
	   makes for easier editing */
	/* temperature += sTemp.offset; */

	/* Limit min/max temperature to +/- 50 C */
	if (temperature > 500)
		temperature = 500;
	if (temperature < -500)
		temperature = -500;

	/* Store measured temperature */
	if (filter) {
		/* Change temperature in 0.1 steps towards measured value */
		if (temperature > sTemp.degrees)
			sTemp.degrees += 1;
		else if (temperature < sTemp.degrees)
			sTemp.degrees -= 1;
	} else {
		/* Override filter */
		sTemp.degrees = (int16_t)temperature;
	}
}


#ifndef CONFIG_TEMPERATURE_METRIC_ONLY
int16_t convert_C_to_F(int16_t value)
{
	int16_t DegF;

	/* Celsius in Fahrenheit = (( TCelsius * 9 ) / 5 ) + 32 */
	DegF = ((value * 9 * 10) / 5 / 10) + 32 * 10;

	return DegF;
}


int16_t convert_F_to_C(int16_t value)
{
	int16_t DegC;

	/* TCelsius =( TFahrenheit - 32 ) * 5 / 9 */
	DegC = (((value - 320) * 5)) / 9;

	return DegC;
}
#endif


