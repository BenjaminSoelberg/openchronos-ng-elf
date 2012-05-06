/*
    drivers/timer.c: Openchronos TA0 timer driver

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

#include "timer.h"

/* source is 32kHz with /2 divider */
#define TIMER0_FREQ 16000

/* list of interrupt sources */
enum timer0_IS {
	TIMER0_IS_CCR0 = 0,
	TIMER0_IS_CCR1 = TA0IV_TA0CCR1,
	TIMER0_IS_CCR2 = TA0IV_TA0CCR2,
	TIMER0_IS_CCR3 = TA0IV_TA0CCR3,
	TIMER0_IS_CCR4 = TA0IV_TA0CCR4,
	TIMER0_IS_OVERFLOW = TA0IV_TA0IFG
};


static struct {
	uint8_t inuse:1;
	uint8_t mode:1; /* 0: timer, 1: delay */
	uint16_t ticks;
	void (*callback_fn)(void);
} timer0_timers[5];


/* this function setups a 1Hz timer ticked every overflow interrupt */
void timer0_init(void)
{
	/* disable overflow interrupts */
	TA0CCTL0 &= ~CCIE;

	/* select external 32kHz source, /2 divider, continous mode */
	TA0CTL |= TASSEL__ACLK | ID__2 | MC__CONTINOUS;

	/* init timers vector */
	uint8_t i = 0;
	while (i < sizeof(timer0_timers))
		timer0_timers[i++].inuse = 0;
}

static int8_t timer0_find_free_timer(void)
{
	int8_t i = -1;
	while (++i < sizeof(timer0_timers)) {
		if (!timer0_timers[i].inuse)
			return i;
	}
	return -1;
}

/* This function was based on original Texas Instruments implementation,
   see LICENSE-TI for more information. */
int8_t timer0_delay(uint16_t duration)
{
	int8_t tid = timer0_create_timer(duration, NULL);
	
	/* no cookie for you */
	if (tid < 0)
		return -1;

	timer0_timers[tid].mode = 1;

	timer0_start_timer(tid);

	/* Wait for interrupt */
	while (1) {
		/* enter low power mode */
		// Go to LPM3
		_BIS_SR(LPM3_bits + GIE);
		__no_operation();

#ifdef USE_WATCHDOG
		/* Service watchdog */
		WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK + WDTCNTCL;
#endif

		/* The interrupt routine sets ticks to zero to signal us
		   that a interrupt happened */
		if (timer0_timers[tid].ticks == 0)
			break;
	}

	timer0_destroy_timer(tid);

	return tid;
}

/* create a timer:
 * duration is in miliseconds, min=1, max=1000 */
int8_t timer0_create_timer(uint16_t duration, void (*callback_fn)(void))
{
	__disable_interrupt();
	int8_t tid = timer0_find_free_timer();

	/* no cookie for you */
	if (tid < 0)
		return -1;

	timer0_timers[tid].inuse = 1;
	__enable_interrupt();

	timer0_timers[tid].mode = 0;
	timer0_timers[tid].ticks = (TIMER0_FREQ / 1000) * duration;
	timer0_timers[tid].callback_fn = callback_fn;

	return tid;
}

void timer0_destroy_timer(int8_t tid)
{
	/* stop timer, just in case */
	timer0_stop_timer(tid);

	/* free up struct */
	timer0_timers[tid].inuse = 0;
}

void timer0_start_timer(int8_t tid)
{
	uint16_t *ta0ccrn = (uint16_t *)TA0CCR0 + tid*2;
	uint16_t *ta0cctln = (uint16_t *)TA0CCTL0 + tid*2;

	/* Set next CCR match */
	*ta0ccrn = TA0R + timer0_timers[tid].ticks;

	/* enable interrupt */
	*ta0cctln |= CCIE;
}

void timer0_stop_timer(int8_t tid)
{
	uint16_t *ta0cctln = (uint16_t *)TA0CCTL0 + tid*2;

	/* disable interrupt */
	*ta0cctln &= ~CCIE;
}

/* unified interrupt vector A0 + A1 */
static void timer0_ISR(enum timer0_IS source)
{
	if (source == TIMER0_IS_OVERFLOW) {
		/* 1Hz timer, nothing to do yet */
		return;
	}
	
	int8_t tid = source >> 1;

	uint16_t *ta0ccrn = (uint16_t *)TA0CCR0 + tid*2;
	uint16_t *ta0cctln = (uint16_t *)TA0CCTL0 + tid*2;

	/* reset flag */
	*ta0cctln &= ~CCIFG;

	if (timer0_timers[tid].mode == 0) {
		/* update CCR for next match */
		*ta0ccrn += timer0_timers[tid].ticks;

		timer0_timers[tid].callback_fn();
	} else
		timer0_timers[tid].ticks = 0;
}

/* interrupt vector for CCR0 */
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void timer0_A0_ISR(void)
{
	timer0_ISR(TIMER0_IS_CCR0);
}

/* interrupt vector for CCR1-4 and overflow */
__attribute__((interrupt(TIMER0_A1_VECTOR)))
void timer0_A1_ISR(void)
{
	timer0_ISR(TA0IV);
}


