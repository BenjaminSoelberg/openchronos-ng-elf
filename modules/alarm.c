/*
    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>
	   - Simplified code, allow simultaneous chime and alarm
	   - Updated to use RTC_A, the realtime clock driver
	   - Ported to new menu display API.
	   18/02/2012: Rewritten to adhere to new modular system

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

#include "ezchronos.h"

/* driver */
#include "display.h"
#include "rtca.h"

static enum {
	EDIT_STATE_HH = 0,
	EDIT_STATE_MM,
} edit_state;

static uint8_t tmp_hh, tmp_mm;

static void refresh_screen()
{
	display_chars(LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0), SEG_ON);
	display_chars(LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0), SEG_ON);
	display_symbol(LCD_SEG_L1_COL, SEG_ON);
}

static void alarm_activated()
{
	/* Force redraw of the screen */
	refresh_screen();
}


static void alarm_deactivated()
{
	/* clean up screen */
	clear_line(LINE1);
}


static void edit(int8_t step)
{
	helpers_loop_fn_t loop_fn = (step > 0 ?
					&helpers_loop_up : &helpers_loop_down);

	if (edit_state == EDIT_STATE_MM) {
		loop_fn(&tmp_mm, 0, 59);

		display_chars(LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0),
							SEG_ON_BLINK_ON);
	} else {
		/* TODO: fix for 12/24 hr! */
		loop_fn(&tmp_hh, 0, 23);

		display_chars(LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0),
							SEG_ON_BLINK_ON);
	}
}


static void edit_next()
{
	helpers_loop_up(&edit_state, EDIT_STATE_HH, EDIT_STATE_MM);

	display_chars(LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0),
		(edit_state == EDIT_STATE_MM ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));

	display_chars(LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0),
		(edit_state == EDIT_STATE_HH ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));
}


static void edit_save()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_alarm(tmp_hh, tmp_mm);

	/* hack to only turn off SOME blinking segments */
	display_chars(LCD_SEG_L1_1_0, _itoa(88, 2, 0), SEG_ON_BLINK_OFF);
	display_chars(LCD_SEG_L1_3_2, _itoa(88, 2, 0), SEG_ON_BLINK_OFF);

	/* force redraw of the screen */
	refresh_screen();
}


/* Star button long press callback. */
static void star_long_pressed()
{
	/* We go into edit mode  */
	edit_state = EDIT_STATE_MM;

	/* Save the current time in edit_buffer */
	rtca_get_alarm(&tmp_hh, &tmp_mm);

	menu_editmode_start(&edit, &edit_next, &edit_save);

}


void alarm_init()
{
	menu_add_entry(NULL, NULL, NULL,
			&star_long_pressed,
			NULL,
			&alarm_activated,
			&alarm_deactivated);

}
