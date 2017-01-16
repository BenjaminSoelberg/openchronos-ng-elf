/**
    Daylight Saving Time for OpenChronos on the TI ez430 chronos watch.
  
    Copyright 2011 Rick Miller <rdmiller3@gmail.com>
    Copyright 2012 Dan Ellis <dpwe@ee.columbia.edu>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

    This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "openchronos.h"

#ifdef CONFIG_RTC_DST

/* "private" defines */
#define N_SUN_OF_MON(n,mon,year) (((n)*7)-rtc_dst_day_of_week((year),(mon),((n)*7)))
#define LAST_SUN_OF_MON(mon,days,year) ((days)-rtc_dst_day_of_week((year),(mon),(days)))
#define DSTNUM(x,y,z) (((uint16_t)(x)*1000)+(uint16_t)(y*10)+(uint16_t)(z))

#include "rtca.h"

#include "rtc_dst.h"

/* "private" functions */
uint8_t rtc_dst_isDateInDST(uint8_t month, uint8_t day, uint8_t hour);

struct rtc_dst_date_struct rtc_dst_dates[2];
uint8_t rtc_dst_state;

uint8_t rtc_dst_day_of_week(uint16_t year, uint8_t month, uint8_t day);


/****************************************************************************/
/* DST initialize function. This is called by the module config setup       */
/****************************************************************************/
void rtc_dst_init(void)
{
    /* Calculate when to switch dates */
    rtc_dst_calculate_dates(rtca_time.year, rtca_time.mon, rtca_time.day, rtca_time.hour);
}

/******************************************************************************/
/* This function is called on hour changes by the rtca interrupt handler.     */
/* It implements the time changes on the appropriate days.                    */
/******************************************************************************/
void rtc_dst_hourly_update(void)
{
    if (rtca_time.hour == 2) {
        /* time changes always occur at 2AM */
        if (rtc_dst_state == RTC_DST_STATE_ST) {
            if (rtca_time.mon == rtc_dst_dates[0].month && rtca_time.day == rtc_dst_dates[0].day) {
                /* spring forward */
                rtc_dst_state = RTC_DST_STATE_DST;
                rtca_time.hour++;
                rtca_set_time();
            }
        } else {
            /* rtc_dst_state == RTC_DST_STATE_DST */
            if (rtca_time.mon == rtc_dst_dates[1].month && rtca_time.day == rtc_dst_dates[1].day) {
                /* fall back */
                rtc_dst_state = RTC_DST_STATE_ST;
                rtca_time.hour--;
                rtca_set_time();
            }
        }
    }
}

/********************************************************************************/
/* This function calculates the new date for DST switching. It should be called */
/* once a year, or whenever the date/time is changed                            */
/* @param uint8_t month The current month.                                      */
/* @param uint8_t day   The current day.                                        */
/* @param uint8_t hour  The current hour (to get the DST right if time is changed on day of DST switch). */
/********************************************************************************/

void rtc_dst_calculate_dates(uint16_t year, uint8_t month, uint8_t day, uint8_t hour)
{

#if (CONFIG_RTC_DST_ZONE == DST_US)
    // DST in US/Canada: 2nd Sun in Mar to 1st Sun in Nov.
    rtc_dst_dates[0].month = 3;
    rtc_dst_dates[0].day = N_SUN_OF_MON(2, 3, year);
    rtc_dst_dates[1].month = 11;
    rtc_dst_dates[1].day = N_SUN_OF_MON(1, 11, year);
#endif
#if (CONFIG_RTC_DST_ZONE == DST_MEX)
    // DST in Mexico: first Sun in Apr to last Sun in Oct.
    rtc_dst_dates[0].month = 4;
    rtc_dst_dates[0].day = N_SUN_OF_MON(1, 4, year);
    rtc_dst_dates[1].month = 10;
    rtc_dst_dates[1].day = LAST_SUN_OF_MON(10, 31, year);
#endif
#if (CONFIG_RTC_DST_ZONE == DST_BRZ)
    // DST in Brazil: third Sun in Oct to third Sun in Feb.
    rtc_dst_dates[0].month = 10;
    rtc_dst_dates[0].day = N_SUN_OF_MON(3, 10, year);
    rtc_dst_dates[1].month = 2;
    rtc_dst_dates[1].day = N_SUN_OF_MON(3, 2, year);
#endif
#if (CONFIG_RTC_DST_ZONE == DST_EU)
    // DST in EU/UK: last Sun in Mar to last Sun in Oct.
    rtc_dst_dates[0].month = 3;
    rtc_dst_dates[0].day = LAST_SUN_OF_MON(3, 31, year);
    rtc_dst_dates[1].month = 10;
    rtc_dst_dates[1].day = LAST_SUN_OF_MON(10, 31, year);
#endif
#if (CONFIG_RTC_DST_ZONE == DST_AUS)
    // DST in Australia: first Sun in Oct to first Sun in Apr.
    rtc_dst_dates[0].month = 10;
    rtc_dst_dates[0].day = N_SUN_OF_MON(1, 10, year);
    rtc_dst_dates[1].month = 4;
    rtc_dst_dates[1].day = N_SUN_OF_MON(1, 4, year);
#endif
#if (CONFIG_RTC_DST_ZONE == DST_NZ)
    // DST in New Zealand: last Sun in Sep to first Sun in Apr.
    rtc_dst_dates[0].month = 9;
    rtc_dst_dates[0].day = LAST_SUN_OF_MON(9, 30, year);
    rtc_dst_dates[1].month = 4;
    rtc_dst_dates[1].day = N_SUN_OF_MON(1, 4, year);
#endif

    // This test may be wrong if you set your watch
    // on the time-change day.
    rtc_dst_state = (rtc_dst_isDateInDST(month, day, hour)) ? RTC_DST_STATE_DST : RTC_DST_STATE_ST;
}

/* Figure out if the specified hour on the specified day is within the DST range */
uint8_t rtc_dst_isDateInDST(uint8_t month, uint8_t day, uint8_t hour)
{
    /* clip hour to a single digit since we only care which side of 2AM it is */
    /* and we're going to do comparisons via a MMDDH integer in 16 bits */
    if (hour > 9) hour = 9;

    if (rtc_dst_dates[0].month < rtc_dst_dates[1].month) {
        /* Northern hemisphere */
        return
            ((DSTNUM(month, day, hour) >= DSTNUM(rtc_dst_dates[0].month, rtc_dst_dates[0].day, 2)) &&
             (DSTNUM(month, day, hour) < DSTNUM(rtc_dst_dates[1].month, rtc_dst_dates[1].day, 2)));
    } else {
        /* Southern hemisphere */
        return (!(
                ((DSTNUM(month, day, hour) >= DSTNUM(rtc_dst_dates[1].month, rtc_dst_dates[1].day,2)) &&
                 (DSTNUM(month, day, hour) < DSTNUM(rtc_dst_dates[0].month, rtc_dst_dates[0].day,2)))));
    }
}

uint8_t rtc_dst_day_of_week(uint16_t year, uint8_t month, uint8_t day)
{
    /* Calculate days since 2000-01-01 */
    uint32_t tmp = (year % 200) * 365;
    tmp += (((year % 200) + 3) / 4); // leap days

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

#endif /* CONFIG_RTC_DST */

