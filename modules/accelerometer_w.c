/**
   modules/accelerometer_w.c: white PCB accelerometer module for openchronos-ng

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

/* This module uses the BMA 250 sensor (white PCB) to calculate various acceleration parameters.

   The up and down keys switch between x axis (default), y axis, z axis, modulus, pitch angle and roll angle measurement.
   The # key changes the measurement scale between +-2g (default), +-4g, +-8g and +-16g. 
 */

#include "messagebus.h"
#include "menu.h"

#include "drivers/display.h"
#include "drivers/as.h"
#include "drivers/bmp_as.h"
#include "drivers/timer.h"

#include <math.h>

static int i;
static int scale;
static int16_t axes[3];

static void print_acc(void)
{
     int32_t vals[7];
     uint8_t j;
     float scale_factor = ((1 << (scale + 1)) * 100) / 512.0;
     uint8_t dec;
     int8_t tmp;

     for (j = 0, vals[3] = 0; j < 3; j++) {
	  vals[j] = axes[j] * scale_factor;
	  vals[3] += vals[j] * vals[j];
     }
     vals[3] = sqrtf(vals[3]);	// Modulus of acceleration vector.
     vals[4] = atan2f(vals[0], vals[2]) * 5729; // Pitch angle. Convert from rad to deg and multiply by 100.
     vals[5] = atan2f(vals[1], vals[2]) * 5729; // Roll angle.


     display_symbol(0, LCD_SEG_L2_DP, SEG_SET);
     switch (i) {
     case 0:
	  display_chars(0, LCD_SEG_L1_3_0, "  X ", SEG_SET);
	  break;
     case 1:
	  display_chars(0, LCD_SEG_L1_3_0, "  Y ", SEG_SET);
	  break;
     case 2:
	  display_chars(0, LCD_SEG_L1_3_0, "  Z ", SEG_SET);
	  break;
     case 3:
	  display_chars(0, LCD_SEG_L1_3_0, "ABSO", SEG_SET);
	  break;
     case 4:
       	  display_chars(0, LCD_SEG_L1_3_0, "PITC", SEG_SET);
	  break;
     case 5:
       	  display_chars(0, LCD_SEG_L1_3_0, "ROLL", SEG_SET);
	  break;
     }

     tmp = vals[i] / 100;
     if (vals[i] < 0)
	  dec = (-vals[i]) % 100;
     else
	  dec = vals[i] % 100;

     if (tmp == 0)
	  display_char(0, LCD_SEG_L2_2, '0', SEG_SET);
     else
	  _printf(0, LCD_SEG_L2_4_2, "%2s", tmp);

     _printf(0, LCD_SEG_L2_1_0, "%02u", dec);

}

static void update_acc(enum sys_message msg)
{
     bmp_as_get_data(axes);

     print_acc();
}

static void acc_activate(void)
{
     axes[0] = axes[1] = axes[2] = 0;
     scale = 0;
     i = 0;
     as_init();
     bmp_as_start(BMP_GRANGE_2G, BMP_BWD_62HZ, BMP_SLEEP_1000MS, 0);
     timer0_delay(1000, LPM3_bits);

     bmp_as_interrupts_t ints = bmp_as_init_interrupts();

     //ints.slope_interrupt.x = 1;
     //ints.slope_interrupt.y = 1;
     //ints.slope_interrupt.z = 1;
     //  ints.tap_interrupt = 2;
     ints.new_interrupt = 1;
     bmp_as_enable_interrupts(ints);

     sys_messagebus_register(&update_acc, SYS_MSG_AS_INT);
}

static void acc_deactivate(void)
{
     display_clear(0, 0);
     sys_messagebus_unregister_all(&update_acc);

     bmp_as_disable_interrupts();
     bmp_as_stop();
}

static void up_btn(void)
{
     i = (i + 1) % 6;
     print_acc();
}

static void down_btn(void)
{
     i = (i + 5) % 6;
     print_acc();
}

static void num_pressed(void)
{
     scale = (scale + 1) % 4;
     display_clear(0, 0);

     bmp_as_stop();
     switch (scale) {
     case 0:
	  bmp_as_start(BMP_GRANGE_2G, BMP_BWD_62HZ, BMP_SLEEP_1000MS, 1);
	  display_chars(0, LCD_SEG_L1_3_0, " 2 G", SEG_SET | BLINK_SET);
	  break;
     case 1:
	  bmp_as_start(BMP_GRANGE_4G, BMP_BWD_62HZ, BMP_SLEEP_1000MS, 1);
	  display_chars(0, LCD_SEG_L1_3_0, " 4 G", SEG_SET | BLINK_SET);
	  break;
     case 2:
	  bmp_as_start(BMP_GRANGE_8G, BMP_BWD_62HZ, BMP_SLEEP_1000MS, 1);
	  display_chars(0, LCD_SEG_L1_3_0, " 8 G", SEG_SET | BLINK_SET);
	  break;
     case 3:
	  bmp_as_start(BMP_GRANGE_16G, BMP_BWD_62HZ, BMP_SLEEP_1000MS, 1);
	  display_chars(0, LCD_SEG_L1_3_0, "16 G", SEG_SET | BLINK_SET);
	  break;
     }

     timer0_delay(1000, LPM3_bits);
     display_chars(0, LCD_SEG_L1_3_0, "8888", SEG_OFF | BLINK_OFF);
     axes[0] = axes[1] = axes[2] = 0;
     print_acc();
}

void mod_accelerometer_w_init(void)
{
     menu_add_entry("ACCEL", &up_btn, &down_btn, &num_pressed, NULL, NULL,
		    NULL, &acc_activate, &acc_deactivate);
}
