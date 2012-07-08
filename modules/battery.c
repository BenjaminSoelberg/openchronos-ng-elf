/*
    battery.c: battery voltage display module

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

#include <openchronos.h>

#include "drivers/display.h"
#include "drivers/battery.h"


static uint8_t batt_v_disp = FALSE;


static void display_battery(void)
{
	/* display battery percentage on line one */
	display_chars(0, LCD_SEG_L1_3_0, _itopct(BATTERY_EMPTY_THRESHOLD,
		   BATTERY_FULL_THRESHOLD, sBatt.voltage), SEG_ON);

	/* display battery voltage in line two (xx.x format) */
	display_chars(0, LCD_SEG_L2_3_0,
	      _sprintf("%4u", sBatt.voltage), SEG_SET);
}


static void battery_activate(void)
{
#ifndef CONFIG_BATTERY_MONITOR
	/* don't need this if the background task is compiled in */
	battery_measurement();
#endif

	/* display static symbols */
	display_symbol(0, LCD_SYMB_BATTERY, SEG_ON);
	display_symbol(0, LCD_SEG_L2_DP,    SEG_ON);
	display_symbol(0, LCD_SYMB_PERCENT, SEG_ON);

	/* refresh display */
	display_battery();
	batt_v_disp = TRUE;
}

static void battery_deactivate(void)
{
	/* cleanup screen */
	batt_v_disp = FALSE;
	display_clear(0, 1);
	display_clear(0, 2);

	/* clear static symbols */
	display_symbol(0, LCD_SEG_L2_DP, SEG_OFF);
	display_symbol(0, LCD_SYMB_PERCENT, SEG_OFF);
	if (!sBatt.low_battery)
		display_symbol(0, LCD_SYMB_BATTERY, SEG_OFF);
}

#ifdef CONFIG_BATTERY_MONITOR
static void minute_event(void)
{
	battery_measurement();

	/* Display blinking battery symbol if low */
	if (sBatt.low_battery)
		display_symbol(0, LCD_SYMB_BATTERY, SEG_ON | BLINK_ON);

	if (batt_v_disp)
		display_battery();
}
#endif

void battery_init(void)
{
	reset_batt_measurement();
	menu_add_entry(" BATT", NULL, NULL, NULL, NULL, NULL, NULL,
		&battery_activate, &battery_deactivate);
#ifdef CONFIG_BATTERY_MONITOR
	sys_messagebus_register(&minute_event, SYS_MSG_RTC_MINUTE);
#endif
}
