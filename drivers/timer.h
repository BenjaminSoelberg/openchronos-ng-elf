#include <ezchronos.h>

#ifndef __TIMER_H__
#define __TIMER_H__

/* Timer0, the main hw timer */
void timer0_init(void);

/* Programable timer */
void timer0_create_prog_timer(uint16_t duration);
void timer0_destroy_prog_timer();

/* Delay timer */
void timer0_delay(uint16_t duration);

enum timer0_event {
	TIMER0_EVENT_1HZ	= 0x01,
	TIMER0_EVENT_10HZ	= 0x02,
	TIMER0_EVENT_PROG	= 0x04
};

/* exclusive use by openchronos system */
volatile enum timer0_event timer0_last_event;

#endif /* __TIMER_H__ */
