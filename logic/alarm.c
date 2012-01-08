/*
    Copyright (C) 2011-2012 Angelo Arrifano <miknix@gmail.com>
	   - Updated to use the improved message display API
	   - Simplified code, allow simultaneous chime and alarm
	   - Updated to use RTC_A, the realtime clock driver

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
// Alarm routines.
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
#include "project.h"
#ifdef CONFIG_ALARM

// driver
#include "display.h"
#include "buzzer.h"
#include "ports.h"
#include "timer.h"

// logic
#include "alarm.h"
#include "clock.h"
#include "user.h"


// *************************************************************************************************
// Prototypes section


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section
struct alarm sAlarm;


// *************************************************************************************************
// Extern section

void alarm_buzzer(void)
{
	// Decrement alarm duration counter
	if (sAlarm.duration-- > 0)
		request.flag.alarm_buzzer = 1;
	else {
		stop_alarm();
	}
}

void alarm_event(rtca_tevent_ev_t ev)
{
	if (sAlarm.hold && ev == RTCA_EV_MINUTE) {
		// the alarm condition is not true anymore so we can
		// re-enable the alarm again.
		// (the RTCA hardware disables alarm after firing off)
		sAlarm.hold = 0;
		if (sAlarm.alarm)
			rtca_enable_alarm();
	} else if (ev == RTCA_EV_ALARM) {
		request.flag.alarm_buzzer = 1;

		// enable hold mode
		// (this has the side effect of not allowing alarms set
		// for the next minute but no sane person will need that)
		sAlarm.hold = 1;

		// keep the buzzer running for a while using the timer..
		sAlarm.running = 1;
		Timer0_A1_Register(&alarm_buzzer);
	} else if (sAlarm.chime && ev == RTCA_EV_HOUR) {
		// Make a beep if in a hour event
		request.flag.alarm_buzzer = 1;
	}
}

// *************************************************************************************************
// @fn          clearalarmData
// @brief       Resets alarmData to 06:30
// @param       none
// @return      none
// *************************************************************************************************
void reset_alarm(void) 
{
	// Default alarm time 06:30
	rtca_set_alarm(6, 30);

	// Alarm and chime are initially off
	sAlarm.duration = ALARM_ON_DURATION;
	sAlarm.alarm 	 = 0;
	sAlarm.chime 	 = 0;
	sAlarm.running  = 0;
}

// *************************************************************************************************
// @fn          stop_alarm
// @brief       Stop active alarm
// @param       none
// @return      none
// *************************************************************************************************
void stop_alarm(void) 
{
	// Stop buzzer
	stop_buzzer();

	// we dont need the timer anymore
	Timer0_A1_Unregister(&alarm_buzzer);

	// Alarm not running anymore
	sAlarm.duration = ALARM_ON_DURATION;
	sAlarm.running = 0;
}	


// *************************************************************************************************
// @fn          sx_alarm
// @brief       Sx button turns alarm on/off.
// @param       u8 line		LINE1
// @return      none
// *************************************************************************************************
void sx_alarm(u8 line)
{
	// UP: Cycle through alarm modes
	if(button.flag.up)
	{
		// this will cycle between all alarm/chime combinations and overflow
		sAlarm.state++;

		// redraw screen soon as possible
		display.flag.line1_full_update = 1;

		// Register RTC only if needed, saving CPU cycles..
		if (sAlarm.state) {
			rtca_tevent_fn_register(alarm_event);
		} else
			rtca_tevent_fn_unregister(alarm_event);

		if (sAlarm.alarm)
			rtca_enable_alarm();
		else
			rtca_disable_alarm();
	}
}


// *************************************************************************************************
// @fn          mx_alarm
// @brief       Set alarm time.
// @param       u8 line		LINE1
// @return      none
// *************************************************************************************************
void mx_alarm(u8 line)
{
	// Clear display
	clear_display_all();

	// Keep global values in case new values are discarded
	u8 hour;
	u8 min;

	rtca_get_alarm(&hour, &min);

	// Display HH:MM (LINE1) 
	{
		u8 *str = _itoa(hour, 2, 0);
		display_chars(LCD_SEG_L1_3_2, str, SEG_ON);
		display_symbol(LCD_SEG_L1_COL, SEG_ON);
	}
	{
		u8 *str = _itoa(min, 2, 0);
		display_chars(LCD_SEG_L1_1_0, str, SEG_ON);
	}
	
	// Init value index
	u8 select = 0;
		
	// Loop values until all are set or user breaks	set
	while(1) 
	{
	  // Idle timeout: exit without saving
	  if (sys.flag.idle_timeout) break;

	  // STAR (short): save, then exit
	  if (button.flag.star)
	  {
	    // Store local variables in global alarm time
	    rtca_set_alarm(hour, min);
	    // Set display update flag
	    display.flag.line1_full_update = 1;
	    break;
	  }

	  if (select == 0) {
	    s32 _hour = hour;
	    set_value(&_hour, 2, 0, 0, 23, SETVALUE_ROLLOVER_VALUE
	                                 + SETVALUE_DISPLAY_VALUE
					 + SETVALUE_NEXT_VALUE,
					 LCD_SEG_L1_3_2,
					 display_hours_12_or_24);
	    hour = _hour;
	  } else {
	    s32 _min = min;
	    set_value(&_min, 2, 0, 0, 59, SETVALUE_ROLLOVER_VALUE
	                                 + SETVALUE_DISPLAY_VALUE
					 + SETVALUE_NEXT_VALUE,
					 LCD_SEG_L1_1_0, display_value1);
	    min = _min;
	  }
	  select = !select;
	}

	// Clear button flag
	button.all_flags = 0;
}


// *************************************************************************************************
// @fn          display_alarm
// @brief       Display alarm time. 24H / 12H time format.
// @param       u8 line	LINE1, LINE2
//		u8 update	DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_CLEAR
// @return      none
// *************************************************************************************************
void display_alarm(u8 line, u8 update)
{
	if (update == DISPLAY_LINE_UPDATE_FULL) {
		u8 hour, min;

		rtca_get_alarm(&hour, &min);

		display_hours_12_or_24(
		            switch_seg(line, LCD_SEG_L1_3_2, LCD_SEG_L2_3_2),
			    hour, 2, 1, SEG_ON);
		display_chars(
		            switch_seg(line, LCD_SEG_L1_1_0, LCD_SEG_L2_1_0),
			    _itoa(min, 2, 0), SEG_ON);
		display_symbol(
		            switch_seg(line, LCD_SEG_L1_COL, LCD_SEG_L2_COL0),
			    SEG_ON);
		// Clear / set alarm icon
		if (sAlarm.alarm)
			display_symbol(LCD_ICON_ALARM, SEG_ON);
		else
			display_symbol(LCD_ICON_ALARM, SEG_OFF);

		// Clear / set chime icon
		if (sAlarm.chime) {
			display_symbol(LCD_ICON_BEEPER2, SEG_ON);
			display_symbol(LCD_ICON_BEEPER3, SEG_ON);
		} else {
			display_symbol(LCD_ICON_BEEPER2, SEG_OFF);
			display_symbol(LCD_ICON_BEEPER3, SEG_OFF);
		}
	}
	else if (update == DISPLAY_LINE_CLEAR)			
	{
		// Clean up function-specific segments before leaving function
		display_symbol(LCD_SYMB_AM, SEG_OFF);
		display_symbol(LCD_ICON_BEEPER3, SEG_OFF);
		// Dont leave the bell ON, it can be reused by other apps..
		display_symbol(LCD_ICON_ALARM, SEG_OFF);
		display_symbol(LCD_ICON_BEEPER2, SEG_OFF);
		display_symbol(LCD_ICON_BEEPER3, SEG_OFF);
	}
}
#endif /* CONFIG_ALARM */
