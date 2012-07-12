/*
    modules/clock.c: clock module for openchronos-ng

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

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
#include <drivers/rtca.h>
#include <drivers/display.h>

static uint16_t tmp_yy;
static uint8_t tmp_mo, tmp_dw, tmp_dd, tmp_hh, tmp_mm, tmp_ss;
static char const * tmp_dws;

static void clock_event(enum sys_message msg)
{
	rtca_get_time(&tmp_hh, &tmp_mm, &tmp_ss);
	rtca_get_date(&tmp_yy, &tmp_mo, &tmp_dd, &tmp_dw, &tmp_dws);

	if (msg | SYS_MSG_RTC_YEAR)
		_printf(1, LCD_SEG_L1_3_0, "%04u", tmp_yy);
#ifdef CONFIG_CLOCK_MONTH_FIRST
	if (msg | SYS_MSG_RTC_MONTH)
		_printf(0, LCD_SEG_L2_4_3, "%02u", tmp_mo);
	if (msg | SYS_MSG_RTC_DAY) {
		_printf(0, LCD_SEG_L2_1_0, "%02u", tmp_dd);
#else
	if (msg | SYS_MSG_RTC_MONTH)
		_printf(0, LCD_SEG_L2_1_0, "%02u", tmp_dd);
	if (msg | SYS_MSG_RTC_DAY) {
		_printf(0, LCD_SEG_L2_4_3, "%02u", tmp_mo);

#endif
		_printf(1, LCD_SEG_L2_2_0, tmp_dws, SEG_SET);
	}
	if (msg | SYS_MSG_RTC_HOUR) {
#ifdef CONFIG_CLOCK_AMPM
		if (tmp_hh > 12) {
			tmp_hh = tmp_hh - 12;
			display_symbol(0,LCD_SYMB_AM,SEG_OFF);
			display_symbol(0,LCD_SYMB_PM,SEG_SET);
		} else {
			if (tmp_hh == 12) {
				display_symbol(0,LCD_SYMB_AM,SEG_OFF);
				display_symbol(0,LCD_SYMB_PM,SEG_SET);
			} else {
				display_symbol(0,LCD_SYMB_PM,SEG_OFF);
				display_symbol(0,LCD_SYMB_AM,SEG_SET);
			}
			if (tmp_hh == 0) {
				tmp_hh = 12;
			}
		}
		_printf(0, LCD_SEG_L1_3_2, "%2u", tmp_hh);
#else
		_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
#endif
	}
	if (msg | SYS_MSG_RTC_MINUTE)
		_printf(0, LCD_SEG_L1_1_0, "%02u", tmp_mm);
}

/********************* edit mode callbacks ********************************/
static void edit_yy_sel(void)
{
	lcd_screen_activate(1);
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_ON);
}
static void edit_yy_dsel(void)
{
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
}
static void edit_yy_set(int8_t step)
{
	/* this allows setting years between 2012 and 2022 */
	*((uint8_t *)&tmp_yy + 1) = 0x07;
	helpers_loop((uint8_t *)&tmp_yy, 220, 230, step);

	_printf(1, LCD_SEG_L1_3_0, "%04u", tmp_yy);
}

static void edit_mo_sel(void)
{
	lcd_screen_activate(0);
#ifdef CONFIG_CLOCK_MONTH_FIRST
	display_chars(0, LCD_SEG_L2_4_3, NULL, BLINK_ON);
#else
	display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_ON);
#endif
}
static void edit_mo_dsel(void)
{
#ifdef CONFIG_CLOCK_MONTH_FIRST
	display_chars(0, LCD_SEG_L2_4_3, NULL, BLINK_OFF);
#else
	display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_OFF);
#endif
}

static void edit_mo_set(int8_t step)
{
	helpers_loop(&tmp_mo, 1, 12, step);
#ifdef CONFIG_CLOCK_MONTH_FIRST
	_printf(0, LCD_SEG_L2_4_3, "%02u", tmp_mo);
#else
	_printf(0, LCD_SEG_L2_1_0, "%02u", tmp_mo);
#endif
}

static void edit_dd_sel(void)
{
	lcd_screen_activate(0);
#ifdef CONFIG_CLOCK_MONTH_FIRST
	display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_ON);
#else
	display_chars(0, LCD_SEG_L2_4_3, NULL, BLINK_ON);
#endif
}

static void edit_dd_dsel(void)
{
#ifdef CONFIG_CLOCK_MONTH_FIRST
	display_chars(0, LCD_SEG_L2_1_0, NULL, BLINK_OFF);
#else
	display_chars(0, LCD_SEG_L2_4_3, NULL, BLINK_OFF);
#endif
}

static void edit_dd_set(int8_t step)
{
	helpers_loop(&tmp_dd, 1, rtca_get_max_days(tmp_mo, tmp_yy), step);
#ifdef CONFIG_CLOCK_MONTH_FIRST
	_printf(0, LCD_SEG_L2_1_0, "%02u", tmp_dd);
#else
	_printf(0, LCD_SEG_L2_4_3, "%02u", tmp_dd);
#endif
}

static void edit_mm_sel(void)
{
	lcd_screen_activate(0);
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

static void edit_hh_sel(void)
{
	lcd_screen_activate(0);
	display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_ON);
}
static void edit_hh_dsel(void)
{
	display_chars(0, LCD_SEG_L1_3_2, NULL, BLINK_OFF);
}
static void edit_hh_set(int8_t step)
{
	helpers_loop(&tmp_hh, 0, 23, step);
#ifdef CONFIG_CLOCK_AMPM
	if (tmp_hh > 12) {
		display_symbol(0,LCD_SYMB_AM,SEG_OFF);
		display_symbol(0,LCD_SYMB_PM,SEG_SET);
		_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh-12);
	} else {
		if (tmp_hh == 0) {
			_printf(0, LCD_SEG_L1_3_2, "%02u", 12);
		} else {
			if (tmp_hh > 9) {
				_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
			} else {
				_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
			}
		}
		if (tmp_hh == 12) {
			display_symbol(0,LCD_SYMB_AM,SEG_OFF);
			display_symbol(0,LCD_SYMB_PM,SEG_SET);
		} else {
			display_symbol(0,LCD_SYMB_PM,SEG_OFF);
			display_symbol(0,LCD_SYMB_AM,SEG_SET);
		}
	}
#else
	_printf(0, LCD_SEG_L1_3_2, "%02u", tmp_hh);
#endif
}

static void edit_save()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_time(tmp_hh, tmp_mm, 0);
	rtca_set_date(tmp_yy, tmp_mo, tmp_dd);

	/* turn off only SOME blinking segments */
	display_chars(0, LCD_SEG_L1_3_0, NULL, BLINK_OFF);
	display_chars(0, LCD_SEG_L2_4_0, NULL, BLINK_OFF);
	display_chars(1, LCD_SEG_L1_3_0, NULL, BLINK_OFF);

	/* return to main screen */
	lcd_screen_activate(0);

	/* start the RTC */
	rtca_start();
}

/* edit mode item table */
static struct menu_editmode_item edit_items[] = {
	{&edit_yy_sel, &edit_yy_dsel, &edit_yy_set},
	{&edit_mo_sel, &edit_mo_dsel, &edit_mo_set},
	{&edit_dd_sel, &edit_dd_dsel, &edit_dd_set},
	{&edit_hh_sel, &edit_hh_dsel, &edit_hh_set},
	{&edit_mm_sel, &edit_mm_dsel, &edit_mm_set},
	{ NULL },
};

/************************ menu callbacks **********************************/
static void clock_activated()
{
	sys_messagebus_register(&clock_event, SYS_MSG_RTC_MINUTE \
													| SYS_MSG_RTC_HOUR \
													| SYS_MSG_RTC_DAY \
													| SYS_MSG_RTC_MONTH);	
	
	/* create two screens, the first is always the active one */
	lcd_screens_create(2);

	/* display stuff that won't change with time */
#ifdef CONFIG_CLOCK_BLINKCOL
	display_symbol(0, LCD_SEG_L1_COL, SEG_ON | BLINK_ON);
#else
	display_symbol(0, LCD_SEG_L1_COL, SEG_ON);
#endif
	display_char(0, LCD_SEG_L2_2, '-', SEG_SET);

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
	lcd_screens_destroy();

	/* clean up screen */
	display_symbol(0, LCD_SEG_L1_COL, BLINK_OFF);
#ifdef CONFIG_CLOCK_AMPM
	display_symbol(0,LCD_SYMB_AM,SEG_OFF);
	display_symbol(0,LCD_SYMB_PM,SEG_OFF);
#endif
	display_clear(0, 1);
	display_clear(0, 2);
}


/* Num button press callback */
static void num_pressed()
{
	lcd_screen_activate(0xff);
}

/* Star button long press callback. */
static void star_long_pressed()
{
	/* stop the hardware RTC */
	rtca_stop();

	/* Save the current time in edit_buffer */
	rtca_get_time(&tmp_hh, &tmp_mm, &tmp_ss);
	rtca_get_date(&tmp_yy, &tmp_mo, &tmp_dd, &tmp_dw, &tmp_dws);

	menu_editmode_start(&edit_save, edit_items);
}

void clock_init()
{
	menu_add_entry("CLOCK", NULL, NULL,
			&num_pressed,
			&star_long_pressed,
			NULL, NULL,
			&clock_activated,
			&clock_deactivated);

}
