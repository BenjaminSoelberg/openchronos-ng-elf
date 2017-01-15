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
static note smb[97] = {0x29a7, 0x29a6, 0x5364, 0x2993, 0x5352, 0x2991, 0x535c, 0x299b, 0x2999, 0x299b, 0x299c, 0x535b, 0x2999, 0xa692, 0x2990, 0x2992, 0x299b, 0x2990, 0x299b, 0x299b, 0x299a, 0x299b, 0x29a7, 0x2990, 0x2992, 0x2992, 0x2990, 0x299b, 0x299c, 0x2990, 0x299c, 0x299c, 0x2990, 0x2992, 0xa6a4, 0x2990, 0x299c, 0x2999, 0x2990, 0x2999, 0x2999, 0x2998, 0x2999, 0x29a6, 0x2990, 0x29a4, 0x29a4, 0x2990, 0x299c, 0x299b, 0x2990, 0x299b, 0x299b, 0x2990, 0x299c, 0xa692, 0x2990, 0x2992, 0x299b, 0x2990, 0x299b, 0x299b, 0x299a, 0x299b, 0x29ab, 0x2990, 0x29a7, 0x29a7, 0x2990, 0x2992, 0x2991, 0x2990, 0x29a9, 0x29a9, 0x2990, 0x29a9, 0xa6a9, 0x2990, 0x29a7, 0x5366, 0x29a9, 0x29a9, 0x29a8, 0x29a9, 0x5364, 0x29a9, 0x29a9, 0x29a8, 0x29a9, 0x2992, 0x2990, 0x2991, 0x2992, 0x2990, 0x2991, 0xa692, 0x000F};

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
	menu_add_entry("MUSIC", NULL, NULL, &num_press, NULL, NULL, NULL,
						&music_activate,
						&music_deactivate);
}
