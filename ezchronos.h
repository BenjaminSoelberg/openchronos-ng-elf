#ifndef __EZCHRONOS_H__
#define __EZCHRONOS_H__

#include <msp430.h>

#include <stdlib.h>

#include "config.h"

/* main menu */

void menu_add_entry(void (*up_btn_fn)(void),
		    void (*down_btn_fn)(void),
		    void (*num_btn_fn)(void),
		    void (*lstar_btn_fn)(void),
		    void (*lnum_btn_fn)(void),
		    void (*activate_fn)(void),
		    void (*deactivate_fn)(void));

void menu_item_next(void);

/* edit mode */
void menu_editmode_start(void (* value_fn)(int8_t),
			 void (* next_item_fn)(void),
			 void (* complete_fn)(void));

/* Include function defined in even_in_range.s TODO: do we even need this?? */
unsigned short __even_in_range(unsigned short __value, unsigned short __bound);

/* Helpers here */
typedef void(* helpers_loop_fn_t)(uint8_t *, uint8_t, uint8_t);
void helpers_loop_up(uint8_t *value, uint8_t lower, uint8_t upper);
void helpers_loop_down(uint8_t *value, uint8_t lower, uint8_t upper);

/* system message bus */
/* WARNING: the enum values are optimized to work with some drivers.
	If you need to add a new entry, append it to the end! */
enum sys_message {
	/* drivers/rtca */
	SYS_MSG_RTC_ALARM		= BIT0,
	SYS_MSG_RTC_MINUTE	= BIT1,
	SYS_MSG_RTC_HOUR		= BIT2,
	SYS_MSG_RTC_DAY		= BIT3,
	SYS_MSG_RTC_MONTH		= BIT4,
	SYS_MSG_RTC_YEAR		= BIT5,
	/* drivers/timer */
	SYS_MSG_TIMER_1HZ		= BIT6,
	SYS_MSG_TIMER_10HZ	= BIT7,
	SYS_MSG_TIMER_PROG	= BIT8,
};

struct sys_messagebus {
	void (*fn)(enum sys_message);
	enum sys_message listens;
	struct sys_messagebus *next;
};

void sys_messagebus_register(void (*callback)(enum sys_message),
                             enum sys_message listens);
void sys_messagebus_unregister(void (*callback)(enum sys_message));

#endif /* __EZCHRONOS_H__ */
