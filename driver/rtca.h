/*
    rtca.h: TI CC430 Hardware Realtime Clock (RTC_A)

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

#ifndef __RTCA_H__
#define __RTCA_H__

#include "project.h"

typedef enum {
	RTCA_EV_ALARM,
	RTCA_EV_MINUTE,
	RTCA_EV_HOUR,
	RTCA_EV_DAY,
	RTCA_EV_MONTH,
	RTCA_EV_YEAR
} rtca_tevent_ev_t;

// the ev variable holds the time event, see rtca_tevent_ev_t for more info.
// please add -fshort-enums to CFLAGS to store rtca_tevent_ev_t as only a byte
typedef void (*rtca_tevent_fn_t)(rtca_tevent_ev_t ev);

// list of time event callback functions
typedef struct rtca_cblist {
	rtca_tevent_fn_t fn;
	struct rtca_cblist *next;
} rtca_cblist_t;

void rtca_init(void);
void rtca_tevent_fn_register(rtca_tevent_fn_t fn);
void rtca_tevent_fn_unregister(rtca_tevent_fn_t fn);

u8 rtca_get_max_days(u8 month, u16 year);
u32 rtca_get_systime(void);

void rtca_get_time(u8 *hour, u8 *min, u8 *sec);
void rtca_set_time(u8 hour, u8 min, u8 sec);

void rtca_get_date(u16 *year, u8 *mon, u8 *day, u8 *dow);
void rtca_set_date(u16 year, u8 mon, u8 day);

void rtca_get_alarm(u8 *hour, u8 *min);
void rtca_set_alarm(u8 hour, u8 min);

#endif // __RTCA_H__
