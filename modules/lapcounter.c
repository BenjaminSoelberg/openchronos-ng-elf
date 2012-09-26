/*
    modules/laptimer.c:

    Copyright (C) 2012   Robert Hoeller <rhh.privat@googlemail.com>

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
/*
 * laptimer.c
 *
 *  Created on: Sep 16, 2012
 *      Author: rhh-privat
 *
 *      This module is intended as a "hello world" application for me
 *      to get used to the openchronos-ng framework..
 *
 *      Use:
 *      You can select the LAP-mode from the function-menu
 *
 *      First line shows "LAP" (in this version) and the second line
 *      shows the lapcounter value.
 *      ^ (up-arrow) increments the lapcounter
 *      v (dn-arrow) decrements the lapcounter
 *      # (num-longpressed) resets the lapcounter
 *      # (num-shortpressed) --- noop
 *      * (star-Longpressed) --- noop
 *		- (both-arrows) --- noop
 */


#include <openchronos.h>
/* driver */
#include <drivers/display.h>


static uint laps;

void drawLapcounterScreen(void)
{
	display_chars(0, LCD_SEG_L1_3_1, "LAP", SEG_SET);
	_printf(0, LCD_SEG_L2_3_0, "%4u", laps);
	display_symbol(0, LCD_SYMB_TOTAL, SEG_ON);

}

static void lapcounter_activated()
{
	drawLapcounterScreen();
}


static void lapcounter_deactivated()
{
	/* clean up screen */
	display_clear(0, 1);
	display_clear(0, 2);
	display_symbol(0, LCD_SYMB_TOTAL, SEG_OFF);
}

static void num_long_pressed()
{
	laps = 0;
	drawLapcounterScreen();
}

static void down_press()
{
	if (laps > 0)
		laps--;
	drawLapcounterScreen();
}
static void up_press()
{
	laps++;
	drawLapcounterScreen();
}


void mod_lapcounter_init(void)
{
	laps = 0;

	menu_add_entry(" LAP ",
			&up_press,
			&down_press, NULL, NULL,
			&num_long_pressed, NULL,
			&lapcounter_activated,
			&lapcounter_deactivated);
}
