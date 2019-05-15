/**
   modules/soundspeed.c: speed of sound calculator module for openchronos-ng

   Copyright (C) 2019 Luca Lorello <strontiumaluminate@gmail.com>

   http://github.com/HashakGik/openchronos-ng-elf

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

/* This module uses the BMP085 sensor (white PCB) to calculate the speed of sound in the current environment.
   Given the current pressure (pa) and temperature (temp), air density is given by:
   rho = pa / (temp * R) [ https://en.wikipedia.org/wiki/Density_of_air ]
   and sound speed is given by:
   speed = sqrt(gamma * pa / rho) [ https://en.wikipedia.org/wiki/Speed_of_sound#Speed_of_sound_in_ideal_gases_and_air ]

   The # key changes the displayed unit between m/s (default), km/h, mph and ft/s.
 */

#include "messagebus.h"
#include "menu.h"
#include "config.h"

#include "drivers/display.h"
#include "drivers/bmp_ps.h"
#include "drivers/ps.h"

#include <math.h>

#define MS_TO_KMH(x) (x * 3.6)
#define MS_TO_MPH(x) (x * 2.237)
#define MS_TO_FTS(x) (x * 3.281)

static uint8_t i;
static uint8_t unit;

static void print_sound(void)
{
    float rho;
    float s;
    uint32_t pa = bmp_ps_get_pa();

    rho = pa / (bmp_ps_get_temp() / 10.0 * 287.058);
    s = sqrtf(1.4 * pa / rho);

    switch (unit) {
    case 0:			// m/s
	display_symbol(0, LCD_UNIT_L1_M, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_PER_S, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_K, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_PER_H, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_I, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_FT, SEG_OFF);
	break;
    case 1:			// km/h
	display_symbol(0, LCD_UNIT_L1_M, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_PER_S, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_K, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_PER_H, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_I, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_FT, SEG_OFF);
	s = MS_TO_KMH(s);
	break;
    case 2:			// mph
	display_symbol(0, LCD_UNIT_L1_M, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_PER_S, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_K, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_PER_H, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_I, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_FT, SEG_OFF);
	s = MS_TO_MPH(s);
	break;
    case 3:			// ft/s
	display_symbol(0, LCD_UNIT_L1_M, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_PER_S, SEG_SET);
	display_symbol(0, LCD_UNIT_L1_K, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_PER_H, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_I, SEG_OFF);
	display_symbol(0, LCD_UNIT_L1_FT, SEG_SET);
	s = MS_TO_FTS(s);
	break;
    }

    _printf(0, LCD_SEG_L1_3_0, "%4u", s);
}


static void sound_interrupt(enum sys_message msg)
{
    if (i == 0)
	print_sound();

    i = (i + 1) % CONFIG_MOD_SOUNDSPEED_REFRESH;
}

static void num_pressed(void)
{
    unit = (unit + 1) % 4;
    print_sound();
}

static void sound_activate(void)
{
    i = unit = 0;
    bmp_ps_init();
    init_pressure_table();
    bmp_ps_start();

    sys_messagebus_register(&sound_interrupt, SYS_MSG_RTC_SECOND);

}

static void sound_deactivate(void)
{
    bmp_ps_stop();
    sys_messagebus_unregister_all(&sound_interrupt);

    display_clear(0, 0);
}


void mod_soundspeed_init(void)
{
    menu_add_entry("SSOUN", NULL, NULL, &num_pressed, NULL, NULL, NULL,
		   &sound_activate, &sound_deactivate);
}
