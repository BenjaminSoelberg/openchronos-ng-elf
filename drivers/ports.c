/*	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


	  Redistribution and use in source and binary forms, with or without
	  modification, are permitted provided that the following conditions
	  are met:

	    Redistributions of source code must retain the above copyright
	    notice, this list of conditions and the following disclaimer.

	    Redistributions in binary form must reproduce the above copyright
	    notice, this list of conditions and the following disclaimer in the
	    documentation and/or other materials provided with the
	    distribution.

	    Neither the name of Texas Instruments Incorporated nor the names of
	    its contributors may be used to endorse or promote products derived
	    from this software without specific prior written permission.

	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ezchronos.h>

/* drivers */
#include "ports.h"
#include "timer.h"

#include "vti_as.h"
#include "vti_ps.h"

#include "display.h"


/* Macro for button IRQ */
#define IRQ_TRIGGERED(flags, bit)		((flags & bit) == bit)

/* check which one of those need volatile */
static uint8_t star_timeout;
static uint8_t num_timeout;
static uint8_t backlight_timeout;
static uint8_t backlight_status;

void buttons_pooling_fn(enum sys_message msg)
{
	/* Detect continuous button high states */
	if (BUTTON_STAR_IS_PRESSED) {
		star_timeout++;

		/* Check if button was held low for some seconds */
		if (star_timeout > LEFT_BUTTON_LONG_TIME) {
			ports_buttons.flag.star_long = 1;
			star_timeout = 0;
		}
	} else {
		star_timeout = 0;
	}

	if (BUTTON_NUM_IS_PRESSED) {
		num_timeout++;

		/* Check if button was held low for some seconds */
		if (num_timeout > LEFT_BUTTON_LONG_TIME) {
			ports_buttons.flag.num_long = 1;
			num_timeout = 0;
		}
	} else {
		num_timeout = 0;
	}
}

void init_buttons(void)
{
	/* Set button ports to input */
	BUTTONS_DIR &= ~ALL_BUTTONS;

	/* Enable internal pull-downs */
	BUTTONS_OUT &= ~ALL_BUTTONS;
	BUTTONS_REN |= ALL_BUTTONS;

	/* IRQ triggers on rising edge */
	BUTTONS_IES &= ~ALL_BUTTONS;

	/* Reset IRQ flags */
	BUTTONS_IFG &= ~ALL_BUTTONS;

	/* Enable button interrupts */
	BUTTONS_IE |= ALL_BUTTONS;

	/* register on 1Hz timer */
	sys_messagebus_register(&buttons_pooling_fn, SYS_MSG_TIMER_1HZ);
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
	uint8_t int_flag, int_enable;

	/* Clear button flags */
	ports_buttons.all_flags = 0;

	/* Remember interrupt enable bits */
	int_enable = BUTTONS_IE;

	/* Store valid button interrupt flag */
	int_flag = BUTTONS_IFG & int_enable;

	/* Debounce buttons */
	if ((int_flag & ALL_BUTTONS) != 0) {
		/* Disable PORT2 IRQ */
		__disable_interrupt();
		BUTTONS_IE = 0x00;
		__enable_interrupt();

		/* Debounce delay 1 */
		timer0_delay(BUTTONS_DEBOUNCE_TIME_IN);
	}

	if (IRQ_TRIGGERED(int_flag, BUTTON_STAR_PIN)) {
		/* STAR button IRQ */

		/* Filter bouncing noise */
		if (BUTTON_STAR_IS_PRESSED)
			ports_buttons.flag.star = 1;

	} else if (IRQ_TRIGGERED(int_flag, BUTTON_NUM_PIN)) {
		/* NUM button IRQ */

		/* Filter bouncing noise */
		if (BUTTON_NUM_IS_PRESSED)
			ports_buttons.flag.num = 1;

	} else if (IRQ_TRIGGERED(int_flag, BUTTON_UP_PIN)) {
		/* UP button IRQ */

		/* Filter bouncing noise */
		if (BUTTON_UP_IS_PRESSED)
			ports_buttons.flag.up = 1;
	
	} else if (IRQ_TRIGGERED(int_flag, BUTTON_DOWN_PIN)) {
		/* DOWN button IRQ */

		/* Filter bouncing noise */
		if (BUTTON_DOWN_IS_PRESSED)
			ports_buttons.flag.down = 1;

	} else if (IRQ_TRIGGERED(int_flag, BUTTON_BACKLIGHT_PIN)) {
		/* BACKLIGHT button IRQ */

		/* Filter bouncing noise */
		if (BUTTON_BACKLIGHT_IS_PRESSED) {
			backlight_status = 1;
			backlight_timeout = 0;
			P2OUT |= BUTTON_BACKLIGHT_PIN;
			P2DIR |= BUTTON_BACKLIGHT_PIN;
			ports_buttons.flag.backlight = 1;
		}
	}

	/* Trying to lock/unlock buttons? */
	if ((ports_buttons.flag.num && ports_buttons.flag.down)
	  || (ports_buttons.flag.star && ports_buttons.flag.up)) {
		ports_buttons.all_flags = 0;
	}

	/* Acceleration sensor IRQ */
	if (IRQ_TRIGGERED(int_flag, AS_INT_PIN)) {
		/* Get data from sensor */
		/* TODO: we should do something here */
	}

	/* Pressure sensor IRQ */
	if (IRQ_TRIGGERED(int_flag, PS_INT_PIN)) {
		/* Get data from sensor */
		/* TODO: we should do something here */
	}

	/* Safe long button event detection */
	if (ports_buttons.flag.star || ports_buttons.flag.num) {
		/* Additional debounce delay to enable safe high detection */
		timer0_delay(BUTTONS_DEBOUNCE_TIME_LEFT);

		/* Check if this button event is short enough */
		if (BUTTON_STAR_IS_PRESSED)
			ports_buttons.flag.star = 0;

		if (BUTTON_NUM_IS_PRESSED)
			ports_buttons.flag.num = 0;
	}

	/* Reenable PORT2 IRQ */
	__disable_interrupt();
	BUTTONS_IFG = 0x00;
	BUTTONS_IE  = int_enable;
	__enable_interrupt();

	/* Exit from LPM3 on RETI */
	_BIC_SR_IRQ(LPM3_bits);
}


