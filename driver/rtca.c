/*
    rtca.c: TI CC430 Hardware Realtime Clock (RTC_A)

    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>

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
/* TODO: The RTC supports chronologic alarms, that is, one can program the
         alarm to bell every hour, or in a specific time and day. For now only
	 basic alarm is implemented (bell at 08:30). */

#include "rtca.h"

#include <stdlib.h>

// stores callback list of functions to call when a time event occurrs
static rtca_cblist_t *cblist = NULL;

// stores system time = number of seconds since power on
// and a cache of RTC registers
// TODO: pack this stuff to save memory
static struct {
	u32 sys;
	u16 year;
	u8 mon;
	u8 day;
	u8 dow;
	u8 hour;
	u8 min;
	u8 sec;
} rtca_time = { 0, 0, 1, 1, 0, 0, 0, 0 };

void rtca_init(void)
{
	// Enable calendar mode (date/time registers are automatically reset)
	// and enable read ready interrupts
	// and set time event interrupts each minute (when enabled)
	// also enable alarm interrupts
	RTCCTL01 |= RTCMODE | RTCRDYIE | RTCAIE;

	// Enable the RTC
	RTCCTL01 &= ~RTCHOLD;
}

void rtca_tevent_fn_register(rtca_tevent_fn_t fn)
{
	rtca_cblist_t **p = &cblist;

	while (*p) {
		p = &(*p)->next;
	}

	// disable interrupts for critical section
	RTCCTL01 &= ~RTCTEVIE;

	// add new node to list
	*p = malloc(sizeof(rtca_cblist_t));
	(*p)->next = NULL;
	(*p)->fn = fn;

	// re-enable minute interrupts
	RTCCTL01 |= RTCTEVIE;
}

void rtca_tevent_fn_unregister(rtca_tevent_fn_t fn)
{
	rtca_cblist_t *p = cblist, *pp = NULL;

	while (p) {
		if (p->fn == fn) {
			if (!pp)
				cblist = p->next;
			else
				pp->next = p->next;

			free(p);
		}

		pp = p;
		p = p->next;
	}

	// disable interrupts if callback list is empty
	if (! cblist)
		RTCCTL01 &= ~RTCTEVIE;
}

// *************************************************************************************************
// @fn          rtca_get_max_days
// @brief       Return number of days for a given month
// @param       month		month as char
//		year		year as int
// @return      day count for given month
// *************************************************************************************************
u8 rtca_get_max_days(u8 month, u16 year)
{
	switch (month) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return (31);

		case 4:
		case 6:
		case 9:
		case 11:
			return (30);

			// 1. A year that is divisible by 4 is a leap year. (Y % 4) == 0
			// 2. Exception to rule 1: a year that is divisible by 100 is not a leap year. (Y % 100) != 0
			// 3. Exception to rule 2: a year that is divisible by 400 is a leap year. (Y % 400) == 0
		case 2:
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
				return (29);
			else
				return (28);

		default:
			return (0);
	}
}

u32 rtca_get_systime(void)
{
	return rtca_time.sys;
}

void rtca_get_time(u8 *hour, u8 *min, u8 *sec)
{
	*sec = rtca_time.sec;
	*min = rtca_time.min;
	*hour = rtca_time.hour;
}

void rtca_set_time(u8 hour, u8 min, u8 sec)
{
	// Stop RTC timekeeping for a while
	RTCCTL01 |= RTCHOLD;

	// update RTC registers
	RTCSEC = (rtca_time.sec = sec);
	RTCMIN = (rtca_time.min = min);
	RTCHOUR = (rtca_time.hour = hour);

	// Resume RTC time keeping
	RTCCTL01 &= ~RTCHOLD;
}

void rtca_get_alarm(u8 *hour, u8 *min)
{
	*hour = RTCAHOUR & 0x7F;
	*min  = RTCAMIN  & 0x7F;
}

void rtca_set_alarm(u8 hour, u8 min)
{
	RTCAHOUR = hour & 0x7F;
	RTCAMIN  = min  & 0x7F;
}

void rtca_enable_alarm()
{
	RTCAHOUR |= 0x80;
	RTCAMIN  |= 0x80;
}

void rtca_disable_alarm()
{
	RTCAHOUR &= 0x7F;
	RTCAMIN  &= 0x7F;
}

void rtca_get_date(u16 *year, u8 *mon, u8 *day, u8 *dow)
{
	*dow = rtca_time.dow;
	*day = rtca_time.day;
	*mon = rtca_time.mon;
	*year = rtca_time.year;
}

void rtca_set_date(u16 year, u8 mon, u8 day)
{
	u8 dow;
	// Stop RTC timekeeping for a while
	RTCCTL01 |= RTCHOLD;


#define BASE_YEAR 1984 // not a leap year, so no need to add 1

	dow = (year - BASE_YEAR) + (year - BASE_YEAR) / 4; // compute number of leap years since BASE_YEAR

	if ((29 == rtca_get_max_days(2, year)) && (mon < 3))
		dow--; // if this is a leap year but before February 29

	dow += day; // add day of current month

	//add this month's dow value
	switch (mon) {
		case 5:
			dow += 1;
			break;

		case 8:
			dow += 2;
			break;

		case 2:
		case 3:
		case 11:
			dow += 3;
			break;

		case 6:
			dow += 4;
			break;

		case 9:
		case 12:
			dow += 5;
			break;

		case 4:
		case 7:
			dow += 6;
			break;

		default:  //January and October
			break;
	}

	dow = dow % 7;

	// update RTC registers and local cache
	RTCDAY = (rtca_time.day = day);
	RTCDOW = (rtca_time.dow = dow);
	RTCMON = (rtca_time.mon = mon);
	rtca_time.year = year;
	RTCYEARL = year & 0xff;
	RTCYEARH = year >> 8;

	// Resume RTC time keeping
	RTCCTL01 &= ~RTCHOLD;
}

#ifdef __GNUC__
#include <legacymsp430.h>
interrupt(RTC_A_VECTOR) RTC_A_ISR(void)
{
#else
#pragma vector = RTC_A_VECTOR
__interrupt void RTC_A_ISR(void)
{
#endif
	uint16_t iv = RTCIV;

	// copy register values
	rtca_time.sec = RTCSEC;

	// increment system time
	rtca_time.sys++;

	// only continue on time event or alarm event
	if (iv != RTCIV_RTCTEVIFG && iv != RTCIV_RTCAIFG)
		return;

	rtca_tevent_ev_t ev = RTCA_EV_ALARM;

	if (iv == RTCIV_RTCTEVIFG) {	//Minute changed!
		ev = RTCA_EV_MINUTE;
		rtca_time.min = RTCMIN;

		// Possible values:
		// 0 - minute changed
		// 1 - hour changed
		// 2 - day changed
		// 3 - month changed
		// 4 - year changed
		if (rtca_time.min == 0) {				//  Hour changed
			ev++;
			rtca_time.hour = RTCHOUR;

			if (rtca_time.hour == 0) {	    		// Day changed
				ev++;
				rtca_time.day = RTCDAY;
				rtca_time.dow = RTCDOW;

				if (rtca_time.day == 1) {	    	// Month changed - day zero doesn't exist
					ev++;
					rtca_time.mon = RTCMON;

					if (rtca_time.mon == 1) {	// Year changed - month zero doesn't exist
						ev++;
						rtca_time.year = RTCYEARL | (RTCYEARH << 8);
					}
				}
			}
		}
	}

	// call event handlers
	rtca_cblist_t *p = cblist;

	while (p) {
		p->fn(ev);
		p = p->next;
	}
}

