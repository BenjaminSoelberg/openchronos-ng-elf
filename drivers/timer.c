/**
    drivers/timer.c: Openchronos TA0 timer driver

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016-2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

/*    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


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

#include "timer.h"
#include "wdt.h"
#include "utils.h"
#include "lpm.h"

/* HARDWARE TIMER ASSIGNMENT:
     TA0CCR0: 20Hz timer used by the button driver
     TA0CCR1: Unused
     TA0CCR2: delay timer with callback
     TA0CCR3: programmable timer via messagebus
     TA0CCR4: timer0_delay, will enter LPMx to save power
    OVERFLOW: 0.244Hz timer ~ 4.1S via messagebus
 */

/* source is ACLK=32768Hz (nominal) with /2 divider */
#define TIMER0_FREQ 16384

/* converts microseconds to clock ticks */
#define TIMER0_TICKS_FROM_MS(T) ((((uint32_t)TIMER0_FREQ) * (uint32_t)T) \
                               / ((uint32_t)1000))

static volatile uint8_t delay_finished;

/* programable timer */
static uint16_t timer0_prog_ticks;

static void (*delay_callback)(void) = NULL;

void init_timer0_20hz();

void timer0_init(void) {
#ifdef CONFIG_TIMER_4S_IRQ
    /* Enable overflow interrupts */
    TA0CTL |= TAIE;
#endif

    /* select external 32kHz source, /2 divider, continuous mode */
    TA0CTL |= TASSEL__ACLK | ID__2 | MC__CONTINUOUS;
    /* SLAU259 page 399: clear internal divider counts after setting mode, clock and divisor*/ 
    TA0CTL |= TACLR;
    init_timer0_20hz();
}

/* ----------------------------------------------------------------------------------------------------- */
/* Below functions are used by modules to request a 20 hz event stream, they are (should) be thread safe */
/* ----------------------------------------------------------------------------------------------------- */

/* Reference count for the timer0 20hz */
static volatile uint8_t ref_count_20hz = 0;

/* 20hz timer */
static uint16_t timer0_20hz_ticks;

/* Initialize the 20Hz timer ticks */
void init_timer0_20hz() {
    timer0_20hz_ticks = TIMER0_TICKS_FROM_MS(50);
}

void start_timer0_20hz() {
    uint16_t int_state;
    ENTER_CRITICAL_SECTION(int_state);
    if (ref_count_20hz == 0) {
        /*50ms from <now> generate compare interrupt*/
        TA0CCR0 = TA0R + timer0_20hz_ticks;
        TA0CCTL0 |= CCIE; // Enable timer0 20hz interrupt
    }
    ref_count_20hz++;
    EXIT_CRITICAL_SECTION(int_state);
}

void stop_timer0_20hz() {
    uint16_t int_state;
    ENTER_CRITICAL_SECTION(int_state);
    ref_count_20hz--;
    if (ref_count_20hz == 0) {
        TA0CCTL0 &= ~CCIE; // Disable timer0 20hz interrupt
    }
    EXIT_CRITICAL_SECTION(int_state);
}

/* ----------------------------- */
/* Various timer/delay functions */
/* ----------------------------- */

/* This function was based on original Texas Instruments implementation,
   see LICENSE-TI for more information. */
void timer0_delay(uint16_t duration, uint16_t LPM_bits) {
    delay_finished = 0;

    /* Set next CCR match */
    TA0CCR4 = TA0R + TIMER0_TICKS_FROM_MS(duration);

    /* enable interrupt */
    TA0CCTL4 |= CCIE;

    /* Wait for interrupt */
    while (1) {
        /* enter low power mode */
        enter_lpm_gie(LPM_bits);

        // Service watchdog (reset counter)
        wdt_poll();

        /* The interrupt routine sets delay_finished to signal us
           that a interrupt happened */
        if (delay_finished == 1)
            break;
    }

    /* disable interrupt */
    TA0CCTL4 &= ~CCIE;
}

void timer0_delay_callback_destroy(void) {
    /* abort a delay without calling callback */
    /* disable interrupt */
    TA0CCTL2 &= ~CCIE;

    /* clear any pending interrupt? */
    TA0CCTL2 = 0;

    /* clear callback */
    delay_callback = NULL;
}

/*
 * Please note: The callback will be called from interrupt
 * context and that LPM state isn't changed after execution
 */
void timer0_delay_callback(uint16_t duration, void(*cbfn)(void)) {
    timer0_delay_callback_destroy();

    /* setup where to go on completion */
    delay_callback = cbfn;

    /* Set next CCR match */
    TA0CCR2 = TA0R + TIMER0_TICKS_FROM_MS(duration);

    /* clear any pending interrupt? */
    TA0CCTL2 = 0;

    /* enable interrupt */
    TA0CCTL2 |= CCIE;
}


/* programable timer:
    duration is in miliseconds, min=1, max=1000 */
void timer0_create_prog_timer(uint16_t duration) {
    timer0_prog_ticks = TIMER0_TICKS_FROM_MS(duration);

    /* set timer to start as soon as possible */
    TA0CCR3 = TA0R + timer0_prog_ticks;

    /* enable timer */
    TA0CCTL3 |= CCIE;
}

void timer0_destroy_prog_timer() {
    /* disable timer */
    TA0CCTL3 &= ~CCIE;
}

/* ------------------------ */
/* Timer interrupt handlers */
/* ------------------------ */

/* interrupt vector for CCR0 */
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer0_A0_ISR(void) {
    /* setup timer for next time */
    TA0CCR0 = TA0R + timer0_20hz_ticks;

    /* increase 20hz counter */
    timer0_20hz_counter++;

    /* store 20hz timer event */
    timer0_last_event |= TIMER0_EVENT_20HZ;

    /* exit from LPM3, give execution back to mainloop */
    _BIC_SR_IRQ(LPM3_bits);
}

/* interrupt vector for CCR1-4 and overflow */
__attribute__((interrupt(TIMER0_A1_VECTOR)))
void timer0_A1_ISR(void) {
    /* reading TA0IV automatically resets the interrupt flag */
    uint8_t flag = (uint8_t) TA0IV; // ISR reason. Only look at the lower 8 bits

    /* programable timer */
    if (flag == TA0IV_TA0CCR3) {
        /* setup timer for next time */
        TA0CCR3 = TA0R + timer0_prog_ticks;

        /* store event */
        timer0_last_event |= TIMER0_EVENT_PROG;

        goto exit_lpm3;
    }

    /* delay timer */
    if (flag == TA0IV_TA0CCR4) {
        delay_finished = 1;
        goto exit_lpm3;
    }

    /* one-shot delay timer with callback */
    if (flag == TA0IV_TA0CCR2) {
        /* disable interrupt */
        TA0CCTL2 &= ~CCIE;

        /* maybe call vector */
        if (delay_callback) {
            void (*tmpfn)(void) = delay_callback;
            /* reset timer so it's not called again */
            delay_callback = NULL;
            /* but then it may be re-set by callback fn */
            tmpfn();
        }

        /* return to LPMx (don't mess with SR bits) */
        return;
    }

#ifdef CONFIG_TIMER_4S_IRQ
    /* 0.24Hz timer, ticked by overflow interrupts */
    if (flag == TA0IV_TA0IFG) {
        /* store event */
        timer0_last_event |= TIMER0_EVENT_4S;

        goto exit_lpm3;
    }
#endif

    return;

exit_lpm3:
    /* exit from LPM3, give execution back to mainloop */
    _BIC_SR_IRQ(LPM3_bits);
}


