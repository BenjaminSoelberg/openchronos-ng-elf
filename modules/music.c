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
/* super mario bros tune */
static note smb[] = {0x25a8, 0x25a8, 0x12d0, 0x4b28, 0x25a4, 0x4b28, 0x4b2b, 0x4b10, 0x4b1b, 0x4b10, 0x4b24, 0x2590, 0x4b1b, 0x2590, 0x4b18, 0x2590, 0x4b11, 0x4b13, 0x2592, 0x4b11, 0x191b, 0x25a8, 0x25ab, 0x4b21, 0x25a9, 0x4b2b, 0x4b28, 0x25a4, 0x25a6, 0x4b13, 0x2590, 0x4b24, 0x2590, 0x4b1b, 0x2590, 0x4b18, 0x2590, 0x4b11, 0x4b13, 0x2592, 0x4b11, 0x191b, 0x25a8, 0x25ab, 0x4b21, 0x25a9, 0x4b2b, 0x4b28, 0x25a4, 0x25a6, 0x4b13, 0x4b10, 0x25ab, 0x25aa, 0x25a9, 0x25a7, 0x2590, 0x25a8, 0x2590, 0x259c, 0x2591, 0x25a4, 0x2590, 0x2591, 0x25a4, 0x25a6, 0x4b10, 0x25ab, 0x25aa, 0x25a9, 0x25a7, 0x2590, 0x25a8, 0x2590, 0x25b4, 0x2590, 0x25b4, 0x25b4, 0x9610, 0x25ab, 0x25aa, 0x25a9, 0x25a7, 0x2590, 0x25a8, 0x2590, 0x259c, 0x2591, 0x25a4, 0x2590, 0x2591, 0x25a4, 0x25a6, 0x4b10, 0x25a7, 0x4b10, 0x25a6, 0x4b10, 0x25a4, 0x000F};

static void num_press()
{
    buzzer_play(smb);
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
                   NULL,
                   NULL,
                   NULL,
                   &music_activate,
                   &music_deactivate);
}
