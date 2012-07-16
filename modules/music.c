/*
    modules/music.c: Buzzer music module for openchronos-ng

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

	            http://www.openchronos-ng.sourceforge.net

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <openchronos.h>

#include <drivers/display.h>
#include <drivers/buzzer.h>

/* tunes generated using contrib/rtttl2bin.py */
/* super mario bros tune */
static note smb[] = {0x2588, 0x4b08, 0x4b08, 0x258c, 0x4b08, 0x4b0b, 0x4b04, 0x4b04, 0x4b04, 0x3204, 0x258f, 0x4b04, 0x4b0c, 0x2584, 0x4b05, 0x4b07, 0x2586, 0x2585, 0x2584, 0x258f, 0x4b08, 0x258b, 0x4b09, 0x2588, 0x4b0b, 0x4b08, 0x258c, 0x2588, 0x3207, 0x3204, 0x258f, 0x4b04, 0x4b0c, 0x2584, 0x4b05, 0x4b07, 0x2586, 0x2585, 0x2584, 0x258f, 0x4b08, 0x258b, 0x4b09, 0x2588, 0x4b0b, 0x4b08, 0x000F};

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
