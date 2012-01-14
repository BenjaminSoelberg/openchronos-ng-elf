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

#ifndef SIDEREALTIME_H_
#define SIDEREALTIME_H_

// *************************************************************************************************
// Defines section

// *************************************************************************************************
// Prototypes section
extern void sync_sidereal(void);
extern void reset_sidereal_clock(void);
extern void sx_sidereal(uint8_t line);
extern void mx_sidereal(uint8_t line);
extern void sidereal_clock_tick(void);
extern void display_sidereal(uint8_t line, uint8_t update);

// *************************************************************************************************
// Global Variable section

//count of different longitudes that can be stored and selected from
#define SIDEREAL_NUM_LON 3

//longitude >0:east, <0:west (the parts are not allowed to differ in sign)
struct longitude {
	int16_t		deg;
	int8_t		min;
	int8_t		sec;
};


struct sidereal_time {
	// Flag to minimize display updates
	uint8_t 		drawFlag;

	// Viewing style
	uint8_t		line1ViewStyle;
	uint8_t		line2ViewStyle;

	// Time data
	uint8_t		hour;
	uint8_t		minute;
	uint8_t 		second;

	//SIDEREAL_NUM_LON different longitudes
	struct longitude lon[SIDEREAL_NUM_LON];
	//selected longitude to use for time calculation
	uint8_t		lon_selection;

	//synchronize to normal time automatically
	uint8_t		sync;
};
extern struct sidereal_time sSidereal_time;

#define SIDEREAL_INFOMEM_ID 0x10

#endif /*SIDEREALTIME_H_*/
