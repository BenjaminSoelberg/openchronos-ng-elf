/*
    temperature.c: temperature display module

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

// system
#include <openchronos.h>

// driver
#include "drivers/display.h"
#include "drivers/temperature.h"

uint8_t temp_edit = 0;


// *************************************************************************************************
// @fn          display_temperature
// @brief       Display routine for battery voltage.
// @param       uint8_t line		LINE2
//				uint8_t update		DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_CLEAR
// @return      none
// *************************************************************************************************
// *************************************************************************************************
// @fn          display_temperature
// @brief       Common display routine for metric and English units.
// @param       uint8_t line			LINE1
//				uint8_t update		DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_CLEAR
// @return      none
// *************************************************************************************************
void display_temperature()
{
	int16_t temperature;

	// Display �C / �F
	display_symbol(0,LCD_SEG_L1_DP1, SEG_ON);
	display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_ON);
	display_clear(0, 1);
#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
	display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
#else
	if (sTemp.is_c) {
		display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
	} else {
		display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
	}
#endif

	// When using English units, convert �C to �F (temp*1.8+32)
#ifdef CONFIG_TEMPERATURE_METRIC_ONLY
		temperature = sTemp.degrees + sTemp.offset;
#else

	if (!sTemp.is_c) {
		temperature = convert_C_to_F(sTemp.degrees + sTemp.offset);
	} else {
		temperature = sTemp.degrees + sTemp.offset;
	}

#endif

	/*Let's just display it with a sign.

	// Indicate temperature sign through arrow up/down icon
	if (temperature < 0) {
		// Convert negative to positive number
		temperature = ~temperature;
		temperature += 1;
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);
	} else { // Temperature is >= 0
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
	}
	*/

	// Display result in xx.x format
	display_chars(0, LCD_SEG_L1_3_1, _sprintf("%2s", temperature/10), SEG_ON);
}

void clear_temperature()
{
	// Clear temp
	display_symbol(0,LCD_SEG_L1_DP1, SEG_OFF);
	display_symbol(0,LCD_UNIT_L1_DEGREE, SEG_OFF);
	display_clear(0, 1);
	display_chars(0,LCD_SEG_L1_3_0,NULL,BLINK_OFF);
}

static void measure_temp(enum sys_message msg) {
	if (temp_edit)
		return;

	temperature_measurement(1);
	display_clear(0, 1);
	display_temperature();
}


// *************************************************************************************************
// @fn          temperature_activate
// @brief       temp display routine.
// @return      none
// *************************************************************************************************
static void temperature_activate() {
	display_chars(0, LCD_SEG_L2_3_0, "TEMP", SEG_SET);
	display_temperature();
	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}

static void temperature_deactivate() {
	/* cleanup screen */
	sys_messagebus_unregister(&measure_temp);
	display_clear(0, 2);
	display_chars(0,LCD_SEG_L1_3_0,NULL,BLINK_OFF);
	temp_edit = 0;
	clear_temperature();
}

static void temp_change_units() {
#ifndef CONFIG_TEMPERATURE_METRIC_ONLY
	sTemp.is_c = !sTemp.is_c;
#endif
}


static void temp_button_up() {
	if (temp_edit) {
		sTemp.offset = sTemp.offset+10;
		display_temperature();
	} else {
		temp_change_units();
	}
}

static void temp_button_down() {
	if (temp_edit) {
		sTemp.offset = sTemp.offset-10;
		display_temperature();
	} else {
		temp_change_units();
	}
}


static void edit_temp_offset() {
	/* We go into edit mode  */
	temp_edit = !temp_edit;
	display_chars(0,LCD_SEG_L1_3_0,NULL,temp_edit ? BLINK_ON : BLINK_OFF);
}

void mod_temperature_init(void) {
	temp_edit = 0;
	menu_add_entry(" TEMP",&temp_button_up, &temp_button_down, NULL, &edit_temp_offset, NULL, NULL,
			&temperature_activate, &temperature_deactivate);
}
