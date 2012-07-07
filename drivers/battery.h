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

#ifndef BATTERY_H_
#define BATTERY_H_

#include <openchronos.h>

// *************************************************************************************************
// Include section

// *************************************************************************************************
// Prototypes section

// Internal functions
extern void reset_batt_measurement(void);
extern void battery_measurement(void);

// Menu functions
extern void display_battery_V();
extern void clear_battery_V();

// *************************************************************************************************
// Defines section

// Battery high voltage threshold
#define BATTERY_HIGH_THRESHOLD			(360u)

// Battery end of life voltage threshold -> disable radio, show "lobatt" message
#define BATTERY_LOW_THRESHOLD			(240u)

// Where we consider the battery full
#define BATTERY_FULL_THRESHOLD			(300u)

// Where we consider the battery empty
#define BATTERY_EMPTY_THRESHOLD			(220u)

#ifndef FALSE
  // the classic false
  #define FALSE (0 == 1)
#endif

#ifndef TRUE
  // the classic true
  #define TRUE  (1 == 1)
#endif

// *************************************************************************************************
// Global Variable section
/* TODO: pack this stuff!!!! */
struct batt {
	// Currently flagged as low? TRUE of FALSE - defined below
	uint8_t low_battery :1;

	// Update available for sys_msg?
	uint8_t has_update :1;

	// Battery voltage
	uint16_t voltage;

	// Battery voltage offset
	int16_t offset;
};
extern struct batt sBatt;


// *************************************************************************************************
// Extern section

#endif /*BATTERY_H_*/
