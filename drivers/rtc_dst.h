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

#ifndef RTC_DST_H_
#define RTC_DST_H_

#define RTC_DST_STATE_ST 0
#define RTC_DST_STATE_DST 1

#define DST_US 1
#define DST_MEX 2
#define DST_BRZ 3
#define DST_EU 4
#define DST_AUS 5
#define DST_NZ 6

struct rtc_dst_date_struct {
    uint8_t month;
    uint8_t day;
};

extern struct rtc_dst_date_struct dst_dates[];
extern uint8_t rtc_dst_state;

void rtc_dst_init(void);
void rtc_dst_calculate_dates(uint16_t year, uint8_t month, uint8_t day, uint8_t hour);
void rtc_dst_hourly_update(void);

#endif
