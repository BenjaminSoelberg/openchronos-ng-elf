/**
   modules/steps.c: pedometer module for openchronos-ng

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

#include "messagebus.h"
#include "menu.h"

#include "drivers/display.h"
#include "drivers/as.h"
#include "drivers/bmp_as.h"
#include "drivers/timer.h"

/* This module uses the accelerator's internal slope interrupt. This is less precise than software processing, but allows for minimal CPU and memory usage.
   A slope of at least 250mg (default setting for the 8G scale) between two consecutive samples will trigger the step count. 
   A long # press resets the counter.
*/

static uint32_t steps;

static void update_steps(enum sys_message msg)
{
  bmp_as_process_interrupt();
  steps = (steps + 1) % 200000; // Limit the step count to the maximum displayable on the display.
  _printf(0, LCD_SEG_L2_5_0, "%6u", steps);
}

static void steps_activate(void)
{
  steps = 0;
  as_init();
  bmp_as_start(BMP_GRANGE_8G, BMP_BWD_31HZ, BMP_SLEEP_10MS, 1); // Filter data at 31.25Hz and set the range to 8g.
  timer0_delay(1000, LPM3_bits);

  bmp_as_interrupts_t ints = bmp_as_init_interrupts();
  ints.slope_interrupt.x = 1;
  ints.slope_interrupt.y = 1;
  ints.slope_interrupt.z = 1;
  bmp_as_enable_interrupts(ints); // Enable slope interrupt on all three axes.

  sys_messagebus_register(&update_steps, SYS_MSG_AS_INT);
  display_chars(0, LCD_SEG_L2_5_0, "     0", SEG_SET);
}

static void steps_deactivate(void)
{
  display_clear(0, 0);
  sys_messagebus_unregister_all(&update_steps);

  bmp_as_disable_interrupts();
  bmp_as_stop();
}

static void long_num_pressed(void)
{
  steps = 0;
  display_chars(0, LCD_SEG_L2_5_0, "     0", SEG_SET);
}

void mod_steps_init(void)
{
  menu_add_entry("STEPS", NULL, NULL, NULL, NULL, &long_num_pressed,
		 NULL, &steps_activate, &steps_deactivate);
}
