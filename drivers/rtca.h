/*
    rtca.h: TI CC430 Hardware Realtime Clock (RTC_A)

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

#ifndef __RTCA_H__
#define __RTCA_H__

#include <openchronos.h>

enum rtca_tevent {
	RTCA_EV_ALARM	= BIT0,
	RTCA_EV_SECOND = BIT1,
	RTCA_EV_MINUTE	= BIT2,
	RTCA_EV_HOUR	= BIT3,
	RTCA_EV_DAY		= BIT4,
	RTCA_EV_MONTH	= BIT5,
	RTCA_EV_YEAR	= BIT6
};

/* Day of week strings */
static char const * const rtca_dow_str[] = {
	"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

struct {
	uint32_t sys;   /* system time: number of seconds since power on */
	uint16_t year;  /* cache of RTC year register */
	uint8_t mon;    /* cache of RTC month register */
	uint8_t day;    /* cache of RTC day register */
	uint8_t dow;    /* cache of RTC day of week register */
	uint8_t hour;   /* cache of RTC hour register */
	uint8_t min;    /* cache of RTC minutes register */
	uint8_t sec;    /* cache of RTC seconds register */
} rtca_time;

#define rtca_stop()		(RTCCTL01 |=  RTCHOLD)
#define rtca_start()		(RTCCTL01 &= ~RTCHOLD)

/* the ev variable holds the time event, see enum rtca_tevent for more info.
please add -fshort-enums to CFLAGS to store rtca_tevent as only a byte */
void rtca_init(void);

uint8_t rtca_get_max_days(uint8_t month, uint16_t year);

void rtca_update_dow();
void rtca_set_time();
void rtca_set_date();

void rtca_get_alarm(uint8_t *hour, uint8_t *min);
void rtca_set_alarm(uint8_t hour, uint8_t min);

void rtca_enable_alarm();
void rtca_disable_alarm();

/* exclusive use by openchronos system */
volatile enum rtca_tevent rtca_last_event;

#endif /* __RTCA_H__ */
