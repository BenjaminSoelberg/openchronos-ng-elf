/**
   modules/crickets.c: cricket's chirp calculator module for openchronos-ng

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

/* This module calculates the number of cricket's chirp at the current temperature by using [ https://en.wikipedia.org/wiki/Dolbear's_law ]
   The original formula was meant to be used as a temperature estimate based on chirp count (of a very specific species of cricket), so the inverse formula might not be precise.

   The # key changes the displayed unit between chirps per minute (default), chirps per hour and chirps per second.
 */

#include "messagebus.h"
#include "menu.h"
#include "config.h"

#include "drivers/display.h"
#include "drivers/temperature.h"

static uint8_t i;
static uint8_t unit;

static void display_chirp(void)
{
     int16_t t;
     int16_t freq;

     temperature_get_C(&t);
     freq = 7 * (t / 10 - 10) + 40;	// t is in tenth of C.

     switch (unit) {
     case 0:			// per minute
	  display_symbol(0, LCD_UNIT_L1_M, SEG_SET);
	  display_symbol(0, LCD_UNIT_L1_I, SEG_SET);
	  display_symbol(0, LCD_UNIT_L1_PER_S, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_PER_H, SEG_OFF);
	  break;
     case 1:			// per hour
	  freq *= 60;
	  display_symbol(0, LCD_UNIT_L1_M, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_I, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_PER_S, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_PER_H, SEG_SET);
	  break;
     case 2:			// per second
	  freq /= 60;
	  display_symbol(0, LCD_UNIT_L1_M, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_I, SEG_OFF);
	  display_symbol(0, LCD_UNIT_L1_PER_S, SEG_SET);
	  display_symbol(0, LCD_UNIT_L1_PER_H, SEG_OFF);
	  break;
     }

     if (freq > 0)
	  _printf(0, LCD_SEG_L1_3_0, "%4u", freq);
     else
	  display_chars(0, LCD_SEG_L1_3_0, "   0", SEG_SET);
}

static void chirp_interrupt(enum sys_message msg)
{
     if (i == 0) {
	  display_chirp();
     }

     i = (i + 1) % CONFIG_MOD_CRICKETS_REFRESH;
}

static void change_unit(void)
{
     unit = (unit + 1) % 3;
     display_chirp();
}

static void crickets_activate(void)
{
     i = unit = 0;
     sys_messagebus_register(&chirp_interrupt, SYS_MSG_RTC_SECOND);
}

static void crickets_deactivate(void)
{
     sys_messagebus_unregister_all(&chirp_interrupt);
     display_clear(0, 0);
}


void mod_crickets_init(void)
{
     menu_add_entry("CRICK", NULL, NULL, &change_unit, NULL, NULL, NULL,
		    &crickets_activate, &crickets_deactivate);
}
