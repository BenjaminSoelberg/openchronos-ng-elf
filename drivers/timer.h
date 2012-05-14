/*!
	\file timer.h
	\brief openchronos-ng timer driver
	\details This driver takes care of the Timer0 hardware timer. From this hardware timer the driver produces two hardware-based timers running at 10Hz and 1Hz. The events produced by those timers are available in #sys_message. Beyound the fixed frequency timers, this driver also implements a programmable timer and a programmable delay.
	\note If you are looking to timer events, then see #sys_message
*/

#include <openchronos.h>

#ifndef __TIMER_H__
#define __TIMER_H__

/*!
	\brief Initializes the timer
	\details This functions is called once upon system initialization.
	\note Modules are strictly forbidden to call this function.
	\internal
*/
void timer0_init(void);

/*!
	\brief 10Hz counter.
	\details This is a counter variable, its value is updated at 10Hz. You can use this to measure timings.
	\note counter overflows should be relatively safe since they only happen once each 6553.6 seconds. However you should handle overflows if your application cannot accept sporadic failures in measurement.
*/
volatile uint16_t timer0_10hz_counter;

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
	\sa timer0_destroy_prog_timer
*/
void timer0_destroy_prog_timer();

/*!
	\brief 1ms - 1s programmable delay
	\details delays execution for \b duration milliseconds. During the delay, interrupts are still generated but #sys_message only broadcasts the events after the delay has finished.
	\note Please avoid using this. No processing is done in the background during the delay, which can have impact in modules that require a responsive system.
*/
void timer0_delay(
	uint16_t duration /*!< delay duration between 1 and 1000 milliseconds */
);

/*!
	\brief Bitfield of events produced by this driver
*/
enum timer0_event {
	TIMER0_EVENT_1HZ	= BIT0,	/*!< 1Hz event */
	TIMER0_EVENT_10HZ	= BIT1,	/*!< 10Hz event */
	TIMER0_EVENT_PROG	= BIT2	/*!< programmable timer event */
};

/*!
	\brief Bitfield holding the last generated (timer) event.
	\details Timer0 interrupt routines update this variable with the last happening events. Inside the mainloop, the system then clears this variable after reading it. Read events are broadcasted in #sys_message.
	\note This function is to be used exclusively by the system. No module is allowed to read or write to this variable.
	\internal
*/
volatile enum timer0_event timer0_last_event;

#endif /* __TIMER_H__ */
