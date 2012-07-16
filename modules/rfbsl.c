/*
    modules/rfbsl.c: RFBSL module for openchronos-ng

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

	           http://www.openchronos-ng.sourceforge.net

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

// system
#include <openchronos.h>

// driver
#include <drivers/display.h>
#include <drivers/battery.h>

// Entry point of of the Flash Updater in BSL memory
#define CALL_RFSBL()   ((void (*)())0x1000)()

// *************************************************************************************************
// @fn          mx_rfbsl
// @brief       This functions starts the RFBSL
// @param       line		LINE1, LINE2
// @return      none
// *************************************************************************************************
static void updown_press()
{
	// Write RAM to indicate we will be downloading the RAM Updater first
	display_chars(0, LCD_SEG_L1_3_0, " RAM", SEG_ON);

	// Call RFBSL
	CALL_RFSBL();
}


// *************************************************************************************************
// @fn          display_rfbsl
// @brief       RFBSL display routine.
// @param       uint8_t line			LINE2
//				uint8_t update		DISPLAY_LINE_UPDATE_FULL
// @return      none
// *************************************************************************************************
static void rfbsl_activate()
{
	/* update screen */
	display_chars(0, LCD_SEG_L2_5_0, " RFBSL", SEG_ON);
}

static void rfbsl_deactivate()
{
	/* cleanup screen */
	display_clear(0, 2);
}

void mod_rfbsl_init(void)
{
	menu_add_entry("RFBSL", NULL, NULL, NULL, NULL, NULL,
						&updown_press,
						&rfbsl_activate,
						&rfbsl_deactivate);
}
