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

// stores function to call when a time event occurrs
static rtca_tevent_fn_t tevent_fn = NULL;

void rtca_init()
{
	// Enable calendar mode (date/time registers are automatically reset)
	// and enable read ready interrupts
	RTCCTL01 |= RTCMODE;

	// Enable the RTC
	RTCCTL01 &= ~RTCHOLD;
}

// sets the tick function (function that is called at a time event)
// set fn to NULL to clear
void rtca_set_tevent_fn(rtca_tevent_fn_t fn)
{
	if (fn) {
		// we have to set this BEFORE enabling the interrupt
		tevent_fn = fn;
		// enable interrupts
		RTCCTL01 |= RTCTEVIE;
	} else {
		// disable interrupts
		RTCCTL01 &= ~RTCTEVIE;
		// we have to set this AFTER disabling the interrupt
		tevent_fn = NULL;
	}
}

void rtca_get_time(u8 *hour, u8 *min, u8 *sec)
{
	// wait until read is ready (avoid reading invalid time)
	while (! (RTCCTL01 & RTCRDY)) {
		__delay_cycles(2);
	}
	
	*sec = RTCSEC;
	*min = RTCMIN;
	*hour = RTCHOUR;
}

void rtca_set_time(u8 hour, u8 min, u8 sec)
{
	RTCSEC = sec;
	RTCMIN = min;
	RTCHOUR = hour;
}

void rtca_get_date(u16 *year, u8 *mon, u8 *day, u8 *dow)
{
	// wait until read is ready (avoid reading invalid time)
	while (! (RTCCTL01 & RTCRDY)) {
		__delay_cycles(2);
	}

	*dow = RTCDOW;
	*day = RTCDAY;
	*mon = RTCMON;
	*year = RTCYEARL | (RTCYEARH << 8);
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
	// interrupt is serviced, clear interrupt
	RTCCTL01 &= ~RTCTEVIFG;

	// for now we only have one interrupt event enabled,
	// so we don't need to check the interrupt source.
	tevent_fn( (RTCCTL01 & (RTCTEV1 | RTCTEV0)) >> 8 );
}

