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

// system
#include <ezchronos.h>

// driver
#include "ports.h"
#include "display.h"
#include "timer.h"

// logic
#include "clock.h"

#ifdef CONFIG_SIDEREAL
#include "sidereal.h"
#endif

#include "date.h"

#ifdef CONFIG_CLOCK_ONLY_SYNC
#include "rfsimpliciti.h"
#endif


// *************************************************************************************************
// Global Variable section
struct time sTime;

void clock_event(rtca_tevent_ev_t ev)
{
	/* Exit if we are not active! */
	if (sTime.edit_state != EDIT_STATE_OFF)
		return;

	uint8_t hour, min, sec;
	rtca_get_time(&hour, &min, &sec);

	switch (ev) {
	case RTCA_EV_HOUR:
		display_chars(LCD_SEG_L1_3_2, _itoa(hour, 2, 0), SEG_ON);

	case RTCA_EV_MINUTE:
		display_chars(LCD_SEG_L1_1_0, _itoa(min, 2, 0), SEG_ON);

	default: /* Only seconds are changes */
		display_chars(LCD_SEG_L2_1_0, _itoa(sec, 2, 0), SEG_ON);
	}
}

void clock_activated()
{
	rtca_tevent_fn_register(&clock_event);
}

void clock_deactivated()
{
	rtca_tevent_fn_unregister(&clock_event);
}

static void increment_value()
{
	switch (sTime.edit_state) {
	case EDIT_STATE_SS:
		helpers_loop_up(&sTime.tmp_sec , 0, 60);
		display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_sec, 2, 0), SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_MM:
		helpers_loop_up(&sTime.tmp_min , 0, 60);
		display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_min, 2, 0), SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_HH:
		helpers_loop_up(&sTime.tmp_hour, 0, 24);  /* TODO: fix for 12/24 hr! */
		display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hour, 2, 0), SEG_ON_BLINK_ON);
		break;
	default:
		break;
	}
}

static void decrement_value()
{
	switch (sTime.edit_state) {
	case EDIT_STATE_SS:
		helpers_loop_down(&sTime.tmp_sec , 0, 60);
		display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_sec, 2, 0), SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_MM:
		helpers_loop_down(&sTime.tmp_min , 0, 60);
		display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_min, 2, 0), SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_HH:
		helpers_loop_down(&sTime.tmp_hour, 0, 24);  /* TODO: fix for 12/24 hr! */
		display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hour, 2, 0), SEG_ON_BLINK_ON);
		break;
	default:
		break;
	}
}

static void edit_next_value()
{
	helpers_loop_up(&sTime.edit_state, 0, 3);
}

static void save_value()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_time(sTime.tmp_hour, sTime.tmp_min, sTime.tmp_sec);

	/* And stop the blinking! */
	clear_blink_mem();

	sTime.edit_state = EDIT_STATE_OFF;
}


/* Star button long press callback. */
static void star_long_pressed()
{
	/* We go into edit mode  */
	sTime.edit_state = EDIT_STATE_HH;

	/* Save the current time in edit_buffer */
	rtca_get_time(&sTime.tmp_hour, &sTime.tmp_min, &sTime.tmp_sec);

	menu_editmode_start(&increment_value, &decrement_value, &edit_next_value, &save_value);

}

void clock_init()
{
	sTime.edit_state = EDIT_STATE_OFF;

#ifdef CONFIG_SIDEREAL
	sTime.UTCoffset  = 0;
#endif
	menu_add_entry(NULL, NULL, NULL,
		       &star_long_pressed,
		       &clock_activated,
		       &clock_deactivated
		      );

}
