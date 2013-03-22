/* (c) <MartinAusChemnitz@gmx.net>, GNU GPL3 */
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
 */


#include <openchronos.h>

/* drivers */
#include "ports.h"
#include "timer.h"

#include "display.h"

#ifdef CONFIG_ACCELEROMETER
#include "vti_as.h"
#endif

#define ALL_BUTTONS				0x1F

static uint8_t timer_20Hz_requested;
static uint16_t btns_last_press;

/*
  20 Hz callback for figuring out the buttons
*/
static void callback_20Hz(enum sys_message msg)
{
	/* save buttons that went from pressed to released */
	ports_btns_state = P2IN & ALL_BUTTONS;

	if (!ports_btns_state) {
		/* turn 20 Hz callback off */
		sys_messagebus_unregister(&callback_20Hz);
		timer_20Hz_requested = 0;
	}
}

void init_buttons(void)
{
	/* Some initialization is done at boot.c */

	/* IRQ triggers on rising edge */
	P2IES &= ~ALL_BUTTONS;

	/* Reset IRQ flags */
	P2IFG &= ~ALL_BUTTONS;

	/* Enable button interrupts */
	P2IE |= ALL_BUTTONS;
}



/*
  Interrupt service routine for
    - buttons
    - acceleration sensor CMA_INT
    - pressure sensor DRDY
*/
__attribute__((interrupt(PORT2_VECTOR)))
void PORT2_ISR(void)
{
	uint8_t buttons = P2IFG & ALL_BUTTONS;

	/* If the interrupt is a button press
	   and is not already in pressed state
	   and is outside of debouncing interval */
	if (buttons && buttons ^ ports_btns_state
				&& timer0_20hz_counter - btns_last_press
					>= CONFIG_BTNS_DEBOUNCE_TIME) {

		btns_last_press = timer0_20hz_counter;
		ports_btns_state |= buttons;

		if (!timer_20Hz_requested) {
			sys_messagebus_register(&callback_20Hz,
						SYS_MSG_TIMER_20HZ);
			timer_20Hz_requested = 1;
		}
	}

	/* Handle accelerometer */
	#ifdef CONFIG_ACCELEROMETER
	/* Check if accelerometer interrupt flag */
	if ((P2IFG & AS_INT_PIN) == AS_INT_PIN)
		as_last_interrupt = 1;
	#endif

	/* A write to the interrupt vector, automatically clears the
	 latest interrupt */
	P2IV = 0x00;
}



