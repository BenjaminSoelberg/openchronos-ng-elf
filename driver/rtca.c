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

void rtca_init()
{
	// TODO: for safety, check if ACLK is 12k

	// Enable calendar mode, disable BCD format
	RTCCTL1 |= RTCMODE;
	RTCCTL1 &= ~RTCBCD;

	// Write sane date to the RTC 11:11:11 Fri(5) 11/11/2011
	rtca_set_date(11, 11, 11, 5, 11, 11, 2011);

	// Enable RTC
	RTCCTL1 |= ~RTCHOLD;
}

void rtca_set_date(u8 sec, u8 min, u8 hour, u8 dow, u8 day, u8 mon, u16 year)
{
	RTCSEC = sec;
	RTCMIN = min;
	RTCHOUR = hour;
	RTCDOW = dow;
	RTCDAY = day;
	RTCMON = mon;
	RTCYEARL = year & 0xff;
	RTCYEARH = year >> 8;
}

