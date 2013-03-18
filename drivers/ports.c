/* (c) <MartinAusChemnitz@gmx.net>, GNU GPL3 */
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

/* contains buttons currently held down */
volatile enum ports_buttons ports_down_btns;

/* contains confirmed button presses (long and short) */
volatile enum ports_buttons ports_pressed_btns;

static uint8_t timer_20Hz_requested;
static uint16_t last_press;

/* 0 bit = ignore until release */
static uint8_t silent_until_release = 0xff;

/*
  20 Hz callback for figuring out the buttons
*/
static void callback_20Hz(enum sys_message msg)
{
	static uint8_t last_state;
	uint8_t buttons = P2IN & ALL_BUTTONS;

	ports_down_btns |= ((last_state ^ buttons) & buttons)
			& silent_until_release;
	/*                  (buttons that changed) */
	uint8_t released = ((last_state ^ buttons) & ~buttons)
			& silent_until_release;
	silent_until_release |= ~buttons;
	last_state = buttons;

	uint16_t pressed_ticks = timer0_20hz_counter - last_press;
	/* check how long btn was pressed and save the event */
	if (pressed_ticks > CONFIG_BUTTONS_LONG_PRESS_TIME) {
		/* suppress */
		silent_until_release &= ~buttons;
		ports_pressed_btns |= buttons << 5;
	} else {
		ports_pressed_btns |= released;
	}

	if (!buttons) {
		/* turn 20 Hz callback off */
		sys_messagebus_unregister(&callback_20Hz);
		timer_20Hz_requested = 0;
	}
}

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
  official function to ask for buttons
*/
uint8_t ports_button_pressed(uint8_t btn, uint8_t with_longpress)
{
	if (with_longpress) {
		return BIT_IS_SET(ports_pressed_btns, btn);
	} else {
		if (BIT_IS_SET(ports_down_btns, btn)) {
			/* suppress */
			silent_until_release &= ~btn;
			return 1;
		} else {
			return 0;
		}
	}
}

/*
  official function to ignore all other button presses up to now
*/
void ports_buttons_clear(void)
{
	ports_down_btns = 0;
	ports_pressed_btns = 0;
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
	/* If the interrupt is a button press */
	if (P2IFG & ALL_BUTTONS) {
		/* turn on 20 Hz callback*/
		if (!timer_20Hz_requested) {
			last_press = timer0_20hz_counter;
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



