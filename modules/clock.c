/*
    modules/clock.c: Openchronos clock module

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

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


#include <ezchronos.h>

/* driver */
#include <drivers/rtca.h>
#include <drivers/display.h>

enum {
	EDIT_STATE_OFF = 0,
	EDIT_STATE_HH,
	EDIT_STATE_MM,
	EDIT_STATE_MO,
	EDIT_STATE_DD,
} edit_state;

static uint16_t tmp_yy;
static uint8_t tmp_mo, tmp_dd, tmp_hh, tmp_mm;

static void clock_event(enum sys_message msg)
{
	/* Exit if we are in edit mode */
	if (edit_state != EDIT_STATE_OFF)
		return;

	uint16_t yy;
	uint8_t hh, mm, ss, dd, dw, mo;
	rtca_get_time(&hh, &mm, &ss);
	rtca_get_date(&yy, &mo, &dd, &dw);

	if (msg | SYS_MSG_RTC_MONTH)
		display_chars(LCD_SEG_L2_1_0, _itoa(mo, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_DAY)
		display_chars(LCD_SEG_L2_4_3, _itoa(dd, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_HOUR)
		display_chars(LCD_SEG_L1_3_2, _itoa(hh, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_MINUTE)
		display_chars(LCD_SEG_L1_1_0, _itoa(mm, 2, 0), SEG_SET);
}

static void clock_activated()
{
	sys_messagebus_register(&clock_event, SYS_MSG_RTC_MINUTE \
													| SYS_MSG_RTC_HOUR \
													| SYS_MSG_RTC_DAY \
													| SYS_MSG_RTC_MONTH);

	/* Force redraw of the screen */
#ifdef CONFIG_CLOCK_BLINKCOL
	display_symbol(LCD_SEG_L1_COL, SEG_ON | BLINK_ON);
#else
	display_symbol(LCD_SEG_L1_COL, SEG_ON);
#endif
	display_char(LCD_SEG_L2_2, '-', SEG_ON);

	clock_event(RTCA_EV_MONTH);
}

static void clock_deactivated()
{
	sys_messagebus_unregister(&clock_event);

	/* clean up screen */
	display_symbol(LCD_SEG_L1_COL, BLINK_OFF);
	display_clear(1);
	display_clear(2);
}

static void edit(int8_t step)
{
	helpers_loop_fn_t loop_fn = (step > 0 ?
					&helpers_loop_up : &helpers_loop_down);

	switch (edit_state) {
	case EDIT_STATE_MO:
		loop_fn(&tmp_mo, 1, 12);

		display_chars(LCD_SEG_L2_1_0, _itoa(tmp_mo, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_DD:
		/* TODO: Fix this, decide where to display year.. */
		loop_fn(&tmp_dd, 1, rtca_get_max_days(tmp_mo, tmp_yy));

		display_chars(LCD_SEG_L2_4_3, _itoa(tmp_dd, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_MM:
		loop_fn(&tmp_mm, 0, 59);

		display_chars(LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_HH:
		/* TODO: fix for 12/24 hr! */
		loop_fn(&tmp_hh, 0, 23);

		display_chars(LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0), SEG_SET);
		break;
	default:
		break;
	}
}


static void edit_next()
{
	helpers_loop_up(&edit_state, EDIT_STATE_HH, EDIT_STATE_DD);

	display_chars(LCD_SEG_L2_1_0, NULL,
			(edit_state == EDIT_STATE_MO ? BLINK_ON : BLINK_OFF));

	display_chars(LCD_SEG_L2_4_3, NULL,
			(edit_state == EDIT_STATE_DD ? BLINK_ON : BLINK_OFF));

	display_chars(LCD_SEG_L1_1_0, NULL,
			(edit_state == EDIT_STATE_MM ? BLINK_ON : BLINK_OFF));

	display_chars(LCD_SEG_L1_3_2, NULL,
			(edit_state == EDIT_STATE_HH ? BLINK_ON : BLINK_OFF));
}

static void edit_save()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_time(tmp_hh, tmp_mm, 0);
	rtca_set_date(tmp_yy, tmp_mo, tmp_dd);

	/* turn off only SOME blinking segments */
	display_chars(LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	display_chars(LCD_SEG_L2_4_0, NULL, BLINK_OFF);

	/* set edit mode state to off */
	edit_state = EDIT_STATE_OFF;
}


/* Star button long press callback. */
static void star_long_pressed()
{
	/* We go into edit mode  */
	edit_state = EDIT_STATE_DD;

	/* Save the current time in edit_buffer */
	uint8_t tmp;
	rtca_get_time(&tmp_hh, &tmp_mm, &tmp);
	rtca_get_date(&tmp_yy, &tmp_mo, &tmp_dd, &tmp);

	menu_editmode_start(&edit, &edit_next, &edit_save);

}

void clock_init()
{
	edit_state = EDIT_STATE_OFF;

	menu_add_entry(NULL, NULL, NULL,
			&star_long_pressed,
			NULL, NULL,
			&clock_activated,
			&clock_deactivated);

}
