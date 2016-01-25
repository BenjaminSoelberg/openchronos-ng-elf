/*
    modules/alarm.c: Alarm module for openchronos-ng

    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>

	           http://www.openchronos-ng.sourceforge.net

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
#include <drivers/display.h>
#include <drivers/rtca.h>

static union {
	struct {
		/* one shot alarm */
		uint8_t alarm:1;
		/* hourly chime */
		uint8_t chime:1;
	};
	uint8_t state:2;
} alarm_state;

static uint8_t tmp_hh, tmp_mm;

static void refresh_screen()
{
	rtca_get_alarm(&tmp_hh, &tmp_mm);

	_printf(0, LCD_SEG_L1_1_0, "%02u", tmp_mm);
	_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
}

static void alarm_event(enum sys_message msg)
{
	/* TODO: */
}

/*************************** edit mode callbacks **************************/
/* Hour */
static void edit_hh_sel(void)
{
	display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_ON);
}

static void edit_hh_dsel(void)
{
	display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
}

static void edit_hh_set(int8_t step)
{
	/* TODO: fix for 12/24 hr! */
	helpers_loop(&tmp_hh, 0, 23, step);
	_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
}

/* Minute */
static void edit_mm_sel(void)
{
	display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_ON);
}

static void edit_mm_dsel(void)
{
	display_chars(0, LCD_SEG_L1_1_0, NULL, BLINK_OFF);
}

static void edit_mm_set(int8_t step)
{
	helpers_loop(&tmp_mm, 0, 59, step);
	_printf(0, LCD_SEG_L1_1_0, "%02u", tmp_mm);
}

/* Save */
static void edit_save(void)
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_alarm(tmp_hh, tmp_mm);
}

/* edit mode item table */
static struct menu_editmode_item edit_items[] = {
	{&edit_hh_sel, &edit_hh_dsel, &edit_hh_set},
	{&edit_mm_sel, &edit_mm_dsel, &edit_mm_set},
	{ NULL },
};

/******************** menu callbacks **************************************/
static void alarm_activated()
{
	/* Force redraw of the screen */
	display_symbol(0, LCD_SEG_L1_COL, SEG_ON);
	refresh_screen();
}


static void alarm_deactivated()
{
	/* clean up screen */
	display_clear(0, 1);
}


/* NUM (#) button pressed callback */
static void num_pressed()
{
	/* this cycles between all alarm/chime combinations and overflow */
	alarm_state.state++;

	/* Register RTC only if needed, saving CPU cycles.. */
	if (alarm_state.state)
		sys_messagebus_register(alarm_event, SYS_MSG_RTC_ALARM);
	else
		sys_messagebus_unregister(alarm_event);

	if (alarm_state.alarm) {
		display_symbol(0, LCD_ICON_ALARM, SEG_ON);
		rtca_enable_alarm();
	} else {
		display_symbol(0, LCD_ICON_ALARM, SEG_OFF);
		rtca_disable_alarm();
	}

	if (alarm_state.chime) {
		display_symbol(0, LCD_ICON_BEEPER2, SEG_ON);
		display_symbol(0, LCD_ICON_BEEPER3, SEG_ON);
	} else {
		display_symbol(0, LCD_ICON_BEEPER2, SEG_OFF);
		display_symbol(0, LCD_ICON_BEEPER3, SEG_OFF);
	}

}


/* Star button long press callback. */
static void star_long_pressed()
{
	/* Save the current time in edit_buffer */
	rtca_get_alarm(&tmp_hh, &tmp_mm);

	menu_editmode_start(&edit_save, edit_items);
}


void mod_alarm_init()
{
	menu_add_entry("ALARM", NULL, NULL,
			&num_pressed,
			&star_long_pressed,
			NULL, NULL,
			&alarm_activated,
			&alarm_deactivated);

}
