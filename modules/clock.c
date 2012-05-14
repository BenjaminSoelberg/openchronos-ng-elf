/*
    modules/clock.c: clock module for openchronos-ng

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


#include <openchronos.h>

/* driver */
#include <drivers/rtca.h>
#include <drivers/display.h>

enum {
	EDIT_STATE_OFF = 0,
	EDIT_STATE_HH,
	EDIT_STATE_MM,
	EDIT_STATE_MO,
	EDIT_STATE_DD,
	EDIT_STATE_YY
} edit_state;

static uint16_t tmp_yy;
static uint8_t tmp_mo, tmp_dw, tmp_dd, tmp_hh, tmp_mm, tmp_ss;

/* vector of screens */
static struct lcd_screen screen[2];

/* pointer to screen being shown */
static struct lcd_screen *scr;

static void clock_event(enum sys_message msg)
{
	/* Exit if we are in edit mode */
	if (edit_state != EDIT_STATE_OFF)
		return;

	rtca_get_time(&tmp_hh, &tmp_mm, &tmp_ss);
	rtca_get_date(&tmp_yy, &tmp_mo, &tmp_dd, &tmp_dw);

	if (msg | SYS_MSG_RTC_YEAR)
		display_chars(&screen[1], LCD_SEG_L2_3_0, _itoa(tmp_yy, 4, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_MONTH)
		display_chars(&screen[0], LCD_SEG_L2_1_0, _itoa(tmp_mo, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_DAY)
		display_chars(&screen[0], LCD_SEG_L2_4_3, _itoa(tmp_dd, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_HOUR)
		display_chars(&screen[0], LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0), SEG_SET);
	if (msg | SYS_MSG_RTC_MINUTE)
		display_chars(&screen[0], LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0), SEG_SET);

	/* refresh real screen */
	lcd_screen_virtual_to_real(scr);
}

static void clock_activated()
{
	sys_messagebus_register(&clock_event, SYS_MSG_RTC_MINUTE \
													| SYS_MSG_RTC_HOUR \
													| SYS_MSG_RTC_DAY \
													| SYS_MSG_RTC_MONTH);	
	
	/* create two empty screens */
	lcd_screen_create(&screen[0]);
	lcd_screen_create(&screen[1]);

	/* set screen0 as current one */
	scr = &screen[0];

	/* on screen zero we display hour, day and month */
#ifdef CONFIG_CLOCK_BLINKCOL
	display_symbol(&screen[0], LCD_SEG_L1_COL, SEG_ON | BLINK_ON);
#else
	display_symbol(&screen[0], LCD_SEG_L1_COL, SEG_ON);
#endif
	display_chars(&screen[0], LCD_SEG_L1_3_0, NULL, SEG_ON);
	display_chars(&screen[0], LCD_SEG_L2_4_0, NULL, SEG_ON);
	display_char(&screen[0], LCD_SEG_L2_2, '-', SEG_SET);

	/* on screen one we display year */
	display_chars(&screen[1], LCD_SEG_L2_3_0, NULL, SEG_ON);

	/* update screens with fake event */
	clock_event(RTCA_EV_YEAR
					| RTCA_EV_MONTH
					| RTCA_EV_DAY
					| RTCA_EV_HOUR
					| RTCA_EV_MINUTE);
}

static void clock_deactivated()
{
	sys_messagebus_unregister(&clock_event);

	/* destroy virtual screens */
	lcd_screen_destroy(&screen[0]);
	lcd_screen_destroy(&screen[1]);

	/* clean up real screen */
	display_symbol(NULL, LCD_SEG_L1_COL, BLINK_OFF);
	display_clear(NULL, 1);
	display_clear(NULL, 2);
}

static void edit(int8_t step)
{
	helpers_loop_fn_t loop_fn = (step > 0 ?
					&helpers_loop_up : &helpers_loop_down);

	switch (edit_state) {
	case EDIT_STATE_YY:
		/* this allows setting years between 2012 and 2022 */
		*((uint8_t *)&tmp_yy + 1) = 0x07;
		loop_fn((uint8_t *)&tmp_yy, 220, 230);

		display_chars(&screen[1], LCD_SEG_L2_3_0, _itoa(tmp_yy, 4, 0), SEG_SET);
		break;

	case EDIT_STATE_MO:
		loop_fn(&tmp_mo, 1, 12);

		display_chars(&screen[0], LCD_SEG_L2_1_0, _itoa(tmp_mo, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_DD:
		loop_fn(&tmp_dd, 1, rtca_get_max_days(tmp_mo, tmp_yy));

		display_chars(&screen[0], LCD_SEG_L2_4_3, _itoa(tmp_dd, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_MM:
		loop_fn(&tmp_mm, 0, 59);

		display_chars(&screen[0], LCD_SEG_L1_1_0, _itoa(tmp_mm, 2, 0), SEG_SET);
		break;

	case EDIT_STATE_HH:
		/* TODO: fix for 12/24 hr! */
		loop_fn(&tmp_hh, 0, 23);

		display_chars(&screen[0], LCD_SEG_L1_3_2, _itoa(tmp_hh, 2, 0), SEG_SET);
		break;
	default:
		break;
	}

	/* refresh real screen */
	lcd_screen_virtual_to_real(scr);
}


static void edit_next()
{
	helpers_loop_up(&edit_state, EDIT_STATE_HH, EDIT_STATE_YY);

	display_chars(&screen[1], LCD_SEG_L2_3_0, NULL,
			(edit_state == EDIT_STATE_YY ? BLINK_ON : BLINK_OFF));

	display_chars(&screen[0], LCD_SEG_L2_1_0, NULL,
			(edit_state == EDIT_STATE_MO ? BLINK_ON : BLINK_OFF));

	display_chars(&screen[0], LCD_SEG_L2_4_3, NULL,
			(edit_state == EDIT_STATE_DD ? BLINK_ON : BLINK_OFF));

	display_chars(&screen[0], LCD_SEG_L1_1_0, NULL,
			(edit_state == EDIT_STATE_MM ? BLINK_ON : BLINK_OFF));

	display_chars(&screen[0], LCD_SEG_L1_3_2, NULL,
			(edit_state == EDIT_STATE_HH ? BLINK_ON : BLINK_OFF));

	/* refresh real screen */
	scr = (edit_state == EDIT_STATE_YY ? &screen[1] : &screen[0]);
	lcd_screen_virtual_to_real(scr);
}

static void edit_save()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_time(tmp_hh, tmp_mm, 0);
	rtca_set_date(tmp_yy, tmp_mo, tmp_dd);

	/* turn off only SOME blinking segments */
	display_chars(&screen[0], LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	display_chars(&screen[0], LCD_SEG_L2_4_0, NULL, BLINK_OFF);
	display_chars(&screen[1], LCD_SEG_L2_3_0, NULL, BLINK_OFF);

	/* set edit mode state to off */
	edit_state = EDIT_STATE_OFF;

	/* refresh real screen */
	lcd_screen_virtual_to_real(scr);
}


/* Num button press callback */
static void num_pressed()
{
	/* switch screen */
	scr = (scr == &screen[0] ? &screen[1] : &screen[0]);

	/* refresh real screen */
	lcd_screen_virtual_to_real(scr);
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

	menu_add_entry(NULL, NULL,
			&num_pressed,
			&star_long_pressed,
			NULL, NULL,
			&clock_activated,
			&clock_deactivated);

}
