/**
   modules/altimeter.c: white PCB altimeter module for openchronos-ng

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

/* This module uses the BMP085 sensor (white PCB) and the original Texas Instrument functions to calculate current altitude (in m).
 */


#include "messagebus.h"
#include "menu.h"

#include "drivers/display.h"
#include "drivers/bmp_ps.h"
#include "drivers/ps.h"
#include "openchronos.h"
#include "config.h"

static uint8_t altitude_counter;

static void update_altitude(enum sys_message msg)
{
     if (altitude_counter == 0) {
	  int16_t alti =
	       conv_pa_to_meter(bmp_ps_get_pa(), bmp_ps_get_temp());
	  if (alti < 0) {
	       display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_SET);
	       alti = -alti;
	  } else
	       display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);

	  if (alti == 0)
	       display_chars(0, LCD_SEG_L1_3_0, "   0", SEG_SET);
	  else
	       _printf(0, LCD_SEG_L1_3_0, "%4u", alti);
     }
     altitude_counter = (altitude_counter + 1) % CONFIG_MOD_ALTIMETER_REFRESH;	// #include "config.h"
}

static void alti_init(void)
{
     bmp_ps_init();
     init_pressure_table();
     bmp_ps_start();

     altitude_counter = 0;
     sys_messagebus_register(&update_altitude, SYS_MSG_RTC_SECOND);	// The SYS_MSG_PS_INT is generated only after start_altitude_measurement() and the measurement is currently done synchronously via spinlocking, so it's not useful.
     display_symbol(0, LCD_UNIT_L1_M, SEG_SET);
}

static void alti_deac(void)
{
     bmp_ps_stop();
     sys_messagebus_unregister_all(&update_altitude);
     display_symbol(0, LCD_UNIT_L1_M, SEG_OFF);
}

void mod_altimeter_init(void)
{
     menu_add_entry("ALTIT", NULL, NULL, NULL, NULL, NULL, NULL, &alti_init,
		    &alti_deac);
}
