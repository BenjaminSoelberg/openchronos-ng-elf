/**
   modules/music.c: Buzzer music module for openchronos-ng

   Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

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

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/display.h"
#include "drivers/buzzer.h"

/* tunes generated using contrib/rtttl2bin.py */
/* nokia tune */
static note nokia[] =  {0x25a8, 0x25a6, 0x4b1a, 0x4b1c, 0x25a5, 0x2593, 0x4b16, 0x4b18, 0x2593, 0x2591, 0x4b15, 0x4b18, 0x6411, 0x000F};

static void num_press()
{
     buzzer_play(nokia);
}


static void music_activate()
{
     display_chars(0, LCD_SEG_L2_4_0, "MUSIC", SEG_ON);
}

static void music_deactivate()
{
     display_clear(0, 2);
}

void mod_music_init(void)
{
     menu_add_entry("MUSIC",
		    NULL,
		    NULL,
		    &num_press,
		    NULL, NULL, NULL, &music_activate, &music_deactivate);
}
