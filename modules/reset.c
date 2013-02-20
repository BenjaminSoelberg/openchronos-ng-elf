/*
    modules/reset.c: Reset module for openchronos-ng

    Copyright (C) 2012-2013 Angelo Arrifano <miknix@gmail.com>

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

static void num_press()
{
	/* reset microcontroller */
	WDTCTL = 0;
}

static void reset_activate()
{
	/* update screen */
	display_chars(0, LCD_SEG_L2_5_0, " RESET", SEG_ON);
}

static void reset_deactivate()
{
	/* cleanup screen */
	display_clear(0, 2);
}

void mod_reset_init(void)
{
	menu_add_entry("RESET", NULL, NULL, &num_press, NULL, NULL, NULL,
						&reset_activate,
						&reset_deactivate);
}
