/*
    drivers/timer.c: Openchronos TA0 timer driver

    Copyright (C) 2012 Aljaž Srebrnič <a2piratesoft@gmail.com>

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

#include <openchronos.h>


#include "buzzer.h"
#include "timer.h"

#define DURATION(note) (note >> 6)
#define OCTAVE(note) ((note >> 4) & 0x0003)
#define PITCH(note) (note & 0x000F)

uint16_t base_notes[13] = {
	0,    /* 0: P  */
	2383, /* 1: A  */
	2249, /* 2: A# */
	2123, /* 3: B  */
	2003, /* 4: C  */
	1891, /* 5: C# */
	1785, /* 6: D  */
	1685, /* 7: D# */
	1590, /* 8: E  */
	1501, /* 9: F  */
	1417, /* A: F# */
	1337, /* B: G  */
	1262  /* C: G# */
};

inline void buzzer_init(void)
{
	/* Reset TA1R, TA1 runs from 32768Hz ACLK */
	TA1CTL = TACLR | TASSEL__SMCLK | MC__STOP;

	/* Enable IRQ, set output mode "toggle" */
	TA1CCTL0 = OUTMOD_4;

	/* Play "welcome" chord: A major */
	note welcome[4] = {0x1901, 0x1904, 0x1908, 0x000F};
	buzzer_play(welcome);
}

inline void buzzer_stop(void)
{
	/* Stop PWM timer */
	TA1CTL &= ~MC_3;

	/* Disable buzzer PWM output */
	P2OUT &= ~BIT7;
	P2SEL &= ~BIT7;

	/* Clear PWM timer interrupt */
	TA1CCTL0 &= ~CCIE;
}

void buzzer_play(note *notes)
{

	/* Allow buzzer PWM output on P2.7 */
	P2SEL |= BIT7;

	/* 0x000F is the "stop bit" */
	while (PITCH(*notes) != 0x000F) {
		if (PITCH(*notes) == 0) {
			/* Stop the timer! We are playing a rest */
			TA1CTL &= ~MC_3;
		} else {
			/* Set PWM frequency */
			TA1CCR0 = base_notes[PITCH(*notes)] >> OCTAVE(*notes);

			/* Start the timer */
			TA1CTL |= MC__UP;
		}

		/* Delay for DURATION(*notes) milliseconds,
		   use LPM1 because we need SMCLK for tone generation */
		timer0_delay(DURATION(*notes), LPM1_bits);

		/* Advance to the next note */
		notes++;
	}

	/* Stop buzzer */
	buzzer_stop();
}
