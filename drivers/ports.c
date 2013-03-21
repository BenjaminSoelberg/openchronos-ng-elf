/*
    drivers/ports.c: Openchronos ports driver

	 Copyright (C) 2012-2013 Angelo Arrifano <miknix@gmail.com>

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

    References:
    [1] http://en.wikipedia.org/wiki/Moving_average
 */


#include <openchronos.h>

/* drivers */
#include "ports.h"

#include "display.h"

#ifdef CONFIG_ACCELEROMETER
#include "vti_as.h"
#endif

#define ALL_BUTTONS 0x1F

/* averaging window resolution */
#define AVG_WIN_RES 8

/* computes smoothed moving average [1] */
#define SMOOTH_AVG(B, I)  (((B) * (CONFIG_BUTTONS_AVG_WIN-1) \
		+ ((I)<<AVG_WIN_RES) * CONFIG_BUTTONS_AVG_WIN) \
		/ CONFIG_BUTTONS_AVG_WIN)

/* gets binary state from averaged value */
#define GET_BINSTATE(B)  ((B) > (CONFIG_BUTTONS_AVG_WIN<<AVG_WIN_RES) \
		/ 2 ? 1 : 0)

static uint16_t btns_avg[5];

/* called by the 100Hz timer */
void ports_scan_btns(void)
{
	uint8_t btns = P2IN & ALL_BUTTONS;

	/* perform a smoothed moving average [1] for each input */
	btns_avg[0] = SMOOTH_AVG(btns_avg[0], (btns >> 0) & 0x01);
	btns_avg[1] = SMOOTH_AVG(btns_avg[1], (btns >> 1) & 0x01);
	btns_avg[2] = SMOOTH_AVG(btns_avg[2], (btns >> 2) & 0x01);
	btns_avg[3] = SMOOTH_AVG(btns_avg[3], (btns >> 3) & 0x01);
	btns_avg[4] = SMOOTH_AVG(btns_avg[4], (btns >> 4) & 0x01);

	uint8_t prev_state = ports_btns_state;
	ports_btns_state = GET_BINSTATE(btns_avg[0])
			| (GET_BINSTATE(btns_avg[1]) << 1)
			| (GET_BINSTATE(btns_avg[2]) << 2)
			| (GET_BINSTATE(btns_avg[3]) << 3)
			| (GET_BINSTATE(btns_avg[4]) << 4);

	ports_btns_flipd = ports_btns_state ^ prev_state;
}

/*
  Interrupt service routine for
    - buttons (disabled)
    - acceleration sensor CMA_INT
    - pressure sensor DRDY
*/
__attribute__((interrupt(PORT2_VECTOR)))
void PORT2_ISR(void)
{
	#ifdef CONFIG_ACCELEROMETER
	/* Check if accelerometer interrupt flag */
	if ((P2IFG & AS_INT_PIN) == AS_INT_PIN)
		as_last_interrupt = 1;
	#endif

	/* A write to the interrupt vector, automatically clears the
	 latest interrupt */
	P2IV = 0x00;
}



