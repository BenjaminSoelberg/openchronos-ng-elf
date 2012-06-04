/*
    drivers/ports.c: Openchronos ports driver
	 
	 Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

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

#define BIT_IS_SET(F, B) ((F) | (B)) == (F)

// *************************************************************************************************
// Defines section

// Macro for button IRQ
#define IRQ_TRIGGERED(flags, bit)		((flags & bit) == bit)

/* Button debounce time (ms) */
#define BUTTONS_DEBOUNCE_TIME	5

/* How long does a button need to be pressed to be long press? */
/* in multiples of 1/10 second */
#define BUTTONS_LONG_PRESS_TIME 3

void init_buttons(void)
{
	/* Set button ports to input */
	P2DIR &= ~ALL_BUTTONS;

	/* Enable internal pull-downs */
	P2OUT &= ~ALL_BUTTONS;
	P2REN |= ALL_BUTTONS;

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
	static uint16_t last_press;

	/* If the interrupt was not raised by a button press, then return */
	if ((P2IFG & ALL_BUTTONS) == 0)
		return;

	/* get mask for buttons in rising edge */
	uint8_t rising_mask = ~P2IES & ALL_BUTTONS;

	/* for those, check which ones raised the interrupt, these are
	 the ones that were just pressed */
	uint8_t buttons = P2IFG & rising_mask;

	if (buttons)
		last_press = timer0_10hz_counter;

	#ifdef CONFIG_ACCELEROMETER
	// Accelerometer is on rising edge in the default configuration
	if (IRQ_TRIGGERED(buttons, AS_INT_PIN))
	{
		// Get data from sensor
		as_last_interrupt = 1;
	}
	#endif

	/* set pressed button IRQ triggers to falling edge,
	 so we can detect when they are released */
	P2IES |= buttons;
		
	/* now get mask for buttons on falling edge
	  (except the ones we just set) */
	uint8_t falling_mask = P2IES & ALL_BUTTONS & ~rising_mask;

	/* now for those check which ones raised the interrupt, these are
	  the ones that were just released */
	buttons = P2IFG & falling_mask;

	/* if a single button was released, then release all the others */
	if (buttons) {
		buttons |= P2IES;

		/* check if button was pressed long enough */
		if (timer0_10hz_counter - last_press > BUTTONS_LONG_PRESS_TIME)
			buttons <<= 5;
		
		/* save pressed buttons */
		ports_pressed_btns |= buttons;

		/* set buttons IRQ triggers to rising edge */
		P2IES &= ~ALL_BUTTONS;

		/* Exit from LPM3 on RETI */
		_BIC_SR_IRQ(LPM3_bits);
	}



	/* A write to the interrupt vector, automatically clears the
	 latest interrupt */
	P2IV = 0x00;
}



