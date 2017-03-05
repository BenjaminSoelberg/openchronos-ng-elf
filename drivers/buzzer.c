/**
    drivers/timer.c: Openchronos TA0 timer driver

    Copyright (C) 2012 Aljaž Srebrnič <a2piratesoft@gmail.com>
    Copyright (C) 2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

    This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "buzzer.h"
#include "timer.h"

#define DURATION(note) (note >> 6)
#define OCTAVE(note) ((note >> 4) & 0x0003)
#define PITCH(note) (note & 0x000F)

note welcome[4] = {0x1931, 0x1934, 0x1938, 0x000F};

// The following note table is calculated using "clock frequency in hz / sound frequency in hz"
uint16_t base_notes[13] = {
        0,     /* 0: P  */
        27273, /* 1: A  */
        25742, /* 2: A# */
        24397, /* 3: B  */
        22934, /* 4: C  */
        21646, /* 5: C# */
        20431, /* 6: D  */
        19285, /* 7: D# */
        18202, /* 8: E  */
        17181, /* 9: F  */
        16216, /* A: F# */
        15306, /* B: G  */
        14447  /* C: G# */
};

volatile static note *notes = NULL;

inline bool is_buzzer_playing() {
    return notes != NULL;
}

inline void buzzer_init(void) {
    /* Reset TA1R, TA1 runs from 32768Hz ACLK */
    TA1CTL = TACLR | TASSEL__SMCLK | MC__STOP;

    /* Enable IRQ, set output mode "toggle" */
    TA1CCTL0 = OUTMOD_4;

    /* Play "welcome" chord: A major */
    buzzer_play(welcome);
}

inline static void buzzer_stop(void) {
    /* Stop PWM timer */
    TA1CTL &= ~MC_3; // Clear any MC bits, effectively a MC_STOP

    /* Disable buzzer PWM output */
    P2OUT &= ~BIT7;
    P2SEL &= ~BIT7;

    /* Clear PWM timer interrupt */
    TA1CCTL0 &= ~CCIE;
}

static void buzzer_play_callback() {
    /* 0x000F is the "stop bit" */
    if (PITCH(*notes) != 0x000F) {
        if (PITCH(*notes) == 0) {
            /* Stop the timer! We are playing a rest */
            TA1CTL &= ~MC_3;
        } else {
            /* Set PWM frequency */
            TA1CCR0 = base_notes[PITCH(*notes)] >> OCTAVE(*notes);

            /* Start the timer */
            TA1CTL |= MC__UP;
        }

        /* Calculate duration delay in ms */
        uint16_t delay = DURATION(*notes);

        /* Advance to the next note */
        notes++;

        /* Delay for DURATION(*notes) milliseconds */
        timer0_delay_callback(delay, &buzzer_play_callback);
    } else {
        /* Stop buzzer */
        buzzer_stop();
        notes = NULL;
    }
}

void buzzer_play(note *async_notes) {
    if (notes) return; // Ignore if we are currently playing.

    notes = async_notes;

    /* Allow buzzer PWM output on P2.7 */
    P2SEL |= BIT7;

    buzzer_play_callback();
}