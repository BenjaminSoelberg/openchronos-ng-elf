/* A timer for 90-second strength training.
 */
#ifndef STRENGTH_H_
#define STRENGTH_H_

// *************************************************************************************************
// Includes
#include "project.h"

// *************************************************************************************************
// Defines

/**
 * How many seconds to count down from the time the start button is pressed to
 * the time the actual time measurement begins.
 * In this time, the user can lean back, grab the handles and start pulling.
 * Must be <= 9 seconds.
 */
#define STRENGTH_COUNTDOWN_SECS 5

/**
 * Beep
 * this many seconds after the start button was pressed.
 */
#define STRENGTH_THRESHOLD_1 (60+STRENGTH_COUNTDOWN_SECS)

/**
 * Beep again
 * this many seconds after the start button was pressed.
 */
#define STRENGTH_THRESHOLD_2 (90+STRENGTH_COUNTDOWN_SECS)

/**
 * Beep again and stop measuring
 * this many seconds after the start button was pressed.
 */
#define STRENGTH_THRESHOLD_END (200+STRENGTH_COUNTDOWN_SECS)

/**
 * How long to switch on the buzzer
 * (suitable for 2nd param of start_buzzer)
 */
#define STRENGTH_BUZZER_ON_TICKS (CONV_MS_TO_TICKS(100))

/**
 * How long to pause between beeps of the buzzer
 * (suitable for 3rd param of start_buzzer)
 */
#define STRENGTH_BUZZER_OFF_TICKS (CONV_MS_TO_TICKS(100))

/**
 * The entire global state of the Strength Timer.
 */
/* TODO: pack this struct! */
typedef struct
{
	struct 
	{
		/**
		 * 1 if in running state, 0 if in stopped or initial.
		 */
		unsigned running : 1;

		/**
		 * 1 if the logic has recomputed the display contents
		 * and the display function has not yet displayed it.
		 */
		unsigned redisplay_requested : 1;
	} flags;

	/**
	 * The tick function may sometimes decide to beep a certain number
	 * of times; it does not start the buzzer immediately (to keep the timer
	 * ISR short) but deposits the requested number of beeps here.
	 * The process_request function will later start the buzzer for this many beeps
	 * and reset this variable again.
	 */
	uint8_t num_beeps;

	/**
	 * Number of seconds since the start button was pressed.
	 */
	uint8_t seconds_since_start;

	/**
	 * ASCII representation of the time to be shown the user.
	 * This differs from seconds_since_start since the countdown time
	 * is displayed as a negative number, so seconds_since_start
	 * and time are constantly STRENGTH_COUNTDOWN_SECS apart.
	 * The biased representation helps reach more than 128 seconds
	 * with an 8-bit counter.
	 */
	uint8_t time[4];
} strength_data_t;

extern strength_data_t strength_data;

static inline uint8_t is_strength()
{
	return strength_data.flags.running ;
}

void strength_tick();

void display_strength_time(uint8_t line, uint8_t update);

void strength_sx(uint8_t);

uint8_t strength_display_needs_updating(void);

#endif
