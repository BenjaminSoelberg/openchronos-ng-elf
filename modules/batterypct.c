/*
    batterypct.c: battery percent display module

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
#include "drivers/battery.h"


uint8_t batt_pct_disp = FALSE;

// *************************************************************************************************
// @fn          display_battery_V
// @brief       Display routine for battery voltage.
// @param       uint8_t line		LINE2
//				uint8_t update		DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_CLEAR
// @return      none
// *************************************************************************************************
void display_battery_pct()
{
	char *str;


	// Set % icon
	display_symbol(0,LCD_SYMB_PERCENT, SEG_ON);

	// Display result in xxx% format
	str = blank_leading_zeroes(_itopct(BATTERY_EMPTY_THRESHOLD, BATTERY_FULL_THRESHOLD, sBatt.voltage));

	display_chars(0, LCD_SEG_L1_3_0, str, SEG_ON);
}

void clear_battery_pct()
{
	// Set % icon
	display_symbol(0,LCD_SYMB_PERCENT, SEG_OFF);
	display_clear(0, 1);

}


// *************************************************************************************************
// @fn          battery_activate
// @brief       battery display routine.
// @param       none
// @return      none
// *************************************************************************************************
static void battery_activate() {
	display_chars(0, LCD_SEG_L2_3_0, "BATT", SEG_SET);
#ifndef CONFIG_BATTERYMON
	battery_measurement(); //Don't need this if the background task is compiled in
#endif
	display_battery_pct();
	batt_pct_disp = TRUE;
}

static void battery_deactivate() {
	/* cleanup screen */
	batt_pct_disp = FALSE;
	display_clear(0, 2);
	clear_battery_pct();
}

static void battery_change() {
	/* Display battery symbol if low */
	display_symbol(0,LCD_SYMB_BATTERY,sBatt.low_battery ? SEG_ON : SEG_OFF);
	if (batt_pct_disp) {
		display_clear(0, 1);
		display_battery_pct();
	}
}

void batterypct_init(void) {
#ifndef CONFIG_BATTERYMON
	reset_batt_measurement(); //Don't need this if batterymon is going to do it.
#endif
	menu_add_entry("BATT",NULL, NULL, NULL, NULL, NULL, NULL,
			&battery_activate, &battery_deactivate);
	sys_messagebus_register(&battery_change, SYS_MSG_BATT);
}
