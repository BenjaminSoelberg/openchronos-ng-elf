/**
    rtca.c: TI CC430 Hardware Realtime Clock (RTC_A)

    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

#include "rtca.h"
#include "rtca_now.h"

#ifdef CONFIG_RTC_DST
#include "rtc_dst.h"
#endif

/* 1. A year that is divisible by 4 is a leap year.
    Exception 1: a year that is divisible by 100 is not a leap year.
    Exception 2: a year that is divisible by 400 is a leap year. */
#define IS_LEAP_YEAR(Y) (((Y)%4 == 0) && (((Y)%100 != 0) || ((Y)%400 == 0)))

/* compute number of leap years since BASE_YEAR */
#define BASE_YEAR 1984 /* not a leap year, so no need to add 1 */
#define LEAPS_SINCE_YEAR(Y) (((Y) - BASE_YEAR) + ((Y) - BASE_YEAR) / 4);

#ifdef CONFIG_MOD_CLOCK_AMPM
uint8_t display_am_pm = 1;
#else
uint8_t display_am_pm = 0;
#endif

void rtca_init(void)
{
    rtca_time.year = COMPILE_YEAR;
    rtca_time.mon = COMPILE_MON;
    rtca_time.day = COMPILE_DAY;
    rtca_time.dow = COMPILE_DOW;
    rtca_time.hour = COMPILE_HOUR;
    rtca_time.min = COMPILE_MIN;
    rtca_time.sec = 59; // So we can see the watch is working after reset

#ifdef CONFIG_RTC_IRQ
    /* Enable calendar mode (date/time registers are automatically reset)
    and enable read ready interrupts
    and set time event interrupts at each minute
    also enable alarm interrupts */
    RTCCTL01 |= RTCMODE | RTCRDYIE | RTCAIE;

    RTCSEC = rtca_time.sec;
    RTCMIN = rtca_time.min;
    RTCHOUR = rtca_time.hour;
    RTCDAY = rtca_time.day;
    RTCDOW = rtca_time.dow;
    RTCMON = rtca_time.mon;
    RTCYEARL = rtca_time.year & 0xff;
    RTCYEARH = rtca_time.year >> 8;

    /* Enable the RTC */
    rtca_start();

    /* Enable minutes interrupts */
    RTCCTL01 |= RTCTEVIE;
#endif

#ifdef CONFIG_RTC_DST
    /* initialize DST module */
    rtc_dst_init();
#endif
}

/* returns number of days for a given month */
uint8_t rtca_get_max_days(uint8_t month, uint16_t year)
{
    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        case 2:
            if (IS_LEAP_YEAR(year))
                return 29;
            else
                return 28;

        default:
            return 0;
    }
}

void rtca_set_time()
{
    /* Stop RTC timekeeping for a while */
    rtca_stop();

    /* update RTC registers */
    RTCSEC = rtca_time.sec;
    RTCMIN = rtca_time.min;
    RTCHOUR = rtca_time.hour;

    /* Resume RTC time keeping */
    rtca_start();
}

void rtca_get_alarm(uint8_t *hour, uint8_t *min)
{
    *hour = RTCAHOUR & 0x1F;
    *min  = RTCAMIN  & 0x3F;
}

void rtca_set_alarm(uint8_t hour, uint8_t min)
{
    /* Disable alarm interrupt while setting alarm */
    uint16_t original_state = RTCCTL01;
    RTCCTL01 &= ~RTCAIE;
    /* Set hour and min while keeping current Alarm Enable state */
    RTCAHOUR = (RTCAHOUR & RTCAE) | hour;
    RTCAMIN  = (RTCAMIN  & RTCAE) | min;
    /* Restore alarm interrupt state*/
    RTCCTL01 = original_state;
}

void rtca_enable_alarm()
{
    /* Disable alarm interrupt while setting alarm */
    RTCCTL01 &= ~RTCAIE;
    /* Set Alarm Enable for both hour and min */
    RTCAHOUR |= RTCAE;
    RTCAMIN  |= RTCAE;
    /* Enable alarm interrupt */
    RTCCTL01 |= RTCAIE;
}

void rtca_disable_alarm()
{
    /* Disable alarm interrupt */
    RTCCTL01 &= ~RTCAIE;
    /* Clear Alarm Enable for both hour and min */
    RTCAHOUR &= ~RTCAE;
    RTCAMIN  &= ~RTCAE;
}

void rtca_update_dow(struct DATETIME *datetime)
{
    uint8_t dow;

    dow = LEAPS_SINCE_YEAR(datetime->year);

    if ((29 == rtca_get_max_days(2, datetime->year)) && (datetime->mon < 3))
        dow--; /* if this is a leap year but before February 29 */

    /* add day of current month */
    dow += datetime->day;

    /* add this month's dow value */
    switch (datetime->mon) {
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
    }

    dow = dow % 7;
    datetime->dow = dow;
}

void rtca_set_date()
{
    /* Stop RTC timekeeping for a while */
    rtca_stop();

    rtca_update_dow(&rtca_time);

    /* update RTC registers and local cache */
    RTCDAY = rtca_time.day;
    RTCDOW = rtca_time.dow;
    RTCMON = rtca_time.mon;
    RTCYEARL = rtca_time.year & 0xff;
    RTCYEARH = rtca_time.year >> 8;

    /* Resume RTC time keeping */
    rtca_start();

#ifdef CONFIG_RTC_DST
    /* calculate new DST switch dates */
    rtc_dst_calculate_dates(rtca_time.year, rtca_time.mon, rtca_time.day, rtca_time.hour);
#endif
}
__attribute__((interrupt(RTC_A_VECTOR)))
void RTC_A_ISR(void)
{
    /* the IV is cleared after a read, so we store it */
    uint16_t iv = RTCIV;

    /* copy register values */
    rtca_time.sec = RTCSEC;

    /* count system time */
    rtca_time.sys++;

    enum rtca_tevent ev = 0;

    /* second event (from the read ready interrupt flag) */
    if (iv == RTCIV_RTCRDYIFG) {    /* Did second changed */
        ev = RTCA_EV_SECOND;
        goto finish;
    }

    if (iv == RTCIV_RTCAIFG) {  /* Did alarm event occurred */
        ev = RTCA_EV_ALARM;
        goto finish;
    }

    if (iv == RTCIV_RTCTEVIFG)    /* Did minute changed */
    {
        ev = RTCA_EV_MINUTE;
        rtca_time.min = RTCMIN;

        if (rtca_time.min != 0)     /* Hour changed */
            goto finish;

        ev |= RTCA_EV_HOUR;
        rtca_time.hour = RTCHOUR;

#ifdef CONFIG_RTC_DST
        rtc_dst_hourly_update();
#endif

        if (rtca_time.hour != 0)    /* Day changed */
            goto finish;

        ev |= RTCA_EV_DAY;
        rtca_time.day = RTCDAY;
        rtca_time.dow = RTCDOW;

        if (rtca_time.day != 1)     /* Month changed */
            goto finish;

        ev |= RTCA_EV_MONTH;
        rtca_time.mon = RTCMON;

        if (rtca_time.mon != 1)     /* Year changed */
            goto finish;

        ev |= RTCA_EV_YEAR;
        rtca_time.year = RTCYEARL | (RTCYEARH << 8);
#ifdef CONFIG_RTC_DST
        /* calculate new DST switch dates */
        rtc_dst_calculate_dates(rtca_time.year, rtca_time.mon, rtca_time.day, rtca_time.hour);
#endif
    }

finish:
    /* append events, since ISR could be triggered
     multipe times until rtca_last_event gets parsed */
    rtca_last_event |= ev;

    /* exit from LPM3, give execution back to mainloop */
    _BIC_SR_IRQ(LPM3_bits);
}

