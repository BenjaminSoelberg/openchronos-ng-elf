/*
 * gps.c
 *
 *  Created on: Oct 9, 2010
 *      Author: gabriel
 */

// *************************************************************************************************
// GPS functions.
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
#include "project.h"

#ifdef CONFIG_USE_GPS

#include <string.h>

// driver
#include "ports.h"
#include "display.h"
#include "timer.h"

// logic
#include "menu.h"
#include "clock.h"
#include "user.h"
#include "gps.h"
#include "sequence.h"

#ifdef CONFIG_SIDEREAL
#include "sidereal.h"
#endif

#include "date.h"


#include "rfsimpliciti.h"
#include "simpliciti.h"


void sx_gps(uint8_t line);
void mx_gps(uint8_t line);
void display_gps(uint8_t line, uint8_t update);


void doorlock_signal_success();
void doorlock_signal_failure();
void doorlock_signal_timeout();
void doorlock_signal_invalid();

uint8_t verify_code();

uint8_t sequence_saved[DOORLOCK_SEQUENCE_MAX_LENGTH] = {0};
uint8_t sequence[DOORLOCK_SEQUENCE_MAX_LENGTH] = {0};

// *************************************************************************************************
// @fn          sx_gps
// @brief       Direct GPS function
// @param       line		LINE1
// @return      none
// *************************************************************************************************
void sx_gps(uint8_t line)
{

	uint8_t sequence_again[DOORLOCK_SEQUENCE_MAX_LENGTH] = {0};
	uint8_t error = DOORLOCK_ERROR_SUCCESS;
	uint8_t i = 0;
	uint16_t avg = 0;

	// Enable idle timeout
	sys.flag.idle_timeout_enabled = 1;

	while (1) {
		// Idle timeout ---------------------------------------------------------------------
		if (sys.flag.idle_timeout) {
			// Clear timeout flag
			sys.flag.idle_timeout = 0;

			// Clear display
			clear_display();
			break;
			// Set display update flags
			//display.flag.full_update = 1;

		}

		if (button.flag.num) {
			break;
		}

		if (button.flag.down) {
			// Clear display
			clear_display_all();

			display_chars(LCD_SEG_L1_3_0, (uint8_t *)"CODE", SEG_ON);
			display_chars(LCD_SEG_L2_4_0, (uint8_t *)"PLEAS", SEG_ON);

			error = verify_code();

			if (error == DOORLOCK_ERROR_SUCCESS) {
				if (sys.flag.low_battery) break;

				// display_sync(LINE2, DISPLAY_LINE_UPDATE_FULL);
				clear_display_all();
				display_chars(LCD_SEG_L1_3_0, (uint8_t *)"LINK", SEG_ON_BLINK_ON);
				//display_chars(LCD_SEG_L2_4_0, (uint8_t*)"   ", SEG_ON);
				start_simpliciti_sync();
				display_chars(LCD_SEG_L1_3_0, (uint8_t *)"LINK", SEG_ON_BLINK_OFF);

				if (simpliciti_flag == SIMPLICITI_STATUS_ERROR);

				{
					display_chars(LCD_SEG_L1_3_0, (uint8_t *)"OUT ", SEG_ON);
					display_chars(LCD_SEG_L2_4_0, (uint8_t *)"RANGE", SEG_ON);
					doorlock_signal_timeout();
				}
			}

			break;
		}

		idle_loop();
	}

	// Clear timeout flag
	sys.flag.idle_timeout = 0;
	// Clear button flags
	button.all_flags = 0;
	// Clear display
	clear_display();
	// Force full display update
	display.flag.full_update = 1;
}


// *************************************************************************************************
// @fn          mx_gps
// @brief       Submenu GPS Function
// @param       uint8_t line		LINE1, LINE2
// @return      none
// *************************************************************************************************
void mx_gps(uint8_t line)
{
	uint8_t error = DOORLOCK_ERROR_SUCCESS;

	if (sequence_saved[0] != 0) {
		// Clear display
		clear_display_all();

		display_chars(LCD_SEG_L1_3_0, (uint8_t *)" OLD", SEG_ON);
		display_chars(LCD_SEG_L2_4_0, (uint8_t *)"CODE", SEG_ON);

		error = verify_code();

		if (error != DOORLOCK_ERROR_SUCCESS) return;
	}

	// Clear display
	clear_display_all();

	display_chars(LCD_SEG_L1_3_0, (uint8_t *)" NEW", SEG_ON);
	display_chars(LCD_SEG_L2_4_0, (uint8_t *)"CODE", SEG_ON);
	sequence_saved[0] = 0;
	error = verify_code();

	if (error == DOORLOCK_ERROR_SUCCESS) {
		memcpy(sequence_saved, sequence, DOORLOCK_SEQUENCE_MAX_LENGTH);


		display_chars(LCD_SEG_L1_3_0, (uint8_t *)"CODE", SEG_ON);
		display_chars(LCD_SEG_L2_4_0, (uint8_t *)"AGAIN", SEG_ON);
		error = verify_code();

		if (error == DOORLOCK_ERROR_SUCCESS) memcpy(sequence_saved, sequence, DOORLOCK_SEQUENCE_MAX_LENGTH);
		else {
			display_chars(LCD_SEG_L1_3_0, (uint8_t *)"CODE", SEG_ON);
			display_chars(LCD_SEG_L2_4_0, (uint8_t *)"FAIL", SEG_ON);
			doorlock_signal_failure();
			sequence_saved[0] = 0;
		}
	}

}

// *************************************************************************************************
// @fn          display_gps
// @brief       Display GPS function
// @param       uint8_t line			LINE1
//				uint8_t update		DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_UPDATE_PARTIAL
// @return      none
// *************************************************************************************************
void display_gps(uint8_t line, uint8_t update)
{
	if (update == DISPLAY_LINE_UPDATE_FULL) {
		display_chars(LCD_SEG_L2_5_0, (uint8_t *)"   GPS", SEG_ON);
	}
}



// *************************************************************************************************
// Simple user notification via buzzer
// *************************************************************************************************

// *************************************************************************************************
// @fn          doorlock_signal_success
// @brief       simple two beeps means success
// @param       none
// @return      none
// *************************************************************************************************
void doorlock_signal_success()
{
	start_buzzer(2, CONV_MS_TO_TICKS(100), CONV_MS_TO_TICKS(50));
	Timer0_A4_Delay(CONV_MS_TO_TICKS(300));
	stop_buzzer();
}

// *************************************************************************************************
// @fn          doorlock_signal_failure
// @brief       3 beeps means failure
// @param       none
// @return      none
// *************************************************************************************************
void doorlock_signal_failure()
{
	start_buzzer(3, CONV_MS_TO_TICKS(100), CONV_MS_TO_TICKS(50));
	//Timer0_A4_Delay(CONV_MS_TO_TICKS(450));
	Timer0_A4_Delay(CONV_MS_TO_TICKS(700));
	stop_buzzer();
}

// *************************************************************************************************
// @fn          doorlock_signal_timeout
// @brief       4 beeps means timeout
// @param       none
// @return      none
// *************************************************************************************************
void doorlock_signal_timeout()
{
	start_buzzer(4, CONV_MS_TO_TICKS(100), CONV_MS_TO_TICKS(50));
	Timer0_A4_Delay(CONV_MS_TO_TICKS(600));
	stop_buzzer();
}

// *************************************************************************************************
// @fn          doorlock_signal_invalid
// @brief       5 beeps means something terrible has happened, e.g. someone is trying to hack us
// @param       none
// @return      none
// *************************************************************************************************
void doorlock_signal_invalid()
{
	start_buzzer(1, CONV_MS_TO_TICKS(1000), CONV_MS_TO_TICKS(10));
	Timer0_A4_Delay(CONV_MS_TO_TICKS(1010));
	stop_buzzer();
}

uint8_t verify_code()
{
	uint8_t error = DOORLOCK_ERROR_FAILURE;


	error = doorlock_sequence(sequence);


	if (error == DOORLOCK_ERROR_SUCCESS) {
		//display_chars(LCD_SEG_L1_3_0, (uint8_t*)"CODE", SEG_ON);
		//display_chars(LCD_SEG_L2_4_0, (uint8_t*)"  OK", SEG_ON);
		//doorlock_signal_success();

		if (sequence_saved[0] != 0) {
			error = sequence_compare(sequence_saved, sequence);
		} else {
			error = DOORLOCK_ERROR_SUCCESS;
		}

		if (error == DOORLOCK_ERROR_SUCCESS) {
			display_chars(LCD_SEG_L1_3_0, (uint8_t *)"CODE", SEG_ON);
			display_chars(LCD_SEG_L2_4_0, (uint8_t *)"CHECK", SEG_ON);
			doorlock_signal_success();
			return DOORLOCK_ERROR_SUCCESS;
		} else {
			display_chars(LCD_SEG_L1_3_0, (uint8_t *)"MIS-", SEG_ON);
			display_chars(LCD_SEG_L2_4_0, (uint8_t *)"MATCH", SEG_ON);
			doorlock_signal_failure();
		}

		// memcpy(sequence_saved,sequence,DOORLOCK_SEQUENCE_MAX_LENGTH);
	} else {
		display_chars(LCD_SEG_L1_3_0, (uint8_t *)"CODE", SEG_ON);
		display_chars(LCD_SEG_L2_4_0, (uint8_t *)"FAIL", SEG_ON);
		doorlock_signal_failure();
	}

	return DOORLOCK_ERROR_FAILURE;

}
#endif // CONFIG_USE_GPS
