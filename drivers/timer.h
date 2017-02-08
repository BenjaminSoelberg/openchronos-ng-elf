/**
    drivers/timer.h: Openchronos TA0 timer driver

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

/*!
    \file timer.h
    \brief openchronos-ng timer driver
    \details This driver takes care of the Timer0 hardware timer. From this hardware timer the driver produces two hardware-based timers running at 20Hz and 4s (period). The events produced by those timers are available in #sys_message. Beyound the fixed frequency timers, this driver also implements a programmable timer and a programmable delay.
    \note If you are looking to timer events, then see #sys_message
*/

#include "openchronos.h"

#ifndef __TIMER_H__
#define __TIMER_H__

void start_timer0_20hz();
void stop_timer0_20hz();

/*!
    \brief Initializes the timer
    \details This functions is called once upon system initialization.
    \note Modules are strictly forbidden to call this function.
    \internal
*/
void timer0_init(void);

/*!
    \brief 20Hz counter.
    \details This is a counter variable, its value is updated at 20Hz. You can use this to measure timings.
    \note counter overflows should be relatively safe since they only happen once each 3276.8 seconds. However you should handle overflows if your application cannot accept sporadic failures in measurement.
*/
volatile uint16_t timer0_20hz_counter;

/*!
    \brief creates a 1000Hz - 1Hz programmable timer
    \details Creates a timer programmable from 1Hz up to 1000Hz. The timer event is available in #sys_message.
    \note You should check what modules are using this function because it cannot be used by more than one module at same time. We recommend you use one of the available fixed timers. If you really need another ticking frequency, contact the openchronos-ng developers for a better solution.
    \sa timer0_destroy_prog_timer
*/
void timer0_create_prog_timer(
    uint16_t duration /*!< timer period between 1 and 1000 milliseconds. */
);

/*!
    \brief destroys a running programmable timer
    \sa timer0_create_prog_timer
*/
void timer0_destroy_prog_timer();

/*!
    \brief 1ms - 1s programmable delay
    \details delays execution for \b duration milliseconds. During the delay, interrupts are still generated but #sys_message only broadcasts the events after the delay has finished.
    The second argument is put directly into the _STATUS_ register. This could of course disrupt the state of the watch. Please be careful and take a look at http://mspgcc.sourceforge.net/manual/x1028.html for information regarding the _STATUS_ register.
    \note Please avoid using this. No processing is done in the background during the delay, which can have impact in modules that require a responsive system.
*/
void timer0_delay(
    uint16_t duration, /*!< delay duration between 1 and 1000 milliseconds */
            uint16_t LPM_bits  /*!< LPM bits to put in the status register, so the user can choose LPM level */
);

/*!
      \brief schedule a callback after a delay
 */

/*!
    \brief 1ms - 1s programmable delay to callback
    \details schedules a callback to the provided function after \b duration milliseconds.  Does not suspend other interrupts.
 */
void timer0_delay_callback(
    uint16_t duration, /*!< delay duration in ms */
    void(*cbfn)(void) /*!< pointer to function to call on completion */
);

/*!
    \brief abort any pending delay callback
 */
void timer0_delay_callback_destroy(void);

/*!
    \brief Bitfield of events produced by this driver
*/
enum timer0_event {
    TIMER0_EVENT_NONE   = 0,    /*!< EMPTY EVENT */
#ifdef CONFIG_TIMER_4S_IRQ
    TIMER0_EVENT_4S     = BIT0, /*!< 0.24Hz ~ 4.1s period event */
#endif
    TIMER0_EVENT_20HZ   = BIT1, /*!< 20Hz event */
    TIMER0_EVENT_PROG   = BIT2  /*!< programmable timer event */
};

/*!
    \brief Bitfield holding the last generated (timer) event.
    \details Timer0 interrupt routines update this variable with the last happening events. Inside the mainloop, the system then clears this variable after reading it. Read events are broadcasted in #sys_message.
    \note This function is to be used exclusively by the system. No module is allowed to read or write to this variable.
    \internal
*/
volatile enum timer0_event timer0_last_event;

#endif /* __TIMER_H__ */
