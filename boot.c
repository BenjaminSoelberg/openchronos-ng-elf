/*
    boot.c: Openchronos-ng on boot wireless updater

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

#include "openchronos.h"

// Entry point of of the Flash Updater in BSL memory
#define CALL_RFSBL()   ((void (*)())0x1000)()

#define BTN_DOWN_PIN		(BIT0)
#define BTN_UP_PIN		(BIT4)

#define BTNS_MASK (BTN_UP_PIN | BTN_DOWN_PIN)

void rfbsl_updater(void)
{
	/* Set button ports to input */
	P2DIR &= ~BTNS_MASK;

	/* Enable internal pull-downs */
	P2OUT &= ~BTNS_MASK;
	P2REN |= BTNS_MASK;

	/* check if up & down buttons are being pressed simultaneously */
	if ((P2IN & BTNS_MASK) == BTNS_MASK)
		CALL_RFSBL();

};

/* put rfbsl_updater in the init8 section which is executed before main */
__attribute__ ((naked, section (".init8")))
void _init8(void)
{
	rfbsl_updater();
}

