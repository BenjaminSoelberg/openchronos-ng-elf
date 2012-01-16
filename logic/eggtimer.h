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

#ifndef eggtimer_H_
#define eggtimer_H_

// *************************************************************************************************
// Include section
#include <project.h>

// *************************************************************************************************
// Prototypes section
extern void init_eggtimer(void);
extern void start_eggtimer(void);
extern void stop_eggtimer(void);
extern void stop_eggtimer_alarm(void);
extern void set_eggtimer_to_defaults(void);
extern void set_eggtimer(void);
extern void eggtimer_tick(void);
extern void mx_eggtimer(uint8_t line);
extern void sx_eggtimer(uint8_t line);
extern void display_eggtimer(uint8_t line, uint8_t update);
extern uint8_t eggtimer_visible(void);


// *************************************************************************************************
// Defines section
#define EGGTIMER_STOP				(0u)
#define EGGTIMER_RUN				(1u)
#define EGGTIMER_ALARM				(2u)

#define EGGTIMER_ALARM_DURATION			(10u)

// *************************************************************************************************
// Global Variable section
/* TODO: pack this stuff!! */
struct eggtimer
{
	uint8_t	state;
	uint8_t	drawFlag;
	uint8_t update_display;
	
	// Values that are decremented each second
	uint8_t	hours;
	uint8_t	minutes;
	uint8_t	seconds;

	// Values to default to, after timer runs out and is cleared
	uint8_t	default_hours;
	uint8_t	default_minutes;
	uint8_t	default_seconds;

	uint8_t	duration; //Number of times to request buzzer double-beep when time is up
};
extern struct eggtimer sEggtimer;


// *************************************************************************************************
// Extern section

#endif /*eggtimer_H_*/
