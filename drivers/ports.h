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

/* Button ports */
#define PORTS_BTN_DOWN_PIN		(BIT0)
#define PORTS_BTN_NUM_PIN		(BIT1)
#define PORTS_BTN_STAR_PIN		(BIT2)
#define PORTS_BTN_BL_PIN		(BIT3)
#define PORTS_BTN_UP_PIN		(BIT4)

enum ports_buttons {
	PORTS_BTN_DOWN		= PORTS_BTN_DOWN_PIN,
	PORTS_BTN_NUM		= PORTS_BTN_NUM_PIN,
	PORTS_BTN_STAR		= PORTS_BTN_STAR_PIN,
	PORTS_BTN_BL		= PORTS_BTN_BL_PIN,
	PORTS_BTN_UP		= PORTS_BTN_UP_PIN,
	PORTS_BTN_LDOWN	= BIT5,
	PORTS_BTN_LNUM		= BIT6,
	PORTS_BTN_LSTAR	= BIT7,
	PORTS_BTN_LBL		= BIT8,
	PORTS_BTN_LUP		= BIT9,
};

/* exclusive for openchronos.c, modules should NOT use this directly */
volatile enum ports_buttons ports_pressed_btns;

void init_buttons(void);

#endif /* __PORTS_H__ */
