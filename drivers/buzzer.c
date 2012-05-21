/*
    drivers/timer.c: Openchronos TA0 timer driver

    Copyright (C) 2012 Aljaž Srebrnič <a2piratesoft@gmail.com>

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
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 
 
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

#include <openchronos.h>


#include "buzzer.h"
#include "timer.h"

struct buzzer sBuzzer;

void buzzer_start(uint8_t cycles, uint16_t on_time, uint16_t off_time)
{
	/* Store new buzzer duration while buzzer is off */
	if (sBuzzer.time == 0) {
		sBuzzer.time 	 = cycles;
		sBuzzer.on_time  = on_time;
		sBuzzer.off_time = off_time;

		/* Reset TA1R, set up mode, TA1 runs from 32768Hz ACLK */
		TA1CTL = TACLR | MC_1 | TASSEL__ACLK;

		/* Set PWM frequency */
		TA1CCR0 = sBuzzer.steps;

		/* Enable IRQ, set output mode "toggle" */
		TA1CCTL0 = OUTMOD_4;

		/* Allow buzzer PWM output on P2.7 */
		P2SEL |= BIT7;

		// Activate Timer0_A3 periodic interrupts
		//fptr_Timer0_A3_function = toggle_buzzer;
		//Timer0_A3_Start(sBuzzer.on_time);

		// Preload timer advance variable
		//sTimer.timer0_A3_ticks = sBuzzer.off_time;

		/* Start with buzzer output on */
		sBuzzer.state 	 	= BUZZER_ON_OUTPUT_ENABLED;
	}
}

inline void buzzer_start_steps(uint8_t cycles, uint16_t on_time, uint16_t off_time, uint16_t steps)
{
	sBuzzer.steps = steps;
	start_buzzer(cycles, on_time, off_time);
}

void buzzer_toggle(void)
{
	/* Turn off buzzer */
	if (sBuzzer.state == BUZZER_ON_OUTPUT_ENABLED) {
		/* Stop PWM timer */
		TA1CTL &= ~(BIT4 | BIT5);

		/* Reset and disable buzzer PWM output */
		P2OUT &= ~BIT7;
		P2SEL &= ~BIT7;

		/* Update buzzer state */
		sBuzzer.state = BUZZER_ON_OUTPUT_DISABLED;

		/* Reload Timer0_A4 IRQ to restart output */
		/* Timer.timer0_A3_ticks = sBuzzer.on_time; */
	} else { /* Turn on buzzer */
		/* Decrement buzzer total cycles */
		/* Stop buzzer when reaching 0 cycles */
		if (--sBuzzer.time == 0) {
			stop_buzzer();
		}

		/* Reload Timer0_A3 to stop output if sBuzzer.time > 0 */
		if (sBuzzer.state != BUZZER_OFF) {
			/* Reset timer TA1 */
			TA1R = 0;
			TA1CTL |= MC_1;

			/* Enable buzzer PWM output */
			P2SEL |= BIT7;

			/* Update buzzer state */
			sBuzzer.state = BUZZER_ON_OUTPUT_ENABLED;

			/* Reload Timer0_A4 IRQ to turn off output */
			/* Timer.timer0_A3_ticks = sBuzzer.off_time; */
		}
	}
}


void buzzer_stop(void)
{
	/* Stop PWM timer */
	TA1CTL &= ~(BIT4 | BIT5);

	/* Disable buzzer PWM output */
	P2OUT &= ~BIT7;
	P2SEL &= ~BIT7;

	/* Clear PWM timer interrupt */
	TA1CCTL0 &= ~CCIE;

	/* Disable periodic start/stop interrupts */
	/* imer0_A3_Stop(); */

	/* Clear variables */
	sBuzzer.time 	= 0;
	sBuzzer.state 	= BUZZER_OFF;
	sBuzzer.steps	= BUZZER_TIMER_STEPS;
}
