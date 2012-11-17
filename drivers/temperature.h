/*
    temperature.h: Temperature driver header

    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>

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

#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <openchronos.h>

void temperature_init(void);
void temperature_measurement(uint8_t filter);

#ifndef CONFIG_TEMPERATURE_METRIC_ONLY
int16_t convert_C_to_F(int16_t value);
int16_t convert_F_to_C(int16_t value);
#endif

/* TODO: pack this stuff!!!! */
struct temp {
	/* Temperature (oC) in 2.1 format */
	int16_t		degrees;
	/* User set calibration value (oC) in 2.1 format */
	int16_t		offset;
#ifndef CONFIG_TEMPERATURE_METRIC_ONLY
	/* C or F */
	uint8_t         is_c:1;
#endif

} sTemp;

#endif /* __TEMPERATURE_H__ */
