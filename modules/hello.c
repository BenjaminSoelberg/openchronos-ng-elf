/**
   modules/hello.c: hello world module for openchronos-ng

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

/* This module shows a scrolling hello world.
   The display_scroll function will probably be included in the display driver.
 */

#include "messagebus.h"
#include "menu.h"
#include "drivers/display.h"

uint8_t str_counter;
char *hello = "HELLO WORLD ITS GOOD TO SEE YOU ALL";

void display_scroll(uint8_t scr_nr, enum display_segment_array segments, char const *str, uint8_t len, enum display_segstate state, uint8_t *i)
{
     uint8_t segnum = segments & 0x0f;
     uint8_t lastchar = (segnum < len)? (len - segnum + 1): segnum;

     display_chars(scr_nr, segments, str + *i, state);
     *i = (*i + 1) % lastchar;
}



static void hello_interrupt(enum sys_message msg)
{
     display_scroll(0, LCD_SEG_L2_4_0, hello, 35, SEG_SET, &str_counter);
}

static void hello_activate(void)
{
     str_counter = 0;
     sys_messagebus_register(&hello_interrupt, SYS_MSG_RTC_SECOND);
}

static void hello_deactivate(void)
{
     sys_messagebus_unregister_all(&hello_interrupt);
}

void mod_hello_init(void)
{
     menu_add_entry("HELLO", NULL, NULL, NULL, NULL, NULL, NULL,
		    &hello_activate, &hello_deactivate);
}
