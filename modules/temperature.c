/*
    temperature.c: temperature display module

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
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

/* drivers */
#include "drivers/display.h"
#include "drivers/temperature.h"

static uint8_t temp_edit;

void display_temperature(void)
{
	/* Display oC / oF */
	display_symbol(0, LCD_SEG_L1_DP1, SEG_ON);
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_ON);
	display_clear(0, 1);

	int16_t temp;
#ifdef CONFIG_TEMPERATURE_METRIC
	display_char(0, LCD_SEG_L1_0, 'C', SEG_ON);
	temperature_get_C(&temp);
#else
	display_char(0, LCD_SEG_L1_0, 'F', SEG_ON);
	temperature_get_F(&temp);
#endif

	display_chars(0, LCD_SEG_L1_3_1, _sprintf("%2s", temp/10), SEG_ON);
}

void clear_temperature(void)
{
	display_symbol(0, LCD_SEG_L1_DP1, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_OFF);
	display_clear(0, 1);
	display_chars(0, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
}

static void measure_temp(enum sys_message msg)
{
	temperature_measurement();
	display_clear(0, 1);
	display_temperature();
}


static void temperature_activate(void)
{
	display_chars(0, LCD_SEG_L2_3_0, "TEMP", SEG_SET);
	display_temperature();
	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}

static void temperature_deactivate(void)
{
	/* cleanup screen */
	sys_messagebus_unregister(&measure_temp);
	display_clear(0, 2);
	display_chars(0, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	temp_edit = 0;
	clear_temperature();
}

static void temp_button_up(void)
{
	if (temp_edit) {
		temperature.offset++;
		display_temperature();
	}
}

static void temp_button_down(void)
{
	if (temp_edit) {
		temperature.offset--;
		display_temperature();
	}
}


static void edit_temp_offset(void)
{
	/* We go into edit mode  */
	temp_edit = !temp_edit;
	display_chars(0, LCD_SEG_L1_3_0, NULL,
		temp_edit ? BLINK_ON : BLINK_OFF);
}

void mod_temperature_init(void)
{
	temp_edit = 0;
	menu_add_entry(" TEMP", &temp_button_up, &temp_button_down,
		NULL, &edit_temp_offset, NULL, NULL,
		&temperature_activate, &temperature_deactivate);
}
