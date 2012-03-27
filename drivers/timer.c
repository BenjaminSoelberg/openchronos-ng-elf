/*
    Copyright (C) 2011 Angelo Arrifano <miknix@gmail.com>
	   - Improved message display API, with timeout feature
		- Improved timer0 API, add register and unregister functions

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
// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//	  Redistribution and use in source and binary forms, with or without
//	  modification, are permitted provided that the following conditions
//	  are met:
//
//	    Redistributions of source code must retain the above copyright
//	    notice, this list of conditions and the following disclaimer.
//
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the
//	    documentation and/or other materials provided with the
//	    distribution.
//
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Timer service routines.
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
#include "project.h"

// driver
#include "timer.h"
#include "ports.h"
#include "buzzer.h"
#include "vti_ps.h"
#ifdef FEATURE_PROVIDE_ACCEL
#include "vti_as.h"
#endif
#include "display.h"
#include "rtca.h"

// logic
#include "battery.h"
#include "stopwatch.h"
#include "altitude.h"
#include "display.h"
#include "rfsimpliciti.h"
#include "simpliciti.h"
#ifdef FEATURE_PROVIDE_ACCEL
#include "acceleration.h"
#endif

#include "temperature.h"

#ifdef CONFIG_SIDEREAL
#include "sidereal.h"
#endif

#if (CONFIG_DST > 0)
#include "dst.h"
#include "date.h"
#endif

#include <stdlib.h>

// *************************************************************************************************
// Prototypes section
void Timer0_Init(void);
void Timer0_Stop(void);
void Timer0_A1_Start(uint16_t ticks);
void Timer0_A1_Stop(void);
void Timer0_A3_Start(uint16_t ticks);
void Timer0_A3_Stop(void);
void Timer0_A4_Delay(uint16_t ticks);
void (*fptr_Timer0_A3_function)(void);
#ifdef CONFIG_USE_GPS
void (*fptr_Timer0_A1_function)(void);
#endif

// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section
struct timer sTimer;

// *************************************************************************************************
// Extern section
extern void BRRX_TimerTask_v(void);
extern void to_lpm(void);

#ifdef CONFIG_ALTI_ACCUMULATOR
extern uint8_t alt_accum_enable; // 1 means the altitude accumulator is enabled
#endif

// *************************************************************************************************
// @fn          Timer0_Init
// @brief       Set Timer0 to a period of 1 or 2 sec. IRQ TACCR0 is asserted when timer overflows.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Init(void)
{
	// Set interrupt frequency to 1Hz
	TA0CCR0   = 32768 - 1;

	// Enable timer interrupt
	TA0CCTL0 |= CCIE;

	// Clear and start timer now
	// Continuous mode: Count to 0xFFFF and restart from 0 again - 1sec timing will be generated by ISR
	TA0CTL   |= TASSEL0 + MC1 + TACLR;

	// Init callback queue
	sTimer.queue = NULL;
}


// *************************************************************************************************
// @fn          Timer0_Start
// @brief       Start Timer0.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Start(void)
{
	// Start Timer0 in continuous mode
	TA0CTL |= MC_2;
}


// *************************************************************************************************
// @fn          Timer0_Stop
// @brief       Stop and reset Timer0.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_Stop(void)
{
	// Stop Timer0
	TA0CTL &= ~MC_2;

	// Set Timer0 count register to 0x0000
	TA0R = 0;
}


void Timer0_A1_Start(uint16_t ticks)
{
	/*old version
	// Set interrupt frequency to 1Hz
	TA0CCR1   = TA0R + 32678 ;

	// Enable timer interrupt
	TA0CCTL1 |= CCIE; */

	uint16_t value;

	// Store timer ticks in global variable
	sTimer.timer0_A1_ticks = ticks;

	// Delay based on current counter value
	value = TA0R + ticks;

	// Update CCR
	TA0CCR1 = value;

	// Reset IRQ flag
	TA0CCTL1 &= ~CCIFG;

	// Enable timer interrupt
	TA0CCTL1 |= CCIE;

}

void Timer0_A1_Stop(void)
{
	// Clear timer interrupt
	TA0CCTL1 &= ~CCIE;
}


void Timer0_A1_Register(void (*callback)(void))
{
	struct cbList **p = &sTimer.queue;

	while (*p) {
		p = &(*p)->next;
	}

	*p = malloc(sizeof(struct cbList));
	(*p)->next = NULL;
	(*p)->fn = callback;
}

void Timer0_A1_Unregister(void (*callback)(void))
{
	struct cbList *p = sTimer.queue, *pp = NULL;

	while (p) {
		if (p->fn == callback) {
			if (!pp)
				sTimer.queue = p->next;
			else
				pp->next = p->next;

			free(p);
		}

		pp = p;
		p = p->next;
	}
}

// *************************************************************************************************
// @fn          Timer0_A3_Start
// @brief       Trigger IRQ every "ticks" microseconds
// @param       ticks (1 tick = 1/32768 sec)
// @return      none
// *************************************************************************************************
void Timer0_A3_Start(uint16_t ticks)
{
	uint16_t value;

	// Store timer ticks in global variable
	sTimer.timer0_A3_ticks = ticks;

	// Delay based on current counter value
	value = TA0R + ticks;

	// Update CCR
	TA0CCR3 = value;

	// Reset IRQ flag
	TA0CCTL3 &= ~CCIFG;

	// Enable timer interrupt
	TA0CCTL3 |= CCIE;
}



// *************************************************************************************************
// @fn          Timer0_A3_Stop
// @brief       Stop Timer0_A3.
// @param       none
// @return      none
// *************************************************************************************************
void Timer0_A3_Stop(void)
{
	// Clear timer interrupt
	TA0CCTL3 &= ~CCIE;
}


// *************************************************************************************************
// @fn          Timer0_A4_Delay
// @brief       Wait for some microseconds
// @param       ticks (1 tick = 1/32768 sec)
// @return      none
// *************************************************************************************************
void Timer0_A4_Delay(uint16_t ticks)
{
	uint16_t value;

	// Exit immediately if Timer0 not running - otherwise we'll get stuck here
	if ((TA0CTL & (BIT4 | BIT5)) == 0) return;

	// Disable timer interrupt
	TA0CCTL4 &= ~CCIE;

	// Clear delay_over flag
	sys.flag.delay_over = 0;

	// Add delay to current timer value
	value = TA0R + ticks;

	// Update CCR
	TA0CCR4 = value;

	// Reset IRQ flag
	TA0CCTL4 &= ~CCIFG;

	// Enable timer interrupt
	TA0CCTL4 |= CCIE;

	// Wait for timer IRQ
	while (1) {
		// Delay in LPM
		to_lpm();

#ifdef USE_WATCHDOG
		// Service watchdog
		WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK + WDTCNTCL;
#endif
#ifdef CONFIG_STOP_WATCH

		// Redraw stopwatch display
		if (is_stopwatch_run()) display_stopwatch(LINE2, DISPLAY_LINE_UPDATE_PARTIAL);

#endif

		// Check stop condition
		if (sys.flag.delay_over) break;
	}
}



// *************************************************************************************************
// @fn          TIMER0_A0_ISR
// @brief       IRQ handler for TIMER0_A0 IRQ
//				Timer0_A0	1/1sec clock tick 			(serviced by function TIMER0_A0_ISR)
//				Timer0_A1	 							(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A2	1/100 sec Stopwatch			(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A3	Configurable periodic IRQ	(serviced by function TIMER0_A1_5_ISR)
//				Timer0_A4	One-time delay				(serviced by function TIMER0_A1_5_ISR)
// @param       none
// @return      none
// *************************************************************************************************
//pfs
#ifdef __GNUC__
__attribute__((interrupt(TIMER0_A0_VECTOR)))
#else
#pragma vector = TIMER0_A0_VECTOR
__interrupt
#endif
void TIMER0_A0_ISR(void)
{
	// Disable IE
	TA0CCTL0 &= ~CCIE;
	// Reset IRQ flag
	TA0CCTL0 &= ~CCIFG;
	// Add 1 sec to TACCR0 register (IRQ will be asserted at 0x7FFF and 0xFFFF = 1 sec intervals)
	TA0CCR0 += 32768;
	// Enable IE
	TA0CCTL0 |= CCIE;

	// While SimpliciTI stack operates, freeze system state
	if (is_rf()) {
		// SimpliciTI automatic timeout
		if (sRFsmpl.timeout == 0) {
			simpliciti_flag |= SIMPLICITI_TRIGGER_STOP;
		} else {
			sRFsmpl.timeout--;
		}

		// Exit from LPM3 on RETI
		_BIC_SR_IRQ(LPM3_bits);
		return;
	}

	// ----------------------------------------------------
	// Go through the callback queue and call the functions
	struct cbList *p = sTimer.queue;

	while (p) {
		p->fn();
		p = p->next;
	};

	// Do a temperature measurement each second while menu item is active
	if (is_temp_measurement()) request.flag.temperature_measurement = 1;

	// Do a pressure measurement each second while menu item is active
#ifdef CONFIG_ALTITUDE

	if (is_altitude_measurement()) {
		// Countdown altitude measurement timeout while menu item is active
		sAlt.timeout--;

		// Stop measurement when timeout has elapsed
		if (sAlt.timeout == 0) {
			stop_altitude_measurement();
			// Show ---- m/ft
			display_chars(LCD_SEG_L1_3_0, (uint8_t *)"----", SEG_ON);
			// Clear up/down arrow
			display_symbol(LCD_SYMB_ARROW_UP, SEG_OFF);
			display_symbol(LCD_SYMB_ARROW_DOWN, SEG_OFF);
		}

		// In case we missed the IRQ due to debouncing, get data now
		if ((PS_INT_IN & PS_INT_PIN) == PS_INT_PIN) request.flag.altitude_measurement = 1;
	}

#endif

#ifdef FEATURE_PROVIDE_ACCEL

	// Count down timeout
	if (is_acceleration_measurement()) {
		// Countdown acceleration measurement timeout
		sAccel.timeout--;

		// Stop measurement when timeout has elapsed
		if (sAccel.timeout == 0) as_stop();

		// If DRDY is (still) high, request data again
		if ((AS_INT_IN & AS_INT_PIN) == AS_INT_PIN) request.flag.acceleration_measurement = 1;
	}

#endif

#ifdef CONFIG_BATTERY

	// If battery is low, decrement display counter
	if (sys.flag.low_battery) {
		if (sBatt.lobatt_display-- == 0) {
			message.flag.prepare = 1;
			message.flag.timeout = 7;
			message.flag.type_lobatt = 1;
			sBatt.lobatt_display = BATTERY_LOW_MESSAGE_CYCLE;
		}
	}

#endif

	// -------------------------------------------------------------------
	// Check idle timeout, set timeout flag
	if (sys.flag.idle_timeout_enabled) {
		/* TODO: THIS IS BROKEN! FIX ASAP */
	}

	// -------------------------------------------------------------------
	// Turn the Backlight off after timeout
	if (sButton.backlight_status == 1) {
		if (sButton.backlight_timeout > BACKLIGHT_TIME_ON) {
			//turn off Backlight
			P2OUT &= ~BUTTON_BACKLIGHT_PIN;
			P2DIR &= ~BUTTON_BACKLIGHT_PIN;
			sButton.backlight_timeout = 0;
			sButton.backlight_status = 0;
		} else {
			sButton.backlight_timeout++;
		}
	}

	// -------------------------------------------------------------------
	// Detect continuous button high states
	if (BUTTON_STAR_IS_PRESSED) {
		sButton.star_timeout++;

		// Check if button was held low for some seconds
		if (sButton.star_timeout > LEFT_BUTTON_LONG_TIME) {
			button.flag.star_long = 1;
			sButton.star_timeout = 0;
		}
	} else {
		sButton.star_timeout = 0;
	}

	if (BUTTON_NUM_IS_PRESSED) {
		sButton.num_timeout++;

		// Check if button was held low for some seconds
		if (sButton.num_timeout > LEFT_BUTTON_LONG_TIME) {
			button.flag.num_long = 1;
			sButton.num_timeout = 0;
		}
	} else {
		sButton.num_timeout = 0;
	}

	// Exit from LPM3 on RETI
	_BIC_SR_IRQ(LPM3_bits);
}


// *************************************************************************************************
// @fn          Timer0_A1_5_ISR
// @brief       IRQ handler for timer IRQ.
//				Timer0_A0	1/1sec clock tick (serviced by function TIMER0_A0_ISR)
//				Timer0_A1	BlueRobin timer / doorlock
//				Timer0_A2	1/100 sec Stopwatch
//				Timer0_A3	Configurable periodic IRQ (used by button_repeat and buzzer)
//				Timer0_A4	One-time delay
// @param       none
// @return      none
// *************************************************************************************************
#ifdef __GNUC__
__attribute__((interrupt(TIMER0_A1_VECTOR)))
#else
#pragma vector = TIMER0_A1_VECTOR
__interrupt
#endif
void TIMER0_A1_5_ISR(void)
{
	uint16_t value;
	/* Disable IE */
	TA0CCTL1 &= ~CCIE;
	/* IRQ flag automatically resetted when TA0IV is read. */

	/* Priority is highest first. */
	switch (TA0IV) {
	case 0x02:  /* Timer0_A1 handler */
	    break;
	case 0x04:  /* Timer0_A2 handler */
	    break;
	case 0x06:  /* Timer0_A3 handler */
	    /* Store new value in CCR */
	    value = TA0R + sTimer.timer0_A3_ticks; //timer0_A3_ticks_g;
	    /* Load CCR register with next capture point */
	    TA0CCR3 = value;
	    /* Enable timer interrupt */
	    TA0CCTL3 |= CCIE;
	    /* Call function handler */
	    fptr_Timer0_A3_function();
	    break;

	case 0x08:  /* Timer0_A4 handler - Used for one-time delay */
		/* Set delay over flag */
		sys.flag.delay_over = 1;
		break;
	}

	/* Exit from LPM3 on RETI */
	_BIC_SR_IRQ(LPM3_bits);
}
