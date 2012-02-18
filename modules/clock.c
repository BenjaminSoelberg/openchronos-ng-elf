/******************************************************************************
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
******************************************************************************/

#include "clock.h"

/* driver */
#include "ports.h"
#include "display.h"
#include "rtca.h"

struct time sTime;

void clock_event(rtca_tevent_ev_t ev)
{
	/* Exit if we are in edit mode */
	if (sTime.edit_state != EDIT_STATE_OFF)
		return;

	uint16_t yy;
	uint8_t hh, mm, ss, dd, dw, mo;
	rtca_get_time(&hh, &mm, &ss);
	rtca_get_date(&yy, &mo, &dd, &dw);

	switch (ev) {
	case RTCA_EV_MONTH:
		display_chars(LCD_SEG_L2_1_0, _itoa(mo, 2, 0), SEG_ON);

	case RTCA_EV_DAY:
		display_chars(LCD_SEG_L2_4_3, _itoa(dd, 2, 0), SEG_ON);

	case RTCA_EV_HOUR:
		display_chars(LCD_SEG_L1_3_2, _itoa(hh, 2, 0), SEG_ON);

	case RTCA_EV_MINUTE:
		display_chars(LCD_SEG_L1_1_0, _itoa(mm, 2, 0), SEG_ON);

	default:
		break;
	}
}

void clock_activated()
{
	rtca_tevent_fn_register(&clock_event);

	/* Force redraw of the screen */
#ifdef CONFIG_CLOCK_BLINKCOL
	display_symbol(LCD_SEG_L1_COL, SEG_ON_BLINK_ON);
#else
	display_symbol(LCD_SEG_L1_COL, SEG_ON);
#endif
	display_char(LCD_SEG_L2_2, '-', SEG_ON);

	clock_event(RTCA_EV_MONTH);
}

void clock_deactivated()
{
	rtca_tevent_fn_unregister(&clock_event);

	/* clean up screen */
	clear_line(LINE1);
	clear_line(LINE2);
}

static void edit_inc()
{
	switch (sTime.edit_state) {
	case EDIT_STATE_MO:
		helpers_loop_up(&sTime.tmp_mo, 1, 12);

		display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_mo, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_DD:
		/* TODO: Fix this, decide where to display year.. */
		helpers_loop_up(&sTime.tmp_dd, 1,
				rtca_get_max_days(sTime.tmp_mo, sTime.tmp_yy));

		display_chars(LCD_SEG_L2_4_3, _itoa(sTime.tmp_dd, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_MM:
		helpers_loop_up(&sTime.tmp_mm, 0, 59);

		display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_mm, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_HH:
		/* TODO: fix for 12/24 hr! */
		helpers_loop_up(&sTime.tmp_hh, 0, 23);

		display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hh, 2, 0),
							SEG_ON_BLINK_ON);
		break;
	default:
		break;
	}
}

static void edit_dec()
{
	switch (sTime.edit_state) {
	case EDIT_STATE_MO:
		helpers_loop_down(&sTime.tmp_mo, 1, 12);

		display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_mo, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_DD:
		/* TODO: Fix this, decide where to display year.. */
		helpers_loop_down(&sTime.tmp_dd, 1,
				rtca_get_max_days(sTime.tmp_mo, sTime.tmp_yy));

		display_chars(LCD_SEG_L2_4_3, _itoa(sTime.tmp_dd, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_MM:
		helpers_loop_down(&sTime.tmp_mm, 0, 59);

		display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_mm, 2, 0),
							SEG_ON_BLINK_ON);
		break;

	case EDIT_STATE_HH:
		/* TODO: fix for 12/24 hr! */
		helpers_loop_down(&sTime.tmp_hh, 0, 23);

		display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hh, 2, 0),
							SEG_ON_BLINK_ON);
		break;
	default:
		break;
	}
}

static void edit_next()
{
	helpers_loop_up(&sTime.edit_state, EDIT_STATE_HH, EDIT_STATE_DD);

	display_chars(LCD_SEG_L2_1_0, _itoa(sTime.tmp_mo, 2, 0),
		(sTime.edit_state == EDIT_STATE_MO ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));

	display_chars(LCD_SEG_L2_4_3, _itoa(sTime.tmp_dd, 2, 0),
		(sTime.edit_state == EDIT_STATE_DD ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));

	display_chars(LCD_SEG_L1_1_0, _itoa(sTime.tmp_mm, 2, 0),
		(sTime.edit_state == EDIT_STATE_MM ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));

	display_chars(LCD_SEG_L1_3_2, _itoa(sTime.tmp_hh, 2, 0),
		(sTime.edit_state == EDIT_STATE_HH ?
					SEG_ON_BLINK_ON : SEG_ON_BLINK_OFF));
}

static void edit_save()
{
	/* Here we return from the edit mode, fill in the new values! */
	rtca_set_time(sTime.tmp_hh, sTime.tmp_mm, 0);
	rtca_set_date(sTime.tmp_yy, sTime.tmp_mo, sTime.tmp_dd);

	/* hack to only turn off SOME blinking segments */
	display_chars(LCD_SEG_L1_1_0, _itoa(88, 2, 0), SEG_ON_BLINK_OFF);
	display_chars(LCD_SEG_L1_3_2, _itoa(88, 2, 0), SEG_ON_BLINK_OFF);

	/* set edit mode state to off */
	sTime.edit_state = EDIT_STATE_OFF;

	/* force redraw of the screen */
	clock_event(RTCA_EV_HOUR);
}


/* Star button long press callback. */
static void star_long_pressed()
{
	/* We go into edit mode  */
	sTime.edit_state = EDIT_STATE_DD;

	/* Save the current time in edit_buffer */
	uint8_t tmp;
	rtca_get_time(&sTime.tmp_hh, &sTime.tmp_mm, &tmp);
	rtca_get_date(&sTime.tmp_yy, &sTime.tmp_mo, &sTime.tmp_dd, &tmp);

	menu_editmode_start(&edit_inc, &edit_dec, &edit_next, &edit_save);

}

void clock_init()
{
	sTime.edit_state = EDIT_STATE_OFF;

	menu_add_entry(NULL, NULL, NULL,
		       &star_long_pressed,
				 NULL,
		       &clock_activated,
		       &clock_deactivated
		      );

}
