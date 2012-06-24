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

#define BIT_IS_SET(F, B) (((F) | (B)) == (F))

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

	/* If the interrupt is not a button press, then handle accel */
	if ((P2IFG & ALL_BUTTONS) == 0)
		goto accel_handler;

	/* get mask for buttons in rising edge */
	uint8_t rising_mask = ~P2IES & ALL_BUTTONS;

	/* for those, check which ones raised the interrupt, these are
	 the ones that were just pressed */
	uint8_t buttons = P2IFG & rising_mask;

	if (buttons)
		last_press = timer0_20hz_counter;

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

#ifdef CONFIG_TIMER_20HZ_IRQ
		uint16_t pressed_ticks = timer0_20hz_counter - last_press;
#else
		/* in case timer is disabled, at least detect short presses */
		uint16_t pressed_ticks = CONFIG_BUTTONS_SHORT_PRESS_TIME;
#endif

		/* check how long btn was pressed and save the event */
		if (pressed_ticks > CONFIG_BUTTONS_LONG_PRESS_TIME)
			ports_pressed_btns |= buttons << 5;
		else if (pressed_ticks >= CONFIG_BUTTONS_SHORT_PRESS_TIME)
			ports_pressed_btns |= buttons;

		/* set buttons IRQ triggers to rising edge */
		P2IES &= ~ALL_BUTTONS;

		/* Exit from LPM3 on RETI */
		_BIC_SR_IRQ(LPM3_bits);
	}

accel_handler:
	#ifdef CONFIG_ACCELEROMETER
	/* Check if accelerometer interrupt flag */
	if ((P2IFG & AS_INT_PIN) == AS_INT_PIN)
		as_last_interrupt = 1;
	#endif

	/* A write to the interrupt vector, automatically clears the
	 latest interrupt */
	P2IV = 0x00;
}



