/**
    battery.c: battery voltage measurement driver

    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>
    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

    This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

/***************************************************************************

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

****************************************************************************/
#include "openchronos.h"

#include "display.h"
#include "battery.h"

#include "ports.h"
#include "adc12.h"

void battery_init(void)
{
    /* Start with battery voltage estimate of full and avoid low
      battery warnings until the voltage estimate converges. */
    battery_info.voltage = BATTERY_FULL_THRESHOLD;
}


void battery_measurement(void)
{
    /* Convert external battery voltage (ADC12INCH_11=AVCC-AVSS/2)
    voltage = adc12_single_conversion(REFVSEL_2, ADC12SHT0_10,
          ADC12SSEL_0, ADC12SREF_1, ADC12INCH_11,
            ADC12_BATT_CONVERSION_TIME_USEC); */
    uint16_t voltage = adc12_single_conversion(REFVSEL_1,
            ADC12SHT0_10, ADC12INCH_11);

    /* Convert ADC value to "x.xx V"
     Ideally we have A11=0->AVCC=0V ... A11=4095(2^12-1)->AVCC=4V
     --> (A11/4095)*4V=AVCC --> AVCC=(A11*4)/4095 */
    voltage = (voltage << 2) / 41;

    /* Correct measured voltage with calibration value */
    voltage += battery_info.offset;

    /* Discard values that are clearly outside the measurement range */
    if (voltage > BATTERY_HIGH_THRESHOLD)
        voltage = battery_info.voltage;

#ifndef CONFIG_BATTERY_DISABLE_FILTER
    /* Filter battery voltage */
    battery_info.voltage = ((voltage << 1)
            + (battery_info.voltage << 3)) / 10;
#else
    /* Get it raw instead for testing */
    battery_info.voltage = voltage;
#endif

    /* Display blinking battery symbol if low */
    if (battery_info.voltage < BATTERY_LOW_THRESHOLD)
        display_symbol(0, LCD_SYMB_BATTERY, SEG_ON | BLINK_ON);
}

