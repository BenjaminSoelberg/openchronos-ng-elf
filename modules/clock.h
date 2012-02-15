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

#ifndef CLOCKTIMER_H_
#define CLOCKTIMER_H_

#include "rtca.h"

// *************************************************************************************************
// Defines section

// Definitions for time format
#define TIMEFORMAT_24H					(0u)
#define TIMEFORMAT_12H					(1u)

/* TODO: display_time should be private scope! */
extern void clock_event(rtca_tevent_ev_t ev);



// *************************************************************************************************
// Global Variable section
/* TODO: pack this stuff!!!! */
struct time
{
	enum {
       EDIT_STATE_OFF		= 0,
	    EDIT_STATE_HH			= 1,
	    EDIT_STATE_MM			= 2,
	} edit_state;

	/* Temporary variables for editing */
	uint8_t tmp_hour;
	uint8_t tmp_min;
	uint8_t tmp_sec;

#ifdef CONFIG_SIDEREAL
	// offset of local time from UTC (=1: set time is UTC+1 =CET)
	int8_t		UTCoffset;
#endif
};
extern struct time sTime;


#endif /*CLOCKTIMER_H_*/
