/**
    temperature.h: Temperature driver

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>

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

#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include "openchronos.h"

void temperature_init(void);
void temperature_measurement(void);
void temperature_get_C(int16_t *temp);
void temperature_get_F(int16_t *temp);

struct {
    uint16_t value;
    int16_t offset;
} temperature;

#endif /* __TEMPERATURE_H__ */
