/**
    modules/flyback.c: flyback stopwatch module for openchronos-ng

	Copyright (C) 2018 Ralf Horstmann <ralf@ackstorm.de>

    http://github.com/ra1fh/openchronos-ng-elf

    This file is part of openchronos-ng-elf.

    openchronos-ng-elf is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng-elf is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

/* flyback.c: flyback stopwatch with logging
 *
 * The flyback mode (menu text "FLYBK") is similar to a flyback
 * stopwatch with the additonal feature of storing timestamps and
 * counting the number of events (up to 19 events/timestamps). In
 * contrast to a regular stopwatch, the timer will automatically
 * restart on button press.
 *
 * Long press of the '*' button will clear all recorded timestamps and
 * reset the stopwatch to 0 and switch back to the start screen. Long
 * press of the '#' button deletes the last recorded timestamp and
 * adjusts the stopwatch to show time since latest remaining time
 * stamp. Short press of '#' will cycle through up to four different
 * screens. Each screen has it's own symbol in the middle symbol line:
 *
 * - counter screen (heart symbol)
 * - chrono screen (stopwatch symbol)
 * - list screen (record symbol)
 * - interval screen (alarm symbol)
 * 
 * The counter screen and the interval screen can be switched off at
 * compile time.
 *
 * On the counter and chrono screens, both the up and the down button
 * will record a new timestamp and restart the stopwatch from
 * zero. Which button was used to restart the timer is stored along
 * with the timestamp. This can be used to differentiate events
 * (start, stop, ...).
 *
 * On the list screen and interval screen, the up and down buttons
 * will scroll through the recorded timestamps or the list of time
 * intervals between recorded timestamps. No new timestamp will be
 * recorded when using the up and down buttons on these screens.
 *
 * The list of timestamps is kept in main memory and is persistent
 * accross mode switches. Also the stopwatch will continue to run when
 * using a different mode inbetween. But, timestamps will be lost
 * when removing the battery, reset or other power loss.
 *
 * The timestamps are stored by copying the current real time clock
 * (RTC) time into memory slots. The flyback stopwatch uses a
 * separate memory slot so that the stopwatch continues to work when
 * all memory slots are used.
 * 
 * When changing the time in the clock module, stored timestamps are
 * not adjusted. This means that the stopwatch view and interval times
 * to previously stored timestamps might become inaccurate.
 *
 * === 1. COUNTER SCREEN ===
 *
 * The upper line of the counter screen shows the current clock
 * time. The lower left two digits show the current number of recorded
 * timestamps. The right three digit of the lower line show the
 * stopwatch time in minutes and seconds. When exceeding ten minutes
 * the display will switch to hours and minutes, indicated by the "mi"
 * symbol to the right. When exceeding 10 hours, the display will
 * switch to two digits with hours and the letter 'h' in the third
 * digit. When exceeding 100 hours, the stopwatch will show '---'.
 *
 * When 19 records have been recorded, this will be indicated by the
 * "MAX" symbol in the lower part of the screen. The flyback stopwatch
 * function is still available, but no further timestamps will be
 * recorded.
 *
 * === 2. CHRONO SCREEN ===
 *
 * The chrono screen is similar to the counter screen. It will show the
 * current time in the first line. There is no counter in the second
 * line. All the digits of the second line are used to display a
 * higher precision stopwatch. Up to 20 hours it will show
 * hours:minutes:seconds. After 20 hours upto 100 hours it will switch
 * to four digits showing hours:minutes. When exceeding 100 hours the
 * display will show '-----'.
 *
 * The up and down buttons will restart the stopwatch and record the
 * current time in a memory slot (unless memory is full, indicated by
 * the 'MAX' symbol)
 *
 * === 3. LIST SCREEN ===
 *
 * The list screen shows the recorded timestamps. It starts with the
 * most recent timestamp. The first line shows hours and minutes, the
 * right to digits of the lower display line show the seconds. The
 * record number is shown in the left two digits of the second line.
 *
 * The up and down buttons can be used to scroll through the list of
 * timestamps.
 *
 * When no timestamp has been recorded yet, the screen will show all
 * zeros.
 *
 * In case the timestamp on display was recorded on a different day,
 * the record symbol will blink.
 *
 * === 4. INTERVAL SCREEN ===
 *
 * The interval screen shows time deltas between recorded timestamps.
 * It starts with the most recent time delta. The first line shows
 * hours and minutes. The two rightmost digits of the second line show
 * the seconds.
 *
 * The up and down buttons can be used to scroll through the list of
 * time deltas.
 *
 * When no timestamps have been recorded yet, the screen shows all
 * zeros. When only one timestamp has been recorded, the count will
 * show '1' and the rest of the display shows all zeros.
 *
 * The time delta view is limited to less than 100 hours. When
 * exceeding 100 hours between two timestamps, the interval will be
 * shown as '--:--'. The delta calculation is calendar correct accross
 * changes of month or year. E.g. you can record a timestamp on 28th
 * of February in a leap year and the next one 1st of March, the time
 * delta will be correct.
 *
 * === Caveats ===
 *
 * - There is currently no way to display the day, month or year of a
 *   recorded timestamp. A blinking record symbol will indicate that
 *   the timestamp on display was recorded on a different day though.
 *
 * - When adjusting the clock, the recorded timestamps are not being
 *   adjusted. So the time delta recorded after the clock change might
 *   in inaccurate, as well as the stopwatch time.
 *
 * - If for some reason the time delta calculation returns an error,
 *   the counter screen and chrono screen might show '-E-' where the
 *   stopwatch time is normally shown. This can happen if the
 *   conversion from RTC time to seconds returns a negative number or
 *   the difference between the values get negative.
 *
 * - The RTC to seconds conversion uses 32bit values, so it will
 *   overflow eventually. When exactly has yet to be determined.
 */

#include <string.h>
#include <time.h>

#include "messagebus.h"
#include "menu.h"

#include "drivers/rtca.h"
#include "drivers/display.h"
#include "drivers/timer.h"

enum {
#ifdef CONFIG_MOD_FLYBACK_ENABLE_COUNTER_SCREEN
	FLYBACK_COUNTER,
#endif
	FLYBACK_CHRONO,
	FLYBACK_LIST,
#ifdef CONFIG_MOD_FLYBACK_ENABLE_INTERVAL_SCREEN
	FLYBACK_INTERVAL,
#endif
	FLYBACK_END,
};

#define FLYBACK_FIRST_SCREEN 0

enum {
	FLYBACK_MARK_NONE, 
	FLYBACK_MARK_UP,
	FLYBACK_MARK_DOWN,
	FLYBACK_MARK_BOTH,
};

#define FLYBACK_MAX_TIMESTAMPS 19

#define TENMINUTES   (10L * 60)
#define TENHOURS     (10L * 60 * 60)
#define TWENTYHOURS  (20L * 60 * 60)
#define HUNDREDHOURS (100L * 60 * 60)

struct ts_s {
	uint16_t year;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t mark;
};

static struct flyback_state {
	struct ts_s ts[FLYBACK_MAX_TIMESTAMPS];
	struct ts_s chrono;   /* timstamp used for stopwatch start time       */
	time_t seconds;       /* stopwatch time in seconds                    */
	uint8_t count;        /* number of records in use                     */
	uint8_t display;      /* displayed record on list and interval screen */
	uint8_t mode;         /* active screen                                */
} flyback_state;

struct flyback_screen {
	void (*init)();                      /* initialization once on mode activation                 */
	void (*statechange)();               /* flyback_state changed (new record, display change ...) */
	void (*stopwatch)();                 /* stopwatch time changed                                 */
	void (*event)(enum sys_message msg); /* standard events                                        */
	void (*updown)(int mark);            /* up/down button pressed                                 */
};

static void flyback_stopwatch(); 
static void flyback_statechange();
static void flyback_make_tm(struct tm* tm, struct ts_s *ts);
static void flyback_copy_rtc(struct ts_s *ts, int mark);
static int flyback_diff_ts(struct ts_s *ts1, struct ts_s *ts2, time_t *s);
static void flyback_update_mark(int display, int mark);
static void flyback_state_record(int mark);
static void flyback_state_up();
static void flyback_state_down();
static void flyback_num_pressed();

/**********************************************************************/
/**************************** SCREENS *********************************/
/**********************************************************************/

/************************* counter screen *****************************/

#ifdef CONFIG_MOD_FLYBACK_ENABLE_COUNTER_SCREEN
static void flyback_counter_init()
{
	display_symbol(FLYBACK_COUNTER, LCD_ICON_HEART, SEG_ON);
	display_symbol(FLYBACK_COUNTER, LCD_SEG_L1_COL, SEG_ON);
	display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_ON);
	display_chars(FLYBACK_COUNTER, LCD_SEG_L2_3_0, " 000", SEG_SET);
}

static void flyback_counter_statechange()
{
	uint8_t mark;
	if (flyback_state.count == FLYBACK_MAX_TIMESTAMPS) {
		mark = FLYBACK_MARK_NONE;
	} else if (flyback_state.count) {
		mark = flyback_state.ts[flyback_state.count - 1].mark;
	} else {
		mark = FLYBACK_MARK_NONE;
	}
	flyback_update_mark(FLYBACK_COUNTER, mark);
	_printf(FLYBACK_COUNTER, LCD_SEG_L2_5_4, "%02u", flyback_state.count);
	display_symbol(FLYBACK_COUNTER, LCD_SYMB_MAX,
				   flyback_state.count == FLYBACK_MAX_TIMESTAMPS ? SEG_ON : SEG_OFF);
}

static void flyback_counter_event(enum sys_message msg)
{
	if (msg & SYS_MSG_RTC_SECOND) {
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L1_COL, ((rtca_time.sec & 0x01) ? SEG_ON : SEG_OFF));
	}
	if (msg & SYS_MSG_RTC_HOUR) {
		_printf(FLYBACK_COUNTER, LCD_SEG_L1_3_2, "%02u", rtca_time.hour);
	}
	if (msg & SYS_MSG_RTC_MINUTE) {
		_printf(FLYBACK_COUNTER, LCD_SEG_L1_1_0, "%02u", rtca_time.min);
	}
}

static void flyback_counter_stopwatch()
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;

	if (flyback_state.count == 0) {
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_5_4, "%02u", 0);
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_0, " 000", SEG_SET);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_OFF);
		flyback_update_mark(FLYBACK_COUNTER, FLYBACK_MARK_NONE);
		return;
	}
	if (flyback_state.seconds < 0) {
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_0, " -E-", SEG_SET);
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_OFF);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_OFF);
		return;
	}

	sec   = (flyback_state.seconds % 60);
	min   = (flyback_state.seconds / 60) % 60;
	hour  = (flyback_state.seconds / 60) / 60;

	if (flyback_state.seconds >= HUNDREDHOURS) {
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_0, " ---", SEG_SET);
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_OFF);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_OFF);
	} else if (flyback_state.seconds >= TENHOURS) {
		/* show hours as: _15h */
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_1, " %02u", hour);
		display_bits(FLYBACK_COUNTER, LCD_SEG_L2_0, 0x47, SEG_SET); // 'h'
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_OFF);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_OFF);
	} else if (flyback_state.seconds >= TENMINUTES) {
		/* show hours/minutes as: _9:59 */
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_2, " %1u", hour);
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_1_0, "%02u", min);
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_ON);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_ON);
	} else {
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_3_2, " %1u", min);
		_printf(FLYBACK_COUNTER, LCD_SEG_L2_1_0, "%02u", sec);
		display_symbol(FLYBACK_COUNTER, LCD_SEG_L2_COL0, SEG_ON);
		display_symbol(FLYBACK_COUNTER, LCD_UNIT_L2_MI, SEG_OFF);
	}
}

static void flyback_counter_updown(int mark)
{
	flyback_state_record(mark);
}
#endif

/************************** chrono screen ***************************/

static void flyback_chrono_init()
{
	display_symbol(FLYBACK_CHRONO, LCD_ICON_STOPWATCH, SEG_ON);
	display_symbol(FLYBACK_CHRONO, LCD_SEG_L1_COL, SEG_ON);
	display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL0, SEG_ON);
	display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL1, SEG_ON);
	display_bits(FLYBACK_CHRONO, LCD_SEG_L2_5, 0x00, SEG_SET);
	display_bits(FLYBACK_CHRONO, LCD_SEG_L2_4, 0x05, SEG_SET);
	_printf(FLYBACK_CHRONO, LCD_SEG_L2_5_4, "%02u", 0);
	_printf(FLYBACK_CHRONO, LCD_SEG_L2_3_2, "%02u", 0);
	_printf(FLYBACK_CHRONO, LCD_SEG_L2_1_0, "%02u", 0);
}

static void flyback_chrono_statechange()
{
	uint8_t mark;
	if (flyback_state.count == FLYBACK_MAX_TIMESTAMPS) {
		mark = FLYBACK_MARK_NONE;
	} else if (flyback_state.count) {
		mark = flyback_state.ts[flyback_state.count - 1].mark;
	} else {
		mark = FLYBACK_MARK_NONE;
	}
	flyback_update_mark(FLYBACK_CHRONO, mark);
	display_symbol(FLYBACK_CHRONO, LCD_SYMB_MAX,
				   flyback_state.count == FLYBACK_MAX_TIMESTAMPS ? SEG_ON : SEG_OFF);
}

static void flyback_chrono_event(enum sys_message msg)
{
	if (msg & SYS_MSG_RTC_SECOND) {
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L1_COL, ((rtca_time.sec & 0x01) ? SEG_ON : SEG_OFF));
	}
	if (msg & SYS_MSG_RTC_HOUR) {
		_printf(FLYBACK_CHRONO, LCD_SEG_L1_3_2, "%02u", rtca_time.hour);
	}
	if (msg & SYS_MSG_RTC_MINUTE) {
		_printf(FLYBACK_CHRONO, LCD_SEG_L1_1_0, "%02u", rtca_time.min);
	}
}

static void flyback_chrono_stopwatch()
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	
	if (flyback_state.count == 0) {
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_5_4, "%02u", 0);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_3_2, "%02u", 0);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_1_0, "%02u", 0);
		flyback_update_mark(FLYBACK_CHRONO, FLYBACK_MARK_NONE);
		return;
	}
	if (flyback_state.seconds < 0) {
		display_chars(FLYBACK_CHRONO, LCD_SEG_L2_5_0, " --E--", SEG_SET);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL0, SEG_OFF);
		return;
	}

	sec   = (flyback_state.seconds % 60);
	min   = (flyback_state.seconds / 60) % 60;
	hour  = (flyback_state.seconds / 60) / 60;

	if (flyback_state.seconds >= HUNDREDHOURS) {
		display_chars(FLYBACK_CHRONO, LCD_SEG_L2_5_0, " -----", SEG_SET);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL0, SEG_OFF);
	} else if (flyback_state.seconds >= TWENTYHOURS) {
		display_chars(FLYBACK_CHRONO, LCD_SEG_L2_5_4, "  ", SEG_SET);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_3_2, "%02u", hour);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_1_0, "%02u", min);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL0, SEG_ON);
	} else {
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_5_4, "%02u", hour);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_3_2, "%02u", min);
		_printf(FLYBACK_CHRONO, LCD_SEG_L2_1_0, "%02u", sec);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL1, SEG_ON);
		display_symbol(FLYBACK_CHRONO, LCD_SEG_L2_COL0, SEG_ON);
	}
}

static void flyback_chrono_updown(int mark)
{
	flyback_state_record(mark);
}	

/*************************** list screen **************************/

static void flyback_list_init()
{
	display_symbol(FLYBACK_LIST, LCD_ICON_RECORD, SEG_ON);
	display_symbol(FLYBACK_LIST, LCD_SEG_L1_COL, SEG_ON);
	display_symbol(FLYBACK_LIST, LCD_SEG_L2_COL0, SEG_ON);
}

static void flyback_list_statechange()
{
	if (flyback_state.count == 0) {
		_printf(FLYBACK_LIST, LCD_SEG_L1_3_2, "%02u", 0);
		_printf(FLYBACK_LIST, LCD_SEG_L1_1_0, "%02u", 0);
		_printf(FLYBACK_LIST, LCD_SEG_L2_1_0, "%02u", 0);
		_printf(FLYBACK_LIST, LCD_SEG_L2_5_4, "%02u", 0);
		flyback_update_mark(FLYBACK_LIST, FLYBACK_MARK_NONE);
		display_symbol(FLYBACK_LIST, LCD_ICON_RECORD, SEG_SET | BLINK_OFF);
	} else {
		_printf(FLYBACK_LIST, LCD_SEG_L1_3_2, "%02u", flyback_state.ts[flyback_state.display].hour);
		_printf(FLYBACK_LIST, LCD_SEG_L1_1_0, "%02u", flyback_state.ts[flyback_state.display].min);
		_printf(FLYBACK_LIST, LCD_SEG_L2_1_0, "%02u", flyback_state.ts[flyback_state.display].sec);
		_printf(FLYBACK_LIST, LCD_SEG_L2_5_4, "%02u", flyback_state.display + 1);
		flyback_update_mark(FLYBACK_LIST, flyback_state.ts[flyback_state.display].mark);
		if (flyback_state.ts[flyback_state.display].day != rtca_time.day ||
			flyback_state.ts[flyback_state.display].mon != rtca_time.mon ||
			flyback_state.ts[flyback_state.display].year != rtca_time.year) {
			display_symbol(FLYBACK_LIST, LCD_ICON_RECORD, SEG_SET | BLINK_ON);
		} else {
			display_symbol(FLYBACK_LIST, LCD_ICON_RECORD, SEG_SET | BLINK_OFF);
		}
	}
	display_symbol(FLYBACK_LIST, LCD_SYMB_MAX,
				   flyback_state.count == FLYBACK_MAX_TIMESTAMPS ? SEG_ON : SEG_OFF);
}

static void flyback_list_updown(int mark)
{
	if (mark == FLYBACK_MARK_UP)
		flyback_state_up();
	else
		flyback_state_down();
}

/************************* interval screen ************************/

#ifdef CONFIG_MOD_FLYBACK_ENABLE_INTERVAL_SCREEN
static void flyback_interval_init()
{
	display_symbol(FLYBACK_INTERVAL, LCD_ICON_ALARM, SEG_ON);
	display_symbol(FLYBACK_INTERVAL, LCD_SEG_L1_COL, SEG_ON);
	display_symbol(FLYBACK_INTERVAL, LCD_SEG_L2_COL0, SEG_ON);
}

static void flyback_interval_statechange()
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	time_t seconds;
	int res;

	if (flyback_state.count == 0) {
		_printf(FLYBACK_INTERVAL, LCD_SEG_L2_5_4, "%02u", 0);
		flyback_update_mark(FLYBACK_INTERVAL, FLYBACK_MARK_NONE);
	} else {
		_printf(FLYBACK_INTERVAL, LCD_SEG_L2_5_4, "%02u", flyback_state.display + 1);
		flyback_update_mark(FLYBACK_INTERVAL, flyback_state.ts[flyback_state.display].mark);
	}
	display_char(FLYBACK_INTERVAL, LCD_SEG_L2_3, ' ', SEG_SET);
	display_char(FLYBACK_INTERVAL, LCD_SEG_L2_2, ' ', SEG_SET);
	display_symbol(FLYBACK_INTERVAL, LCD_SYMB_MAX,
				   flyback_state.count == FLYBACK_MAX_TIMESTAMPS ? SEG_ON : SEG_OFF);

	if (flyback_state.display == 0) {
		_printf(FLYBACK_INTERVAL, LCD_SEG_L1_3_2, "%02u", 0);
		_printf(FLYBACK_INTERVAL, LCD_SEG_L1_1_0, "%02u", 0);
		_printf(FLYBACK_INTERVAL, LCD_SEG_L2_1_0, "%02u", 0);
		return;
	}

	res = flyback_diff_ts(&flyback_state.ts[flyback_state.display - 1],
	                     &flyback_state.ts[flyback_state.display],
	                     &seconds);

	if (res != 0 || seconds >= HUNDREDHOURS) {
		display_chars(FLYBACK_INTERVAL, LCD_SEG_L1_3_0, "----", SEG_SET);
		display_chars(FLYBACK_INTERVAL, LCD_SEG_L2_2_0,   "--", SEG_SET);
		return;
	}

	sec   = (seconds % 60);
	min   = (seconds / 60) % 60;
	hour  = (seconds / 60) / 60;

	_printf(FLYBACK_INTERVAL, LCD_SEG_L1_3_2, "%02u", hour);
	_printf(FLYBACK_INTERVAL, LCD_SEG_L1_1_0, "%02u", min);
	_printf(FLYBACK_INTERVAL, LCD_SEG_L2_1_0, "%02u", sec);
}

static void flyback_interval_updown(int mark)
{
	if (mark == FLYBACK_MARK_UP)
		flyback_state_up();
	else
		flyback_state_down();
}
#endif

static struct flyback_screen flyback_screens[] = {
#ifdef CONFIG_MOD_FLYBACK_ENABLE_COUNTER_SCREEN
	{
		.init        = flyback_counter_init,
		.statechange = flyback_counter_statechange,
		.stopwatch 	 = flyback_counter_stopwatch,
		.event     	 = flyback_counter_event,
		.updown    	 = flyback_counter_updown,
	},
#endif
	{
		.init        = flyback_chrono_init,
		.statechange = flyback_chrono_statechange,
		.stopwatch 	 = flyback_chrono_stopwatch,
		.event     	 = flyback_chrono_event,
		.updown    	 = flyback_chrono_updown,
	},
	{
		.init        = flyback_list_init,
		.statechange = flyback_list_statechange,
		.event       = NULL,
		.stopwatch   = NULL,
		.updown      = flyback_list_updown,
	},
#ifdef CONFIG_MOD_FLYBACK_ENABLE_INTERVAL_SCREEN
	{
		.init        = flyback_interval_init,
		.statechange = flyback_interval_statechange,
		.event     	 = NULL,
		.stopwatch 	 = NULL,
		.updown    	 = flyback_interval_updown,
	},
#endif
};

/**********************************************************************/
/************************ HELPER FUNCTIONS ****************************/
/**********************************************************************/

static void flyback_make_tm(struct tm* tm, struct ts_s *ts)
{
	tm->tm_year = ts->year - 1900;
	tm->tm_mon = ts->mon - 1;
	tm->tm_mday = ts->day;
	tm->tm_hour = ts->hour;
	tm->tm_min = ts->min;
	tm->tm_sec = ts->sec;
	tm->tm_wday = 0;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;
}

static void flyback_copy_rtc(struct ts_s *ts, int mark)
{
	ts->year = rtca_time.year;
	ts->mon = rtca_time.mon;
	ts->day = rtca_time.day;
	ts->hour = rtca_time.hour;
	ts->min = rtca_time.min;
	ts->sec = rtca_time.sec;
	ts->mark = mark;
}

static int flyback_diff_ts(struct ts_s *ts1, struct ts_s *ts2, time_t *s)
{
	struct tm tm1, tm2;
	time_t t1, t2;
	time_t seconds;

	flyback_make_tm(&tm1, ts1);
	flyback_make_tm(&tm2, ts2);
	t1 = mktime(&tm1);
	t2 = mktime(&tm2);

	if (t1 < 0 || t2 < 0) {
		return -1;
	}

	seconds = t2 - t1;
	if (seconds < 0) {
		return -1;
	}

	*s = seconds;
	return 0; /* success */
}

static void flyback_update_mark(int display, int mark)
{
	switch(mark) {
	case FLYBACK_MARK_UP:
		display_symbol(display, LCD_SYMB_ARROW_UP, SEG_ON);
		display_symbol(display, LCD_SYMB_ARROW_DOWN, SEG_OFF);
		break;
	case FLYBACK_MARK_DOWN:
		display_symbol(display, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(display, LCD_SYMB_ARROW_DOWN, SEG_ON);
		break;
	case FLYBACK_MARK_BOTH:
		display_symbol(display, LCD_SYMB_ARROW_UP, SEG_ON);
		display_symbol(display, LCD_SYMB_ARROW_DOWN, SEG_ON);
		break;
	default:
		display_symbol(display, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(display, LCD_SYMB_ARROW_DOWN, SEG_OFF);
		break;
	}
}

static void flyback_state_record(int mark)
{
	flyback_copy_rtc(&flyback_state.chrono, FLYBACK_MARK_NONE);
	if (flyback_state.count < FLYBACK_MAX_TIMESTAMPS) {
		flyback_copy_rtc(&flyback_state.ts[flyback_state.count], mark);
		flyback_state.count++;
	}
	flyback_stopwatch();
	flyback_statechange();
}

static void flyback_state_up()
{
	if (flyback_state.display + 1 < flyback_state.count)
		flyback_state.display++;
	flyback_statechange();
}

static void flyback_state_down()
{
	if (flyback_state.display > 0)
		flyback_state.display--;
	flyback_statechange();
}

static void flyback_statechange()
{
	if (flyback_screens[flyback_state.mode].statechange)
		flyback_screens[flyback_state.mode].statechange();
}

static void flyback_stopwatch()
{
	struct ts_s now;
	int res = 0;
	if (flyback_state.count > 0) {
		flyback_copy_rtc(&now, 0);
		res = flyback_diff_ts(&flyback_state.chrono, &now, &flyback_state.seconds);
		if (res < 0)
			flyback_state.seconds = -1;
	}
	if (flyback_screens[flyback_state.mode].stopwatch)
		flyback_screens[flyback_state.mode].stopwatch();
}

static void flyback_event(enum sys_message msg)
{
	if (flyback_screens[flyback_state.mode].event)
		flyback_screens[flyback_state.mode].event(msg);
	if (msg & SYS_MSG_RTC_SECOND)
		flyback_stopwatch();
}

/**********************************************************************/
/************************ MENU CALLBACKS ******************************/
/**********************************************************************/
static void flyback_activate()
{
	lcd_screens_create(FLYBACK_END);
	for (int i = 0; i < FLYBACK_END; i++) {
		if (flyback_screens[i].init) {
			flyback_screens[i].init();
		}
	}
	flyback_state.display = 0;
	flyback_state.mode = FLYBACK_END - 1;
	flyback_num_pressed();
	
	sys_messagebus_register(&flyback_event,
	                        SYS_MSG_RTC_HOUR | SYS_MSG_RTC_MINUTE | SYS_MSG_RTC_SECOND
	);
}

static void flyback_deactivate()
{
	sys_messagebus_unregister_all(&flyback_event);

	/* destroy virtual screens */
	lcd_screens_destroy();

	/* clean up screen */
	display_clear(FLYBACK_FIRST_SCREEN, 0);
}

static void flyback_num_pressed()
{
	if (++flyback_state.mode == FLYBACK_END) {
		flyback_state.mode = FLYBACK_FIRST_SCREEN;
	}
	lcd_screen_activate(flyback_state.mode);

	if (flyback_state.count > 0) {
		flyback_state.display = flyback_state.count - 1;
	} else {
		flyback_state.display = 0;
	}

	flyback_statechange();
	flyback_event(SYS_MSG_RTC_HOUR | SYS_MSG_RTC_MINUTE | SYS_MSG_RTC_SECOND);
}

static void flyback_reset_all()
{
	flyback_state.count = 0;
	flyback_state.display = 0;
	flyback_state.mode = FLYBACK_END - 1;
	flyback_num_pressed();
}

static void flyback_reset_one()
{
	static uint16_t flyback_last_press = 0;

	/* After CONFIG_BUTTONS_LONG_PRESS_TIME the longpress callbacks
	 * get called every 50ms, which will quickly delete all entries.
	 * To prevent that, store the 20Hz counter value and make sure
	 * there is a little time between invocations.
	 */
	if (timer0_20hz_counter - flyback_last_press < CONFIG_BUTTONS_LONG_PRESS_TIME) {
		flyback_last_press = timer0_20hz_counter;
		return;
	} else {
		flyback_last_press = timer0_20hz_counter;
	}

	if (flyback_state.count > 0) {
		flyback_state.count--;
		if (flyback_state.count > 0) {
			flyback_state.display = flyback_state.count - 1;
			memcpy(&flyback_state.chrono,
			       &flyback_state.ts[flyback_state.count -1],
			       sizeof(flyback_state.chrono));
		} else {
			flyback_state.display = 0;
		}
		flyback_stopwatch();
		flyback_statechange();
	}
}

static void flyback_up_pressed()
{
	if (flyback_screens[flyback_state.mode].updown)
		flyback_screens[flyback_state.mode].updown(FLYBACK_MARK_UP);
}

static void flyback_down_pressed()
{
	if (flyback_screens[flyback_state.mode].updown)
		flyback_screens[flyback_state.mode].updown(FLYBACK_MARK_DOWN);
}

void mod_flyback_init()
{
	menu_add_entry("FLYBK",
	               &flyback_up_pressed,   /* up         */
	               &flyback_down_pressed, /* down       */
	               &flyback_num_pressed,  /* num        */
	               &flyback_reset_all,    /* star long  */
	               &flyback_reset_one,    /* num long   */
	               NULL,                  /* up + down  */
	               &flyback_activate,     /* activate   */
	               &flyback_deactivate);  /* deactivate */
}
