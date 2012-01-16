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
// Date functions.
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
#include "project.h"

// driver
#include "display.h"
#include "ports.h"

// logic
#include "date.h"
#include "user.h"
#include "clock.h"

#ifdef CONFIG_SIDEREAL
#include "sidereal.h"
#endif

// *************************************************************************************************
// Prototypes section
void reset_date(void);
void mx_date(line_t line);
void sx_date(line_t line);
void display_date(line_t line, update_t update);


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section
struct date sDate;

// *************************************************************************************************
// Extern section


// *************************************************************************************************
// @fn          reset_date
// @brief       Reset date to start value.
// @param       none
// @return      none
// *************************************************************************************************
void reset_date(void)
{
	// Show default display
	sDate.view = 0;

	rtca_tevent_fn_register(&date_event);
}

void date_event(rtca_tevent_ev_t ev)
{
	if (ev >= RTCA_EV_DAY) { // Day changed
		// Indicate to display function that new value is available
		sDate.update_display = 1;
	}
}


// *************************************************************************************************
// @fn          mx_date
// @brief       Date set routine.
// @param       line		LINE1, LINE2
// @return      none
// *************************************************************************************************
void mx_date(line_t line)
{
#ifdef CONFIG_USE_SYNC_TOSET_TIME
	return;
#else
	uint8_t day;
	uint8_t dow;
	uint8_t month;
	uint16_t year;

	uint8_t select;
	uint8_t *str;
	int32_t val;
	uint8_t max_days;

	// Clear display
	clear_display_all();

	// Convert global to local variables
	rtca_get_date(&year, &month, &day, &dow);

	// Init value index
	select = 0;

	// Init display
	// LINE1: YYYY (will be drawn by set_value)
	// LINE2: MM  DD

	str = _itoa(day, 2, 1);
	display_chars(LCD_SEG_L2_1_0, str, SEG_ON);

	str = _itoa(month, 2, 1);
	display_chars(LCD_SEG_L2_5_4, str, SEG_ON);

	// Loop values until all are set or user breaks	set
	while (1) {
		// Idle timeout: exit without saving
		if (sys.flag.idle_timeout) break;

		// Button STAR (short): save, then exit
		if (button.flag.star) {
			// Copy local variables to global variables
			rtca_set_date(year, month, day);
#ifdef CONFIG_SIDEREAL

			if (sSidereal_time.sync > 0)
				sync_sidereal();

#endif

			// Full display update is done when returning from function
			break;
		}

		switch (select) {
		case 0:		// Set year
			val = year;
			set_value(&val, 4, 0, 2008, 2100, SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_3_0, display_value1);
			year = val;
			select = 1;
			break;

		case 1:		// Set month
			val = month;
			set_value(&val, 2, 1, 1, 12, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L2_5_4, display_value1);
			month = val;
			select = 2;
			break;

		case 2:		// Set day
			val = day;
			set_value(&val, 2, 1, 1, 31, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L2_1_0, display_value1);
			day = val;
			select = 0;
			break;
		}

		// Check if day is still valid, if not clamp to last day of current month
		max_days = rtca_get_max_days(month, year);

		if (day > max_days) day = max_days;
	}

	// Clear button flag
	button.all_flags = 0;
#endif
}


// *************************************************************************************************
// @fn          sx_date
// @brief       Date user routine. Toggles view between DD.MM and YYYY.
// @param       line		LINE1, LINE2
// @return      none
// *************************************************************************************************
void sx_date(line_t line)
{
	// Rotate through 4 views
	if (++sDate.view >= 4) sDate.view = 0;

	if (sDate.view == 3) sTime.line2ViewStyle = DISPLAY_DEFAULT_VIEW;
}


// *************************************************************************************************
// @fn          display_date
// @brief       Display date in DD.MM format (metric units) or MM.DD (English units).
// @param       line_t line			LINE1, LINE2
//				update_t update		DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_UPDATE_PARTIAL
// @return      none
// *************************************************************************************************
void display_date(line_t line, update_t update)
{
#ifdef CONFIG_DAY_OF_WEEK
	const uint8_t weekDayStr[7][3] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
#endif
	/* nothing to clear */
	if (update == DISPLAY_LINE_CLEAR)
		return;

	/* if in partial update mode, only update if needed or if in view==3 */
	if (update == DISPLAY_LINE_UPDATE_PARTIAL
			&& ((!sDate.update_display) && (sDate.view != 3)))
		return;

	sDate.update_display = 0;

	uint8_t *str;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t dow;

	rtca_get_date(&year, &month, &day, &dow);

	switch (sDate.view) {
	case 0: /* WWW.DD */
#ifdef CONFIG_DAY_OF_WEEK
		str = _itoa(day, 2, 1);
		display_chars(switch_seg(line, LCD_SEG_L1_1_0, LCD_SEG_L2_1_0),
								str, SEG_ON);
		/* TODO:Get time from RTC */
		str = (uint8_t *)weekDayStr[dow];
		display_chars(switch_seg(line, LCD_SEG_L1_3_2, LCD_SEG_L2_4_2),
								str, SEG_ON);

		display_symbol(switch_seg(line, LCD_SEG_L1_DP1, LCD_SEG_L2_DP),
								SEG_ON);
		break;
#endif

	case 1: /* MM  DD */
		display_symbol(switch_seg(line, LCD_SEG_L1_DP1, LCD_SEG_L2_DP),
								SEG_ON);
#ifndef CONFIG_METRIC_ONLY

		if (!sys.flag.use_metric_units) {
			str = _itoa(day, 2, 0);
			display_chars(switch_seg(line, LCD_SEG_L1_1_0,
						LCD_SEG_L2_1_0), str, SEG_ON);

			str = _itoa(month, 2, 1);
			display_chars(switch_seg(line, LCD_SEG_L1_3_2,
						LCD_SEG_L2_3_2), str, SEG_ON);
		} else
#endif
		{
			str = _itoa(day, 2, 0);
			display_chars(switch_seg(line, LCD_SEG_L1_3_2,
						LCD_SEG_L2_3_2), str, SEG_ON);

			str = _itoa(month, 2, 0);
			display_chars(switch_seg(line, LCD_SEG_L1_1_0,
						LCD_SEG_L2_1_0), str, SEG_ON);
		}

		break;

	case 2: /* YYYY */
		str = _itoa(year, 4, 0);
		display_chars(switch_seg(line, LCD_SEG_L1_3_0, LCD_SEG_L2_3_0),
								str, SEG_ON);
		break;

	default: /* TIME */
		/* TODO: this doesnt belong here and should be moved
		   to clock.c */
		display_time(line, update);
	}
}
