/*
    drivers/display.c: Display driver for the eZ430-chronos

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

#include <openchronos.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"

/* Swap nibble */
#define SWAP_NIBBLE(x)              ((((x) << 4) & 0xF0) | (((x) >> 4) & 0x0F))

/* LCD controller memory map */
#define LCD_MEM_1          			((uint8_t*)0x0A20)
#define LCD_MEM_2          			((uint8_t*)0x0A21)
#define LCD_MEM_3          			((uint8_t*)0x0A22)
#define LCD_MEM_4          			((uint8_t*)0x0A23)
#define LCD_MEM_5          			((uint8_t*)0x0A24)
#define LCD_MEM_6          			((uint8_t*)0x0A25)
#define LCD_MEM_7          			((uint8_t*)0x0A26)
#define LCD_MEM_8          	 		((uint8_t*)0x0A27)
#define LCD_MEM_9          			((uint8_t*)0x0A28)
#define LCD_MEM_10         			((uint8_t*)0x0A29)
#define LCD_MEM_11         			((uint8_t*)0x0A2A)
#define LCD_MEM_12         			((uint8_t*)0x0A2B)


/* Memory assignment */
#define LCD_SEG_L1_0_MEM			(LCD_MEM_6)
#define LCD_SEG_L1_1_MEM			(LCD_MEM_4)
#define LCD_SEG_L1_2_MEM			(LCD_MEM_3)
#define LCD_SEG_L1_3_MEM			(LCD_MEM_2)
#define LCD_SEG_L1_COL_MEM			(LCD_MEM_1)
#define LCD_SEG_L1_DP1_MEM			(LCD_MEM_1)
#define LCD_SEG_L1_DP0_MEM			(LCD_MEM_5)
#define LCD_SEG_L2_0_MEM			(LCD_MEM_8)
#define LCD_SEG_L2_1_MEM			(LCD_MEM_9)
#define LCD_SEG_L2_2_MEM			(LCD_MEM_10)
#define LCD_SEG_L2_3_MEM			(LCD_MEM_11)
#define LCD_SEG_L2_4_MEM			(LCD_MEM_12)
#define LCD_SEG_L2_5_MEM			(LCD_MEM_12)
#define LCD_SEG_L2_COL1_MEM			(LCD_MEM_1)
#define LCD_SEG_L2_COL0_MEM			(LCD_MEM_5)
#define LCD_SEG_L2_DP_MEM			(LCD_MEM_9)
#define LCD_SYMB_AM_MEM				(LCD_MEM_1)
#define LCD_SYMB_PM_MEM				(LCD_MEM_1)
#define LCD_SYMB_ARROW_UP_MEM		(LCD_MEM_1)
#define LCD_SYMB_ARROW_DOWN_MEM		(LCD_MEM_1)
#define LCD_SYMB_PERCENT_MEM		(LCD_MEM_5)
#define LCD_SYMB_TOTAL_MEM			(LCD_MEM_11)
#define LCD_SYMB_AVERAGE_MEM		(LCD_MEM_10)
#define LCD_SYMB_MAX_MEM			(LCD_MEM_8)
#define LCD_SYMB_BATTERY_MEM		(LCD_MEM_7)
#define LCD_UNIT_L1_FT_MEM			(LCD_MEM_5)
#define LCD_UNIT_L1_K_MEM			(LCD_MEM_5)
#define LCD_UNIT_L1_M_MEM			(LCD_MEM_7)
#define LCD_UNIT_L1_I_MEM			(LCD_MEM_7)
#define LCD_UNIT_L1_PER_S_MEM		(LCD_MEM_5)
#define LCD_UNIT_L1_PER_H_MEM		(LCD_MEM_7)
#define LCD_UNIT_L1_DEGREE_MEM		(LCD_MEM_5)
#define LCD_UNIT_L2_KCAL_MEM		(LCD_MEM_7)
#define LCD_UNIT_L2_KM_MEM			(LCD_MEM_7)
#define LCD_UNIT_L2_MI_MEM			(LCD_MEM_7)
#define LCD_ICON_HEART_MEM			(LCD_MEM_2)
#define LCD_ICON_STOPWATCH_MEM		(LCD_MEM_3)
#define LCD_ICON_RECORD_MEM			(LCD_MEM_1)
#define LCD_ICON_ALARM_MEM			(LCD_MEM_4)
#define LCD_ICON_BEEPER1_MEM		(LCD_MEM_5)
#define LCD_ICON_BEEPER2_MEM		(LCD_MEM_6)
#define LCD_ICON_BEEPER3_MEM		(LCD_MEM_7)

/* Bit masks for write access */
#define LCD_SEG_L1_0_MASK			(BIT2+BIT1+BIT0+BIT7+BIT6+BIT5+BIT4)
#define LCD_SEG_L1_1_MASK			(BIT2+BIT1+BIT0+BIT7+BIT6+BIT5+BIT4)
#define LCD_SEG_L1_2_MASK			(BIT2+BIT1+BIT0+BIT7+BIT6+BIT5+BIT4)
#define LCD_SEG_L1_3_MASK			(BIT2+BIT1+BIT0+BIT7+BIT6+BIT5+BIT4)
#define LCD_SEG_L1_COL_MASK			(BIT5)
#define LCD_SEG_L1_DP1_MASK			(BIT6)
#define LCD_SEG_L1_DP0_MASK			(BIT2)
#define LCD_SEG_L2_0_MASK			(BIT3+BIT2+BIT1+BIT0+BIT6+BIT5+BIT4)
#define LCD_SEG_L2_1_MASK			(BIT3+BIT2+BIT1+BIT0+BIT6+BIT5+BIT4)
#define LCD_SEG_L2_2_MASK			(BIT3+BIT2+BIT1+BIT0+BIT6+BIT5+BIT4)
#define LCD_SEG_L2_3_MASK			(BIT3+BIT2+BIT1+BIT0+BIT6+BIT5+BIT4)
#define LCD_SEG_L2_4_MASK			(BIT3+BIT2+BIT1+BIT0+BIT6+BIT5+BIT4)
#define LCD_SEG_L2_5_MASK			(BIT7)
#define LCD_SEG_L2_COL1_MASK		(BIT4)
#define LCD_SEG_L2_COL0_MASK		(BIT0)
#define LCD_SEG_L2_DP_MASK			(BIT7)
#define LCD_SYMB_AM_MASK			(BIT1+BIT0)
#define LCD_SYMB_PM_MASK			(BIT0)
#define LCD_SYMB_ARROW_UP_MASK		(BIT2)
#define LCD_SYMB_ARROW_DOWN_MASK	(BIT3)
#define LCD_SYMB_PERCENT_MASK		(BIT4)
#define LCD_SYMB_TOTAL_MASK			(BIT7)
#define LCD_SYMB_AVERAGE_MASK		(BIT7)
#define LCD_SYMB_MAX_MASK			(BIT7)
#define LCD_SYMB_BATTERY_MASK		(BIT7)
#define LCD_UNIT_L1_FT_MASK			(BIT5)
#define LCD_UNIT_L1_K_MASK			(BIT6)
#define LCD_UNIT_L1_M_MASK			(BIT1)
#define LCD_UNIT_L1_I_MASK			(BIT0)
#define LCD_UNIT_L1_PER_S_MASK		(BIT7)
#define LCD_UNIT_L1_PER_H_MASK		(BIT2)
#define LCD_UNIT_L1_DEGREE_MASK		(BIT1)
#define LCD_UNIT_L2_KCAL_MASK		(BIT4)
#define LCD_UNIT_L2_KM_MASK			(BIT5)
#define LCD_UNIT_L2_MI_MASK			(BIT6)
#define LCD_ICON_HEART_MASK			(BIT3)
#define LCD_ICON_STOPWATCH_MASK		(BIT3)
#define LCD_ICON_RECORD_MASK		(BIT7)
#define LCD_ICON_ALARM_MASK			(BIT3)
#define LCD_ICON_BEEPER1_MASK		(BIT3)
#define LCD_ICON_BEEPER2_MASK		(BIT3)
#define LCD_ICON_BEEPER3_MASK		(BIT3)

#define LCD_SEG_MEM     (LCD_MEM_1)
#define LCD_BLK_MEM   (LCD_MEM_1 + 0x20)
#define LCD_MEM_LEN   12

/***************************************************************************
 ***************************** LOCAL STORAGE *******************************
 **************************************************************************/

#define SPRINTF_STR_LEN 8

/* storage for itoa function */
static char sprintf_str[SPRINTF_STR_LEN];

/* pointer to active screen */
static struct lcd_screen *display_screens;
static uint8_t display_nrscreens;
static uint8_t display_activescr;

/* 7-segment character bit assignments */
#define SEG_A     (BIT4)
#define SEG_B     (BIT5)
#define SEG_C     (BIT6)
#define SEG_D     (BIT7)
#define SEG_E     (BIT2)
#define SEG_F     (BIT0)
#define SEG_G     (BIT1)

/* Table with memory bit assignment for digits "0"-"9" and chars "A"-"Z"
     A
   F   B
     G
   E   C
     D
*/
static const uint8_t lcd_font[] = {
	SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F, // Displays "0"
	SEG_B + SEG_C,                           // Displays "1"
	SEG_A + SEG_B +      SEG_D + SEG_E +      SEG_G, // Displays "2"
	SEG_A + SEG_B + SEG_C + SEG_D +            SEG_G, // Displays "3"
	SEG_B + SEG_C +            SEG_F + SEG_G, // Displays "4"
	SEG_A +      SEG_C + SEG_D +      SEG_F + SEG_G, // Displays "5"
	SEG_A +      SEG_C + SEG_D + SEG_E + SEG_F + SEG_G, // Displays "6"
	SEG_A + SEG_B + SEG_C,                         // Displays "7"
	SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G, // Displays "8"
	SEG_A + SEG_B + SEG_C + SEG_D +      SEG_F + SEG_G, // Displays "9"
	0                                        ,     // Displays " " (:)
	0                                        ,     // Displays " " (;)
	SEG_A +                        SEG_F + SEG_G,  // Displays "<" as high c
	SEG_D +            SEG_G,    // Displays "="
	0                                        ,     // Displays " " (>)
	SEG_A + SEG_B +            SEG_E +      SEG_G, // Displays "?"
	0                                        ,     // Displays " " (@)
	SEG_A + SEG_B + SEG_C +      SEG_E + SEG_F + SEG_G, // Displays "A"
	SEG_C + SEG_D + SEG_E + SEG_F + SEG_G, // Displays "b"
	SEG_D + SEG_E +      SEG_G,  // Displays "c"
	SEG_B + SEG_C + SEG_D + SEG_E +      SEG_G, // Displays "d"
	SEG_A +           +SEG_D + SEG_E + SEG_F + SEG_G, // Displays "E"
	SEG_A +                  SEG_E + SEG_F + SEG_G, // Displays "f"
	SEG_A + SEG_B + SEG_C + SEG_D +      SEG_F + SEG_G, // Displays "g" same as 9
	SEG_C +      SEG_E + SEG_F + SEG_G, // Displays "h"
	SEG_E            ,     // Displays "i"
	SEG_A + SEG_B + SEG_C + SEG_D                  , // Displays "J"
	SEG_D +      SEG_F + SEG_G,  // Displays "k"
	SEG_D + SEG_E + SEG_F      , // Displays "L"
	SEG_A + SEG_B + SEG_C +      SEG_E + SEG_F      , // Displays "M"
	SEG_C +      SEG_E +      SEG_G,   // Displays "n"
	SEG_C + SEG_D + SEG_E +      SEG_G, // Displays "o"
	SEG_A + SEG_B +            SEG_E + SEG_F + SEG_G, // Displays "P"
	SEG_A + SEG_B + SEG_C +            SEG_F + SEG_G, // Displays "q"
	SEG_E +      SEG_G,    // Displays "r"
	SEG_A +      SEG_C + SEG_D +      SEG_F + SEG_G, // Displays "S" same as 5
	SEG_D + SEG_E + SEG_F + SEG_G, // Displays "t"
	SEG_C + SEG_D + SEG_E            , // Displays "u"
	SEG_C + SEG_D + SEG_E            , // Displays "v" same as u
	SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G, // Displays "W"
	SEG_B + SEG_C +     +SEG_E + SEG_F + SEG_G, // Displays "X" as H
	SEG_B + SEG_C + SEG_D +      SEG_F + SEG_G, // Displays "Y"
	SEG_A + SEG_B +      SEG_D + SEG_E +      SEG_G, // Displays "Z" same as 2
	SEG_B + SEG_E + SEG_G, //Displays "[" as _|`
	0,                     //Displays "\" ( )
	SEG_C + SEG_F + SEG_G, //Displays "]" as `|_
	SEG_A,                 //Displays "^"
	SEG_D                 //Displays "_"
};


/* Table with memory address for each display element */
static const uint8_t *segments_lcdmem[] = {
	LCD_SYMB_AM_MEM,
	LCD_SYMB_PM_MEM,
	LCD_SYMB_ARROW_UP_MEM,
	LCD_SYMB_ARROW_DOWN_MEM,
	LCD_SYMB_PERCENT_MEM,
	LCD_SYMB_TOTAL_MEM,
	LCD_SYMB_AVERAGE_MEM,
	LCD_SYMB_MAX_MEM,
	LCD_SYMB_BATTERY_MEM,
	LCD_UNIT_L1_FT_MEM,
	LCD_UNIT_L1_K_MEM,
	LCD_UNIT_L1_M_MEM,
	LCD_UNIT_L1_I_MEM,
	LCD_UNIT_L1_PER_S_MEM,
	LCD_UNIT_L1_PER_H_MEM,
	LCD_UNIT_L1_DEGREE_MEM,
	LCD_UNIT_L2_KCAL_MEM,
	LCD_UNIT_L2_KM_MEM,
	LCD_UNIT_L2_MI_MEM,
	LCD_ICON_HEART_MEM,
	LCD_ICON_STOPWATCH_MEM,
	LCD_ICON_RECORD_MEM,
	LCD_ICON_ALARM_MEM,
	LCD_ICON_BEEPER1_MEM,
	LCD_ICON_BEEPER2_MEM,
	LCD_ICON_BEEPER3_MEM,
	LCD_SEG_L1_3_MEM,
	LCD_SEG_L1_2_MEM,
	LCD_SEG_L1_1_MEM,
	LCD_SEG_L1_0_MEM,
	LCD_SEG_L1_COL_MEM,
	LCD_SEG_L1_DP1_MEM,
	LCD_SEG_L1_DP0_MEM,
	LCD_SEG_L2_5_MEM,
	LCD_SEG_L2_4_MEM,
	LCD_SEG_L2_3_MEM,
	LCD_SEG_L2_2_MEM,
	LCD_SEG_L2_1_MEM,
	LCD_SEG_L2_0_MEM,
	LCD_SEG_L2_COL1_MEM,
	LCD_SEG_L2_COL0_MEM,
	LCD_SEG_L2_DP_MEM,
};


/* Table with bit mask for each display element */
static const uint8_t segments_bitmask[] = {
	LCD_SYMB_AM_MASK,
	LCD_SYMB_PM_MASK,
	LCD_SYMB_ARROW_UP_MASK,
	LCD_SYMB_ARROW_DOWN_MASK,
	LCD_SYMB_PERCENT_MASK,
	LCD_SYMB_TOTAL_MASK,
	LCD_SYMB_AVERAGE_MASK,
	LCD_SYMB_MAX_MASK,
	LCD_SYMB_BATTERY_MASK,
	LCD_UNIT_L1_FT_MASK,
	LCD_UNIT_L1_K_MASK,
	LCD_UNIT_L1_M_MASK,
	LCD_UNIT_L1_I_MASK,
	LCD_UNIT_L1_PER_S_MASK,
	LCD_UNIT_L1_PER_H_MASK,
	LCD_UNIT_L1_DEGREE_MASK,
	LCD_UNIT_L2_KCAL_MASK,
	LCD_UNIT_L2_KM_MASK,
	LCD_UNIT_L2_MI_MASK,
	LCD_ICON_HEART_MASK,
	LCD_ICON_STOPWATCH_MASK,
	LCD_ICON_RECORD_MASK,
	LCD_ICON_ALARM_MASK,
	LCD_ICON_BEEPER1_MASK,
	LCD_ICON_BEEPER2_MASK,
	LCD_ICON_BEEPER3_MASK,
	LCD_SEG_L1_3_MASK,
	LCD_SEG_L1_2_MASK,
	LCD_SEG_L1_1_MASK,
	LCD_SEG_L1_0_MASK,
	LCD_SEG_L1_COL_MASK,
	LCD_SEG_L1_DP1_MASK,
	LCD_SEG_L1_DP0_MASK,
	LCD_SEG_L2_5_MASK,
	LCD_SEG_L2_4_MASK,
	LCD_SEG_L2_3_MASK,
	LCD_SEG_L2_2_MASK,
	LCD_SEG_L2_1_MASK,
	LCD_SEG_L2_0_MASK,
	LCD_SEG_L2_COL1_MASK,
	LCD_SEG_L2_COL0_MASK,
	LCD_SEG_L2_DP_MASK,
};

/***************************************************************************
 ***************************** LOCAL FUNCTIONS *****************************
 **************************************************************************/

static void write_lcd_mem(uint8_t *segmem, uint8_t *blkmem,
                  uint8_t bits, uint8_t bitmask, uint8_t state)
{
	if ( (state | SEG_OFF) == state) {
		// Clear all segments
		*segmem = (uint8_t)(*segmem & ~bitmask);
	}

	if ( (state | SEG_ON) == state) {
		// Set visible segments
		*segmem = (uint8_t)(*segmem | bits);
	}

	if ( (state | BLINK_OFF) == state) {
		// Clear blink segments
		*blkmem = (uint8_t)(*blkmem & ~bitmask);
	}

	if ( (state | BLINK_ON) == state) {
		// Set blink segments
		*blkmem = (uint8_t)(*blkmem | bits);
	}
}

/***************************************************************************
 **************************** EXPORTED FUNCTIONS ***************************
 **************************************************************************/

/*
	lcd_screens_create()
*/
void lcd_screens_create(uint8_t nr)
{
	/* allocate memory */
	display_nrscreens = nr;
	display_screens = malloc(sizeof(struct lcd_screen) * nr);

	/* the first screen is the active one */
	display_activescr = 0;
	display_screens[0].segmem = LCD_SEG_MEM;
	display_screens[0].blkmem = LCD_BLK_MEM;

	/* allocate mem for the remaining and copy real screen over */
	uint8_t i = 1;
	for (; i<nr; i++) {
		display_screens[i].segmem = malloc(LCD_MEM_LEN);
		display_screens[i].blkmem = malloc(LCD_MEM_LEN);
		memcpy(display_screens[i].segmem, LCD_SEG_MEM, LCD_MEM_LEN);
		memcpy(display_screens[i].blkmem, LCD_BLK_MEM, LCD_MEM_LEN);
	}
}

/*
	lcd_screens_destroy()
*/
void lcd_screens_destroy(void)
{
	uint8_t i = 0;

	/* switch to screen 0 and display any pending data */
	lcd_screen_activate(0);

	/* now we can delete all the screens */
	for (; i<display_nrscreens; i++) {
		if (i != display_activescr) {
			free(display_screens[i].segmem);
			free(display_screens[i].blkmem);
		}
		display_screens[i].segmem = NULL;
		display_screens[i].segmem = NULL;
	}

	free(display_screens);
	display_screens = NULL;
}

uint8_t get_active_lcd_screen_nr(void)
{
	return display_activescr;
}
/*
	lcd_screen_activate()
   - the memory pointed by display_screens is assumed to be on
	contiguos memory
	if scr_nr == 0xff, then activate next screen.
*/
void lcd_screen_activate(uint8_t scr_nr)
{
	uint8_t prevscr = display_activescr;

	if (scr_nr == 0xff)
		helpers_loop(&display_activescr, 0, display_nrscreens - 1, 1);
	else
		display_activescr = scr_nr;

	/* allocate memory for previous screen */
	display_screens[prevscr].segmem = malloc(LCD_MEM_LEN);
	display_screens[prevscr].blkmem = malloc(LCD_MEM_LEN);

	/* copy real screen contents to previous screen */
	memcpy(display_screens[prevscr].segmem, LCD_SEG_MEM, LCD_MEM_LEN);
	memcpy(display_screens[prevscr].blkmem, LCD_BLK_MEM, LCD_MEM_LEN);

	/* update real screen with contents from activated screen */
	memcpy(LCD_SEG_MEM, display_screens[display_activescr].segmem, LCD_MEM_LEN);
	memcpy(LCD_BLK_MEM, display_screens[display_activescr].blkmem, LCD_MEM_LEN);
	
	/* free memory from the activated screen */
	free(display_screens[display_activescr].segmem);
	free(display_screens[display_activescr].blkmem);

	/* set activated screen as real screen output */
	display_screens[display_activescr].segmem = LCD_SEG_MEM;
	display_screens[display_activescr].blkmem = LCD_BLK_MEM;
}

void display_clear(uint8_t scr_nr, uint8_t line)
{
	if (line == 1) {
		display_chars(scr_nr, LCD_SEG_L1_3_0, NULL, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L1_DP1, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L1_DP0, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L1_COL, SEG_OFF);
	} else if (line == 2) {
		display_chars(scr_nr, LCD_SEG_L2_5_0, NULL, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L2_DP, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(scr_nr, LCD_SEG_L2_COL0, SEG_OFF);
	} else {
		uint8_t *lcdptr = (display_screens ?
		            display_screens[scr_nr].segmem : (uint8_t *)LCD_SEG_MEM);
		uint8_t i = 1;

		for (; i <= 12; i++) {
			*(lcdptr++) = 0x00;
		}
	}
}

char *_sprintf(const char *fmt, int16_t n) {
	int8_t i = 0;
	int8_t j = 0;
		
	while (1) {
		/* copy chars until end of string or a int substitution is found */ 
		while (fmt[i] != '%') {
			if (fmt[i] == '\0' || j == SPRINTF_STR_LEN - 2) {
				sprintf_str[j] = '\0';
				return sprintf_str;
			}
			sprintf_str[j++] = fmt[i++];
		}
		i++;

		int8_t digits = 0;
		int8_t zpad = ' ';
		/* parse int substitution */
		while (fmt[i] != 's' && fmt[i] != 'u' && fmt[i] != 'x') {
			if (fmt[i] == '0')
				zpad = '0';
			else
				digits = fmt[i] - '0';
			i++;
		}

		/* show sign */
		if (fmt[i] == 's') {
			if (n < 0) {
				sprintf_str[j++] = '-';
				n = (~n) + 1;
			} else
				sprintf_str[j++] = ' ';
		}

		j += digits - 1;
		int8_t j1 = j + 1;

		/* convert int to string */
		if (fmt[i] == 'x') {
			do {
				sprintf_str[j--] = "0123456789ABCDEF"[n & 0x0F];
				n >>= 4;
				digits--;
			} while (n > 0);
		} else {
			do {
				sprintf_str[j--] = n % 10 + '0';
				n /= 10;
				digits--;
			} while (n > 0);
		}

		/* pad the remaining */
		while (digits > 0) {
			sprintf_str[j--] = zpad;
			digits--;
		}

		j = j1;
	}

	return sprintf_str;
}

// *************************************************************************************************
// @fn          _itopct
// @brief       Converts integer n to a percent string between low and high. (uses _itoa internally)
// @param       uint32_t low		0% value
//				uint32_t high		100% value
//				uint32_t n			integer to convert
//
// @return      uint8_t				string
// *************************************************************************************************
char *_itopct(uint32_t low,uint32_t high,uint32_t n)
{

	// Return "0" if the value is under the low
	if (n < low) return (char *) "  0";

	// Return "100" if the value is over the high
	if (n > high) return (char *) "100";

	return _sprintf("%3u", (((n*100)-(low*100))/(high-low)));
}

void display_symbol(uint8_t scr_nr, enum display_segment symbol,
                                               enum display_segstate state)
{
	if (symbol <= LCD_SEG_L2_DP) {
		// Get LCD memory address for symbol from table
		uint8_t *segmem = (uint8_t *)segments_lcdmem[symbol];
		uint8_t *blkmem = segmem + 0x20;

		if (display_screens) {
			/* get offset */
			uint8_t offset = segmem - LCD_MEM_1;
			
			segmem = display_screens[scr_nr].segmem + offset;
			blkmem = display_screens[scr_nr].blkmem + offset;
		}

		// Get bits for symbol from table
		uint8_t bits 	= segments_bitmask[symbol];

		// Write LCD memory
		// (bitmask for symbols equals bits)
		write_lcd_mem(segmem, blkmem, bits, bits, state);
	}
}

void display_bits(uint8_t scr_nr, enum display_segment segment,
                  uint8_t bits  , enum display_segstate state)
{
	// Write to single 7-segment character
	if ((segment >= LCD_SEG_L1_3) && (segment <= LCD_SEG_L2_DP)) {
		// Get LCD memory address for segment from table
		uint8_t *segmem = (uint8_t *)segments_lcdmem[segment];
		uint8_t *blkmem = segmem + 0x20;

		if (display_screens) {
			/* get offset */
			uint8_t offset = segmem - LCD_MEM_1;

			segmem = display_screens[scr_nr].segmem + offset;
			blkmem = display_screens[scr_nr].blkmem + offset;
		}

        // Get bitmask for character from table
        uint8_t bitmask = segments_bitmask[segment];

		// When addressing LINE2 7-segment characters need to swap high- and low-nibble,
		// because LCD COM/SEG assignment is mirrored against LINE1
		if (segment >= LCD_SEG_L2_5) {
			bits = SWAP_NIBBLE(bits);
		}

		// Physically write to LCD memory
		write_lcd_mem(segmem, blkmem, bits, bitmask, state);
	}
}

void display_char(uint8_t scr_nr, enum display_segment segment,
                  char chr, enum display_segstate state)
{
     uint8_t bits = 0;       // Bits to write (default ' ' blank)
 
     // Get bits from font set
     if ((chr >= 0x30) && (chr <= 0x5A)) {
         // Use font set
         bits = lcd_font[chr - 0x30];
     } else if (chr == 0x2D) {
         // '-' not in font set
         bits = BIT1;
     }
 
     // When addressing LCD_SEG_L2_5, need to convert ASCII '1' and 'L' to 1 bit,
     // because LCD COM/SEG assignment is special for this incomplete character
     if (segment == LCD_SEG_L2_5 && (chr == '1' || chr == 'L')) bits = SWAP_NIBBLE(BIT7);
 
     // Write bits to memory
     display_bits(scr_nr, segment, bits, state);
}

void display_chars(uint8_t scr_nr,
                   enum display_segment_array segments,
                   char const * str,
                   enum display_segstate state)
{
	uint8_t i = 0;
	uint8_t len = (segments & 0x0f);
	segments = 38 - (segments >> 4);

	for (; i < len; i++) {
		/* stop if we find a null termination */
		if (str) {
			if (str[i] == '\0')
				return;
			display_char(scr_nr, segments + i, str[i], state);
		 } else
			display_char(scr_nr, segments + i, '8', state);
	}
}

// *************************************************************************************************
// @fn          start_blink
// @brief       Start blinking.
// @param       none
// @return      none
// *************************************************************************************************
void start_blink(void)
{
	LCDBBLKCTL |= LCDBLKMOD0;
}


// *************************************************************************************************
// @fn          stop_blink
// @brief       Stop blinking.
// @param       none
// @return      none
// *************************************************************************************************
void stop_blink(void)
{
	LCDBBLKCTL &= ~LCDBLKMOD0;
}


// *************************************************************************************************
// @fn          stop_blink
// @brief       Clear blinking memory.
// @param       none
// @return      none
// *************************************************************************************************
void clear_blink_mem(void)
{
	LCDBMEMCTL |= LCDCLRBM;
}




