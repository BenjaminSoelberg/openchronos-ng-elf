/*
 modules/stopwatch.c

 Copyright (C) 2012   Ricardo Martin Marcucci
 Copyright (C) 2012   Robert Hoeller <rhh.privat@googlemail.com>

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
/*
 * cronometer.c
 *
 *  Created on: Sep 16, 2012
 *      Author: Ricardo Martin Marcucci
 *
 *      This module is intended as a "hello world" application for me
 *      to get used to the openchronos-ng framework..
 *
 *      Use:
 *      You can select the LAP-mode from the function-menu
 *
 *      First line shows "LAP" (in this version) and the second line
 *      shows the lapcounter value.
 *      ^ (up-arrow) increments the lapcounter
 *      v (dn-arrow) decrements the lapcounter
 *      # (num-longpressed) resets the lapcounter
 *      # (num-shortpressed) --- noop
 *      * (star-Longpressed) --- noop
 *		- (both-arrows) --- noop
 */

#include <openchronos.h>
/* driver */
#include <drivers/display.h>

/* Defines */

#define SWATCH_MODE_OFF			(0u)
#define SWATCH_MODE_ON			(1u)
#define SWATCH_MODE_BACKGROUND	(2u)
#define MAX_LAPS	10

/*
 * A structure to save diferent times
 */
struct swatch_time {
	// ACC_MODE_OFF, ACC_MODE_ON, ACCEL_MODE_BACKGROUND
	uint8_t hours;
	// Sensor raw data
	uint8_t minutes;
	// Sensor raw data
	uint8_t seconds;

	uint8_t cents;
};

/*
 * A structure with the swatch configuration
 */
struct swatch_conf {
	uint8_t state;
	uint8_t laps;
	uint8_t lap_act;
};
//Add two extra for current time and display time

#define SW_DISPLAYNG	MAX_LAPS +1
#define SW_COUNTING		MAX_LAPS

extern struct swatch_time sSwatch_time[MAX_LAPS + 2];
struct swatch_time sSwatch_time[MAX_LAPS + 2];

extern struct swatch_conf sSwatch_conf;
struct swatch_conf sSwatch_conf;

/*
 * Helper Functions
 */
void clear_stopwatch(void);
void increment_lap_stopwatch(void);

/* Function to write the screen */

void drawStopWatchScreen(void) {

	if (sSwatch_conf.state != SWATCH_MODE_BACKGROUND) {
		sSwatch_time[SW_DISPLAYNG] = sSwatch_time[sSwatch_conf.lap_act];
		if (SW_COUNTING == sSwatch_conf.lap_act) {
			if (sSwatch_conf.state == SWATCH_MODE_OFF) {
				display_chars(0, LCD_SEG_L1_3_0, "STOP", SEG_SET);
			} else {
				display_chars(0, LCD_SEG_L1_3_2, "LP", SEG_SET);
				_printf(0, LCD_SEG_L1_1_0, "%2u", sSwatch_conf.laps);
			}

		} else {
			display_chars(0, LCD_SEG_L1_3_2, "LP", SEG_SET);
			_printf(0, LCD_SEG_L1_1_0, "%2u", sSwatch_conf.lap_act +1);
		}
		if (sSwatch_time[SW_DISPLAYNG].minutes < 20
				&& sSwatch_time[SW_DISPLAYNG].hours == 0) {
			_printf(0, LCD_SEG_L2_5_4, "%02u",
					sSwatch_time[SW_DISPLAYNG].minutes);
			_printf(0, LCD_SEG_L2_3_2, "%02u",
					sSwatch_time[SW_DISPLAYNG].seconds);
			_printf(0, LCD_SEG_L2_1_0, "%02u",
					sSwatch_time[SW_DISPLAYNG].cents);
		} else {
			_printf(0, LCD_SEG_L2_5_4, "%02u",
					sSwatch_time[SW_DISPLAYNG].hours);
			_printf(0, LCD_SEG_L2_3_2, "%02u",
					sSwatch_time[SW_DISPLAYNG].minutes);
			_printf(0, LCD_SEG_L2_1_0, "%02u",
					sSwatch_time[SW_DISPLAYNG].seconds);
		}
	}
	if (sSwatch_conf.state != SWATCH_MODE_OFF) {
		if (sSwatch_time[SW_COUNTING].cents == 5) {
			display_symbol(0, LCD_ICON_STOPWATCH, SEG_ON);
		} else if (sSwatch_time[SW_COUNTING].cents == 55) {
			display_symbol(0, LCD_ICON_STOPWATCH, SEG_OFF);
		}
	}
}

/* Function called every 5ms to increment the counters */
static void stopwatch_event() {

	if (sSwatch_conf.state != SWATCH_MODE_OFF) {
		sSwatch_time[SW_COUNTING].cents += 5;
		if (sSwatch_time[SW_COUNTING].cents >= 100) {
			sSwatch_time[SW_COUNTING].cents = 0;
			sSwatch_time[SW_COUNTING].seconds++;
			if (sSwatch_time[SW_COUNTING].seconds >= 60) {
				sSwatch_time[SW_COUNTING].seconds = 0;
				sSwatch_time[SW_COUNTING].minutes++;
				if (sSwatch_time[SW_COUNTING].minutes >= 60) {
					sSwatch_time[SW_COUNTING].minutes = 0;
					sSwatch_time[SW_COUNTING].hours++;
					if (sSwatch_time[SW_COUNTING].hours >= 20) {
						sSwatch_time[SW_COUNTING].hours = 0;
					}
				}
			}
		}
		drawStopWatchScreen();
	}
}

/* Activation of the module */
static void stopwatch_activated() {
	display_symbol(0, LCD_SEG_L2_COL0, SEG_ON);
	display_symbol(0, LCD_SEG_L2_COL1, SEG_ON);
	if (sSwatch_conf.state == SWATCH_MODE_BACKGROUND) {
		sSwatch_conf.state = SWATCH_MODE_ON;
		return;
	}

	sys_messagebus_register(&stopwatch_event, SYS_MSG_TIMER_20HZ);
	drawStopWatchScreen();
}

/* Deactivation of the module */
static void stopwatch_deactivated() {
	/* clean up screen */
	display_clear(0, 1);
	display_clear(0, 2);
	if (sSwatch_conf.state == SWATCH_MODE_ON) {
		sSwatch_conf.state = SWATCH_MODE_BACKGROUND;
		return;
	} else {

		sys_messagebus_unregister(&stopwatch_event);
		display_symbol(0, LCD_ICON_STOPWATCH, SEG_OFF);
		display_symbol(0, LCD_SEG_L2_COL0, SEG_OFF);
		display_symbol(0, LCD_SEG_L2_COL1, SEG_OFF);
	}
}

static void down_press() {
	if (sSwatch_conf.state == SWATCH_MODE_ON) {
		increment_lap_stopwatch();

	} else if (sSwatch_conf.laps != 0) {
		if (sSwatch_conf.lap_act > sSwatch_conf.laps - 1) {
			sSwatch_conf.lap_act = 0;
		} else if (sSwatch_conf.lap_act > 0) {
			sSwatch_conf.lap_act--;
		}
		drawStopWatchScreen();
	}
}
static void up_press() {
	if (sSwatch_conf.state == SWATCH_MODE_ON) {
		increment_lap_stopwatch();
	} else if (sSwatch_conf.laps != 0) {
		if (sSwatch_conf.lap_act < sSwatch_conf.laps - 1) {
			sSwatch_conf.lap_act++;
		} else if (sSwatch_conf.lap_act > sSwatch_conf.laps - 1) {
			sSwatch_conf.lap_act = sSwatch_conf.laps - 1;
		}
		drawStopWatchScreen();
	}
}
static void num_press() {
	if (sSwatch_conf.state == SWATCH_MODE_OFF) {
		sSwatch_conf.state = SWATCH_MODE_ON;
		sSwatch_conf.lap_act = SW_COUNTING;
	} else {
		sSwatch_conf.state = SWATCH_MODE_OFF;
	}
	drawStopWatchScreen();
}

static void num_long_pressed() {

	if (sSwatch_conf.state == SWATCH_MODE_OFF) {
		clear_stopwatch();
		drawStopWatchScreen();
	}
}

void mod_stopwatch_init(void) {
	sSwatch_conf.state = SWATCH_MODE_OFF;
	clear_stopwatch();

	menu_add_entry("ST WH", &up_press, &down_press, &num_press, NULL,
			&num_long_pressed, NULL, &stopwatch_activated,
			&stopwatch_deactivated);
}

/*
 * Helper Functions
 */

void clear_stopwatch(void) {
	sSwatch_time[SW_COUNTING].cents = 0;
	sSwatch_time[SW_COUNTING].hours = 0;
	sSwatch_time[SW_COUNTING].minutes = 0;
	sSwatch_time[SW_COUNTING].seconds = 0;
	sSwatch_conf.laps = 0;
	sSwatch_conf.lap_act = SW_COUNTING;
}

void increment_lap_stopwatch(void) {
	sSwatch_time[sSwatch_conf.laps] = sSwatch_time[SW_COUNTING];
	if (sSwatch_conf.laps < (MAX_LAPS - 1)) {
		sSwatch_conf.laps++;
	}
}
