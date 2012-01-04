/*
    Copyright (C) 2011 Angelo Arrifano <miknix@gmail.com>
	   - Updated to use the improved message display API
	   - Simplified code, allow simultaneous chime and alarm

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


/*****************************************************************************
 ** @fn     alarm_tick
 ** @brief  this function is called every 1 second
 ** @param  none
 ** @return none
 ****************************************************************************/
void alarm_tick()
{
	// Hack to prevent sAlarm.running to be set several times within 1sec,
	// which otherwise would prevent us to stop the alarm noise..
	if (sTime.drawFlag >= 2) {
		// If the chime is enabled, we beep here
		if (sTime.minute == 0) {
			if (sAlarm.chime) {
				request.flag.alarm_buzzer = 1;
			}
		}
		// Check if alarm needs to be turned on
		// Start with minutes - only 1/60 probability to match
		if (sAlarm.alarm && sTime.minute == sAlarm.minute
		                                     && sTime.hour == sAlarm.hour) {
			// Indicate that alarm is beeping
			sAlarm.running = 1;
		}
	}
	// Generate alarm signal
	if (sAlarm.running)
	{
		// Decrement alarm duration counter
		if (sAlarm.duration-- > 0)
		{
			request.flag.alarm_buzzer = 1;
		}
		else
		{
			sAlarm.duration = ALARM_ON_DURATION;
			stop_alarm();
		}
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
	sAlarm.hour   = 6;
	sAlarm.minute = 30;

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
	// Alarm not running anymore
	sAlarm.running = 0;
	
	// Stop buzzer
	stop_buzzer();
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

		// Register timer only if we need it, saving CPU cycles..
		if (sAlarm.state)
			Timer0_A1_Register(&alarm_tick);
		else
			Timer0_A1_Unregister(&alarm_tick);
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
	s32 hours    = sAlarm.hour;
	s32 minutes  = sAlarm.minute;

	// Display HH:MM (LINE1) 
	{
		u8 *str = _itoa(hours, 2, 0);
		display_chars(LCD_SEG_L1_3_2, str, SEG_ON);
		display_symbol(LCD_SEG_L1_COL, SEG_ON);
	}
	{
		u8 *str = _itoa(minutes, 2, 0);
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
	    sAlarm.hour = hours;
	    sAlarm.minute = minutes;
	    // Set display update flag
	    display.flag.line1_full_update = 1;
	    break;
	  }

	  if (select == 0)
	    set_value(&hours, 2, 0, 0, 23, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_3_2, display_hours_12_or_24);
     else
	    set_value(&minutes, 2, 0, 0, 59, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_1_0, display_value1);
	  select = !select;
	}

	// Clear button flag
	button.all_flags = 0;

	// Indicate to display function that new value is available
	display.flag.update_alarm = 1;
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
		display_hours_12_or_24(switch_seg(line, LCD_SEG_L1_3_2, LCD_SEG_L2_3_2), sAlarm.hour, 2, 1, SEG_ON);
		display_chars(switch_seg(line, LCD_SEG_L1_1_0, LCD_SEG_L2_1_0), _itoa(sAlarm.minute, 2, 0), SEG_ON);
		display_symbol(switch_seg(line, LCD_SEG_L1_COL, LCD_SEG_L2_COL0), SEG_ON);
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
