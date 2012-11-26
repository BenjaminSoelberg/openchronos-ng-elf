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

static void display_temperature(void)
{
	int16_t temp;
#ifdef CONFIG_TEMPERATURE_METRIC
	temperature_get_C(&temp);
#else
	temperature_get_F(&temp);
#endif
	_printf(0, LCD_SEG_L2_3_0, "%03s", temperature.offset);
	_printf(0, LCD_SEG_L1_3_1, "%2s", temp/10);
	display_char(0, LCD_SEG_L1_0, (temp%10)+48, SEG_SET);
}

static void measure_temp(enum sys_message msg)
{
	temperature_measurement();
	display_temperature();
}

/********************* edit mode callbacks ********************************/

static void edit_offset_sel(void)
{
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_ON);
}
static void edit_offset_dsel(void)
{
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_OFF);
}
static void edit_offset_set(int8_t step)
{
	temperature.offset += step;
	display_temperature();
}


static void edit_save()
{
	/* turn off blinking segments */
	display_chars(0, LCD_SEG_L2_3_0, NULL, BLINK_OFF);
}

static struct menu_editmode_item edit_items[] = {
	{&edit_offset_sel, &edit_offset_dsel, &edit_offset_set},
	{ NULL },
};

/************************** menu callbacks ********************************/

static void temperature_activate(void)
{
	/* display static elements */
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_ON);
	display_symbol(0, LCD_SEG_L1_DP0, SEG_ON);
#ifdef CONFIG_TEMPERATURE_METRIC
	display_char(0, LCD_SEG_L2_4, 'C', SEG_SET);
#else
	display_char(0, LCD_SEG_L2_4, 'F', SEG_SET);
#endif
	
	/* display -- symbol while a measure is not performed */
	display_chars(0, LCD_SEG_L1_2_0, "---", SEG_ON);

	sys_messagebus_register(&measure_temp, SYS_MSG_TIMER_4S);
}

static void temperature_deactivate(void)
{
	sys_messagebus_unregister(&measure_temp);
	
	/* cleanup screen */
	display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_OFF);
	display_symbol(0, LCD_SEG_L1_DP0, SEG_OFF);
}

static void temperature_edit(void)
{
	/* We go into edit mode  */
	menu_editmode_start(&edit_save, edit_items);
}

void mod_temperature_init(void)
{
	menu_add_entry(" TEMP", NULL, NULL,
		NULL, &temperature_edit, NULL, NULL,
		&temperature_activate, &temperature_deactivate);
}
