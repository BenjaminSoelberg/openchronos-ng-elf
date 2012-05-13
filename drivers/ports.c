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


/* Port, pins and interrupt resources for buttons */
#define BUTTONS_IN				(P2IN)
#define BUTTONS_OUT				(P2OUT)
#define BUTTONS_DIR				(P2DIR)
#define BUTTONS_REN				(P2REN)
#define BUTTONS_IE				(P2IE)
#define BUTTONS_IES				(P2IES)
#define BUTTONS_IFG				(P2IFG)
#define BUTTONS_IRQ_VECT2		(PORT2_VECTOR)

#define ALL_BUTTONS				0x1F

#define BIT_IS_SET(F, B) ((F) | (B)) == (F)

/* Button debounce time (ms) */
#define BUTTONS_DEBOUNCE_TIME	5

/* How long does a button need to be pressed to be long press? */
/* in multiples of 1/10 second */
#define BUTTONS_LONG_PRESS_TIME 3

/* check which one of those need volatile */

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
	if ((BUTTONS_IFG & ALL_BUTTONS) == 0)
		return;

	/* get mask for buttons in rising edge */
	uint8_t rising_mask = ~BUTTONS_IES & ALL_BUTTONS;

	/* for those, check which ones raised the interrupt, these are
	 the ones that were just pressed */
	uint8_t buttons = BUTTONS_IFG & rising_mask;

	if (buttons)
		last_press = timer0_10hz_counter;

	/* set pressed button IRQ triggers to falling edge,
	 so we can detect when they are released */
	BUTTONS_IES |= buttons;
		
	/* now get mask for buttons on falling edge
	  (except the ones we just set) */
	uint8_t falling_mask = BUTTONS_IES & ALL_BUTTONS & ~rising_mask;

	/* now for those check which ones raised the interrupt, these are
	  the ones that were just released */
	buttons = BUTTONS_IFG & falling_mask;

	/* if a single button was released, then release all the others */
	if (buttons) {
		buttons |= BUTTONS_IES;

		/* check if button was pressed long enough */
		if (timer0_10hz_counter - last_press > BUTTONS_LONG_PRESS_TIME)
			buttons <<= 5;
		
		/* save pressed buttons */
		ports_pressed_btns |= buttons;

		/* set buttons IRQ triggers to rising edge */
		BUTTONS_IES &= ~ALL_BUTTONS;

		/* Exit from LPM3 on RETI */
		_BIC_SR_IRQ(LPM3_bits);
	}

	/* A write to the interrupt vector, automatically clears the
	 latest interrupt */
	P2IV = 0x00;
}


