/*
    Daylight Saving Time for OpenChronos on the TI ez430 chronos watch.
    Copyright 2011 Rick Miller <rdmiller3@gmail.com>

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
#include "project.h"

#if (CONFIG_DST > 0)
/* "private" defines */
#define N_SUN_OF_MON(n,mon,year) (((n)*7)-dst_day_of_week((year),(mon),((n)*7)))
#define LAST_SUN_OF_MON(mon,days,year) ((days)-dst_day_of_week((year),(mon),(days)))
#define DSTNUM(x,y) (((uint16_t)(x)*100)+(uint16_t)(y))

/* driver */
#include "rtca.h"

/* logic */
#include "dst.h"

/* "private" functions */
void dst_event(rtca_tevent_ev_t ev);
uint8_t dst_isDateInDST(uint8_t month, uint8_t day);





struct dst_date_struct dst_dates[2];
uint8_t dst_state;

uint8_t dst_day_of_week(uint16_t year, uint8_t month, uint8_t day);


/****************************************************************************/
/* DST initialize function. This is meant to be called on application init. */
/****************************************************************************/
void dst_init(void)
{
	uint8_t day, month, dow;
	uint16_t year;

	rtca_get_date(&year, &month, &day, &dow);
	/* Calculate when to switch dates */
	dst_calculate_dates(year, month, day);
	/* Register callback */
	rtca_tevent_fn_register(&dst_event);
}

/******************************************************************************/
/* This function is called on RTC_A 1/min interval. If hour hasn't changed it */
/*     immediately returns. If hour changed, then it adjusts time for DST     */
/*     accordingly.                                                           */
/******************************************************************************/
void dst_event(rtca_tevent_ev_t ev)
{
	if (ev < RTCA_EV_HOUR) // needs only 1/hour servicing
		return;

	uint8_t hour, minute, second;
	uint8_t day, month, dow;
	uint16_t year;

	rtca_get_time(&hour, &minute, &second);
	rtca_get_date(&year, &month, &day, &dow);

	/* check if we have to re-calculate DST switch days. */
	if (ev == RTCA_EV_YEAR)
		dst_calculate_dates(year, month, day);

	if ((hour == 1) && dst_isDateInDST(month, day) && (dst_state == DST_STATE_ST)) {
		/* spring forward */
		hour++;
		dst_state = DST_STATE_DST;
	}

	if ((hour == 2) && (!dst_isDateInDST(month, day)) && (dst_state != DST_STATE_ST)) {
		/* fall back */
		hour--;
		dst_state = DST_STATE_ST;
	}

	rtca_set_time(hour, minute, second);
}

/********************************************************************************/
/* This function calculates the new date for DST switching. It should be called */
/* once a year.                                                                 */
/* @param uint8_t month The current month.                                           */
/* @param uint8_t day The current day.                                               */
/********************************************************************************/
void dst_calculate_dates(uint16_t year, uint8_t month, uint8_t day)
{
#if (CONFIG_DST == 1)
	// DST in US/Canada: 2nd Sun in Mar to 1st Sun in Nov.
	dst_dates[0].month = 3;
	dst_dates[0].day = N_SUN_OF_MON(2, 3, year);
	dst_dates[1].month = 11;
	dst_dates[1].day = N_SUN_OF_MON(1, 11, year);
#endif
#if (CONFIG_DST == 2)
	// DST in Mexico: first Sun in Apr to last Sun in Oct.
	dst_dates[0].month = 4;
	dst_dates[0].day = N_SUN_OF_MON(1, 4, year);
	dst_dates[1].month = 10;
	dst_dates[1].day = LAST_SUN_OF_MON(10, 31, year);
#endif
#if (CONFIG_DST == 3)
	// DST in Brazil: third Sun in Oct to third Sun in Feb.
	dst_dates[0].month = 10;
	dst_dates[0].day = N_SUN_OF_MON(3, 10, year);
	dst_dates[1].month = 2;
	dst_dates[1].day = N_SUN_OF_MON(3, 2, year);
#endif
#if (CONFIG_DST == 4)
	// DST in EU/UK: last Sun in Mar to last Sun in Oct.
	dst_dates[0].month = 3;
	dst_dates[0].day = LAST_SUN_OF_MON(3, 31, year);
	dst_dates[1].month = 10;
	dst_dates[1].day = LAST_SUN_OF_MON(10, 31, year);
#endif
#if (CONFIG_DST == 5)
	// DST in Australia: first Sun in Oct to first Sun in Apr.
	dst_dates[0].month = 10;
	dst_dates[0].day = N_SUN_OF_MON(1, 10, year);
	dst_dates[1].month = 4;
	dst_dates[1].day = N_SUN_OF_MON(1, 4, year);
#endif
#if (CONFIG_DST == 6)
	// DST in New Zealand: last Sun in Sep to first Sun in Apr.
	dst_dates[0].month = 9;
	dst_dates[0].day = LAST_SUN_OF_MON(9, 30, year);
	dst_dates[1].month = 4;
	dst_dates[1].day = N_SUN_OF_MON(1, 4, year);
#endif

	// This test may be wrong if you set your watch
	// on the time-change day.
	dst_state = (dst_isDateInDST(month, day)) ? DST_STATE_DST : DST_STATE_ST;
}

/* I don't have a very good idea on what this does.. */
uint8_t dst_isDateInDST(uint8_t month, uint8_t day)
{
	if (dst_dates[0].month < dst_dates[1].month) {
		/* Northern hemisphere */
		return
			((DSTNUM(month, day) >= DSTNUM(dst_dates[0].month, dst_dates[0].day)) &&
			 (DSTNUM(month, day) < DSTNUM(dst_dates[1].month, dst_dates[1].day)));
	} else {
		/* Southern hemisphere */
		return (!(
				((DSTNUM(month, day) >= DSTNUM(dst_dates[1].month, dst_dates[1].day)) &&
				 (DSTNUM(month, day) < DSTNUM(dst_dates[0].month, dst_dates[0].day)))));
	}
}

uint8_t dst_day_of_week(uint16_t year, uint8_t month, uint8_t day)
{
	/* Calculate days since 2000-01-01 */
	uint32_t tmp = (year % 200) * 365;
	tmp += ((year % 200) / 4); // leap days

	switch (month) { // using lots of drop-through!
	case 12:
		tmp += 30; /* for nov */

	case 11:
		tmp += 31; /* for oct */

	case 10:
		tmp += 30; /* for sep */

	case 9:
		tmp += 31; /* for aug */

	case 8:
		tmp += 31; /* for jul */

	case 7:
		tmp += 30; /* for jun */

	case 6:
		tmp += 31; /* for may */

	case 5:
		tmp += 30; /* for apr */

	case 4:
		tmp += 31; /* for mar */

	case 3:
		tmp += 28; /* for feb */

		if ((year % 4) == 0) {
			tmp++;
		}

	case 2:
		tmp += 31; /* for jan */

	case 1:
	default:
		/* do nothing */
		break;
	}

	tmp += day;
	tmp--; /* because day-of-month is 1-based (2000-01-01 is the ZERO day). */

	/* day zero (2000-01-01) was a Saturday. */
	return (uint8_t)((tmp + 6) % 7);
}

#endif /* (CONFIG_DST > 0) */

