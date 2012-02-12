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
#include "user.h"

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

// Display values for time format selection
#if (OPTION_TIME_DISPLAY == CLOCK_DISPLAY_SELECT)
const uint8_t selection_Timeformat[][4] = {
	"24H", "12H"
};
#endif


void clock_event(rtca_tevent_ev_t ev)
{
    /* Exit if we are not active! */
    if (!sTime.active)
    	return;

    uint8_t hour, min, sec;
    rtca_get_time(&hour, &min, &sec);

    switch(ev) {
	case RTCA_EV_HOUR:
	    display_chars(LCD_SEG_L1_3_2, _itoa(hour, 2, 0), SEG_ON);
	case RTCA_EV_MINUTE:
	    display_chars(LCD_SEG_L1_1_0, _itoa(min, 2, 0), SEG_ON);
	default: /* Only seconds are changes */
	    display_chars(LCD_SEG_L2_1_0, _itoa(sec, 2, 0), SEG_ON);
    }
}


// *************************************************************************************************
// @fn          convert_hour_to_12H_format
// @brief       Convert internal 24H time to 12H time.
// @param       uint8_t hour		Hour in 24H format
// @return      uint8_t				Hour in 12H format
// *************************************************************************************************
#if (OPTION_TIME_DISPLAY > CLOCK_24HR)
static uint8_t convert_hour_to_12H_format(uint8_t hour)
{
	// 00:00 .. 11:59 --> AM 12:00 .. 11:59
	if (hour == 0)
		return (hour + 12);
	else if (hour <= 12)
		return (hour);
	// 13:00 .. 23:59 --> PM 01:00 .. 11:59
	else
		return (hour - 12);
}


// *************************************************************************************************
// @fn          is_hour_am
// @brief       Checks if internal 24H time is AM or PM
// @param       uint8_t hour		Hour in 24H format
// @return      uint8_t				1 = AM, 0 = PM
// *************************************************************************************************
static uint8_t is_hour_am(uint8_t hour)
{
	// 00:00 .. 11:59 --> AM 12:00 .. 11:59
	if (hour < 12) return (1);
	// 12:00 .. 23:59 --> PM 12:00 .. 11:59
	else return (0);
}

#if (OPTION_TIME_DISPLAY == CLOCK_DISPLAY_SELECT)
// *************************************************************************************************
// @fn          display_selection_Timeformat
// @brief       Display time format 12H / 24H.
// @param       uint8_t segments			Target segments where to display information
//				uint32_t index			0 or 1, index for value string
//				uint8_t digits			Not used
//				uint8_t blanks			Not used
// @return      none
// *************************************************************************************************
static void display_selection_Timeformat1(uint8_t segments, uint32_t index, uint8_t digits, uint8_t blanks, uint8_t dummy)
{
	if (index < 2) display_chars(segments, (uint8_t *)selection_Timeformat[index], SEG_ON_BLINK_ON);
}
#endif // CLOCK_DISPLAY_SELECT

#endif //OPTION_TIME_DISPLAY


static void increment_value()
{
    switch(sTime.edit_state){
	case edit_state_seconds:
	    helpers_loop_up( &sTime.tmp_sec , 0, 60);
	    display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_sec, 2, 0), SEG_ON_BLINK_ON);
	    break;
	case edit_state_minutes:
	    helpers_loop_up( &sTime.tmp_min , 0, 60);
	    display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_min, 2, 0), SEG_ON_BLINK_ON);
	    break;
	case edit_state_hours:
	    helpers_loop_up( &sTime.tmp_hour, 0, 24); /* TODO: fix for 12/24 hr! */
	    display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hour, 2, 0), SEG_ON_BLINK_ON);
	    break;
    }
}

static void decrement_value()
{
    switch(sTime.edit_state){
	case edit_state_seconds:
	    helpers_loop_down( &sTime.tmp_sec , 0, 60);
	    display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_sec, 2, 0), SEG_ON_BLINK_ON);
	    break;
	case edit_state_minutes:
	    helpers_loop_down( &sTime.tmp_min , 0, 60);
	    display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_min, 2, 0), SEG_ON_BLINK_ON);
	    break;
	case edit_state_hours:
	    helpers_loop_down( &sTime.tmp_hour, 0, 24); /* TODO: fix for 12/24 hr! */
	    display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hour, 2, 0), SEG_ON_BLINK_ON);
	    break;
    }
}

static void edit_next_value(){
    helpers_loop_up(&sTime.edit_state, 0, 3);
}

/* Star button long press callback. */
static void star_long_pressed()
{
    /* We go into edit mode  */
    sTime.edit_state = edit_state_hours;

    /* Save the current time in edit_buffer */
    rtca_get_time(&sTime.tmp_hour, &sTime.tmp_min, &sTime.tmp_sec);

    menu_edit_mode_start(&edit_next_value, &increment_value, &decrement_value);
    
    /* Here we return from the edit mode, fill in the new values! */
    rtca_set_time(sTime.tmp_hour, sTime.tmp_min, sTime.tmp_sec);
    
}

void clock_init()
{
    /* The module is not enabled by default. */
    sTime.active = 0;

#ifdef CONFIG_SIDEREAL
    sTime.UTCoffset  = 0;
#endif

    rtca_tevent_fn_register(&clock_event);
}
