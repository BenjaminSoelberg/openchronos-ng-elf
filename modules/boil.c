/**
   modules/boil.c: boiling point calculator module for openchronos-ng

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

/* This module uses the BMP085 sensor (white PCB) to calculate the boiling point of eight substances in the current environment, by using the Antoine equation [ https://en.wikipedia.org/wiki/Antoine_equation ].

   The up and down keys change the substance (water is default).
   The # key changes the displayed unit between celsius (default), farenheit, kelvin and rankine.
 */

#include "messagebus.h"
#include "menu.h"
#include "config.h"

#include "drivers/display.h"
#include "drivers/bmp_ps.h"
#include "drivers/ps.h"

#include <math.h>

#define C_TO_K(x) (x + 273.15000)
#define C_TO_F(x) (x * 1.8 + 32000)
#define C_TO_R(x) (x * 1.8 + 491.67000)

#define PA_TO_MMHG(x) (x / 133.32237)

// Data taken from table B.4: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781118477304.app2
#define SUB_NUM 8
static float a[SUB_NUM] =
{ 8.01767, 8.07240, 8.11220, 6.90565, 6.99052, 6.95464, 6.93710,
  7.23160 };
static float b[SUB_NUM] =
{ 1715.700000, 1574.99000, 1592.86400, 1211.03300, 1452.43000, 1344.80000, 1171.20000,
  1277.030 };
static float c[SUB_NUM] =
{ 234.26800, 238.87000, 226.18400, 220.79000, 215.30700, 219.48200, 227.00000,
  237.23000 };
static char substances[SUB_NUM][6] =
{ "WATER", "METHA", "ETHAN", "BENZE", "XYLEN", "TOLUE", "CLFOR",
  "ACETO" };

static uint8_t i;
static uint8_t unit;
static uint8_t boil_counter;

static void print_boil(void)
{
     float t = b[i] / (a[i] - log10f(PA_TO_MMHG(bmp_ps_get_pa()))) - c[i];

     display_chars(0, LCD_SEG_L2_4_0, substances[i], SEG_SET);
     display_symbol(0, LCD_UNIT_L1_DEGREE, SEG_SET);
     switch (unit) {
     case 0:			// Celsius
	  display_char(0, LCD_SEG_L1_0, 'C', SEG_SET);
	  break;
     case 1:			// Farenheit
	  display_char(0, LCD_SEG_L1_0, 'F', SEG_SET);
	  t = C_TO_F(t);
	  break;
     case 2:			// Kelvin
	  display_char(0, LCD_SEG_L1_0, 'K', SEG_SET);
	  t = C_TO_K(t);
	  break;
     case 3:			// Rankine
	  display_char(0, LCD_SEG_L1_0, 'R', SEG_SET);
	  t = C_TO_R(t);
	  break;
     }

     if ((int) t < 0) {
	  display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_SET);
	  t = -t;
     } else
	  display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);

     
     if ((uint16_t) t == 0)
	  display_chars(0, LCD_SEG_L1_3_1, "  0", SEG_SET);
     else
	  _printf(0, LCD_SEG_L1_3_1, "%3u", (uint16_t) t);
}

static void boil_interrupt(enum sys_message msg)
{
     if (boil_counter == 0)
	  print_boil();

     boil_counter = (boil_counter + 1) % CONFIG_MOD_BOIL_REFRESH;
}

static void up_btn(void)
{
     i = (i + 1) % SUB_NUM;
     print_boil();
}

static void down_btn(void)
{
     i = (i + SUB_NUM - 1) % SUB_NUM;
     print_boil();
}

static void num_pressed(void)
{
     unit = (unit + 1) % 4;
     print_boil();
}

static void boil_activate(void)
{
     i = unit = boil_counter = 0;
     bmp_ps_init();
     init_pressure_table();
     bmp_ps_start();

     sys_messagebus_register(&boil_interrupt, SYS_MSG_RTC_SECOND);
}

static void boil_deactivate(void)
{
     bmp_ps_stop();
     sys_messagebus_unregister_all(&boil_interrupt);

     display_clear(0, 0);
}

void mod_boil_init(void)
{
     menu_add_entry("BOIL", &up_btn, &down_btn, &num_pressed, NULL, NULL,
		    NULL, &boil_activate, &boil_deactivate);
}
