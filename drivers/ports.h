/*
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


	  Redistribution and use in source and binary forms, with or without
	  modification, are permitted provided that the following conditions
	  are met:

	    Redistributions of source code must retain the above copyright
	    notice, this list of conditions and the following disclaimer.

	    Redistributions in binary form must reproduce the above copyright
	    notice, this list of conditions and the following disclaimer in the
	    documentation and/or other materials provided with the
	    distribution.

	    Neither the name of Texas Instruments Incorporated nor the names of
	    its contributors may be used to endorse or promote products derived
	    from this software without specific prior written permission.

	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PORTS_H__
#define __PORTS_H__

union ports_buttons {
	struct {
		/* Manual button events */
		uint8_t star:1;			/* Short STAR button press */
		uint8_t num:1;			/* Short NUM button press */
		uint8_t up:1;				/* Short UP button press */
		uint8_t down:1;			/* Short DOWN button press */
		uint8_t backlight:1;	/* Short BACKLIGHT button press */
		uint8_t star_long:1;	/* Long STAR button press */
		uint8_t num_long:1;		/* Long NUM button press */
	} flag;
	uint8_t all_flags; /* Shortcut to all display flags (for reset) */
};

/* exclusive for ezchronos.c, modules should NOT use this directly */
volatile union ports_buttons ports_buttons;

void init_buttons(void);

#endif /* __PORTS_H__ */
