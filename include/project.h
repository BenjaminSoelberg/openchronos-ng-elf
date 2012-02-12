/*
    Copyright (C) 2011 Angelo Arrifano <miknix@gmail.com>
	   - Improve message display API, with timeout feature

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

#ifndef PROJECT_H_
#define PROJECT_H_

// *************************************************************************************************
// Include section
#include <msp430.h>
#include <stdint.h>

#include <config.h>

// *************************************************************************************************
// Defines section

// moved to config.h
// Comment this to not use the LCD charge pump
//#define USE_LCD_CHARGE_PUMP

// Comment this define to build the application without watchdog support
//#define USE_WATCHDOG

// end of move


// Use/not use filter when measuring physical values
#define FILTER_OFF						(0u)
#define FILTER_ON						(1u)


// *************************************************************************************************
// Macro section

// Conversion from usec to ACLK timer ticks
#define CONV_US_TO_TICKS(usec)				(((usec) * 32768) / 1000000)

// Conversion from msec to ACLK timer ticks
#define CONV_MS_TO_TICKS(msec)				(((msec) * 32768) / 1000)


// *************************************************************************************************
// Typedef section

typedef uint8_t line_t;
typedef uint8_t update_t;

typedef enum {
	MENU_ITEM_NOT_VISIBLE = 0,	// Menu item is not visible
	MENU_ITEM_VISIBLE		// Menu item is visible
} menu_t;


// Set of system flags
typedef union {
	struct {
		uint16_t idle_timeout		: 1;    // Timeout after inactivity
		uint16_t idle_timeout_enabled   : 1;    // When in set mode, timeout after a given period
		uint16_t mask_buzzer		: 1;	// Do not output buzz for next button event
		uint16_t up_down_repeat_enabled : 1;    // While in set_value(), create virtual UP/DOWN button events
		uint16_t low_battery		: 1;    // 1 = Battery is low
		uint16_t use_metric_units	: 1;    // 1 = Use metric units, 0 = use English units
		uint16_t am_pm_time		: 1;    // 1 = Display times as AM/PM else 24Hr
		uint16_t delay_over		: 1;    // 1 = Timer delay over
	} flag;
	uint16_t all_flags;            // Shortcut to all display flags (for reset)
} s_system_flags;
extern volatile s_system_flags sys;


// Set of request flags
typedef union {
	struct {
		uint16_t temperature_measurement	: 1;    // 1 = Measure temperature
		uint16_t voltage_measurement		: 1;    // 1 = Measure voltage
		uint16_t altitude_measurement		: 1;    // 1 = Measure air pressure
#ifdef CONFIG_ALTI_ACCUMULATOR
		uint16_t altitude_accumulator           : 1;	// 1 = Measure altitude & accumulate it
#endif
		uint16_t acceleration_measurement	: 1;	// 1 = Measure acceleration
		uint16_t alarm_buzzer			: 1;	// 1 = Output buzzer for alarm
#ifdef CONFIG_EGGTIMER
		uint16_t eggtimer_buzzer		: 1; // 1 = Output buzzer for eggtimer
#endif
#ifdef CONFIG_STRENGTH
		uint16_t strength_buzzer		: 1;    // 1 = Output buzzer from strength_data
#endif
	} flag;
	uint16_t all_flags;            // Shortcut to all display flags (for reset)
} s_request_flags;
extern volatile s_request_flags request;

// *************************************************************************************************
// Global Variable section


// feature dependency calculations

#if defined( CONFIG_PHASE_CLOCK ) || defined( CONFIG_ACCEL) || defined (CONFIG_USE_GPS)
#define FEATURE_PROVIDE_ACCEL
#endif

#if defined (CONFIG_USEPPT) || defined (CONFIG_PHASE_CLOCK) || defined(CONFIG_ACCEL)
#define SIMPLICITI_TX_ONLY_REQ
#endif

#if defined (CONFIG_ALTITUDE) || defined (CONFIG_VARIO) || defined (CONFIG_ALTI_ACCUMULATOR)
#define FEATURE_ALTITUDE
#endif

#if defined (CONFIG_USEPPT) || defined (CONFIG_EGGTIMER) || defined(CONFIG_ACCEL) || defined(CONFIG_USE_GPS)
#define SIMPLICITI_TX_ONLY_REQ
#endif

#if defined(CONFIG_INFOMEM) &&  !defined(CONFIG_SIDEREAL)
//undefine feature if it is not used by any option
#undef CONFIG_INFOMEM
#endif

#endif /*PROJECT_H_*/
