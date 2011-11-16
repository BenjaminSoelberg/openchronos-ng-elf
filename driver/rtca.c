/*
    rtca.c: TI CC430 Hardware Realtime Clock (RTC_A)
 
    Copyright (C) 2011 Angelo Arrifano <miknix@gmail.com>

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
	RTCCTL01 |= RTCMODE | RTCRDYIE;

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

	// re-enable interrupts
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
	RTCSEC = sec;
	RTCMIN = min;
	RTCHOUR = hour;
}

void rtca_get_date(u16 *year, u8 *mon, u8 *day, u8 *dow)
{
	*dow = rtca_time.dow;
	*day = rtca_time.day;
	*mon = rtca_time.mon;
	*year = rtca_time.year;
}

void rtca_set_date(u16 year, u8 mon, u8 day, u8 dow)
{
	RTCDOW = dow;
	RTCDAY = day;
	RTCMON = mon;
	RTCYEARL = year & 0xff;
	RTCYEARH = year >> 8;
}

#ifdef __GNUC__
#include <legacymsp430.h>
interrupt (RTC_A_VECTOR) RTC_A_ISR(void)
#else
#pragma vector = RTC_A_VECTOR
__interrupt void RTC_A_ISR(void)
#endif
{
	// this register read will clear the latest pending interrupt flag
	u8 IRS = (RTCIV & 0xff);

	if (IRS == RTCIV_RTCRDYIFG) {
		// copy register values
		rtca_time.sec = RTCSEC;
		rtca_time.min = RTCMIN;
		rtca_time.hour = RTCHOUR;
		rtca_time.dow = RTCDOW;
		rtca_time.day = RTCDAY;
		rtca_time.mon = RTCMON;
		rtca_time.year = RTCYEARL | (RTCYEARH << 8);
	
		// increment system time
		rtca_time.sys++;
	} else if (IRS == RTCIV_RTCTEVIFG) {
		// just in case...
		if (! cblist)
			return;

		// for now we only have one interrupt event enabled,
		// so we don't need to check the interrupt source.
		rtca_cblist_t *p = cblist;
		while (p) {
			p->fn( (RTCCTL01 & (RTCTEV1 | RTCTEV0)) >> 8 );
			p = p->next;
		};
	}
}

