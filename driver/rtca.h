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

#include "project.h"

// the ev variable will hold the time event, that is:
// 0x00 - Minute changed
// 0x01 - Hour changed
// 0x02 - Every day at midnight (00:00)
// 0x03 - Every day at noon (12:00)
typedef void (*rtca_tevent_fn_t)(u8 ev);

// list of time event callback functions
typedef struct rtca_cblist {
	rtca_tevent_fn_t fn;
	struct rtca_cblist *next;
} rtca_cblist_t;

void rtca_init();
void rtca_tevent_fn_register(rtca_tevent_fn_t fn);
void rtca_tevent_fn_unregister(rtca_tevent_fn_t fn);

void rtca_get_time(u8 *hour, u8 *min, u8 *sec);
void rtca_set_time(u8 hour, u8 min, u8 sec);

void rtca_get_date(u16 *year, u8 *mon, u8 *day, u8 *dow);
void rtca_set_date(u16 year, u8 mon, u8 day, u8 dow);
