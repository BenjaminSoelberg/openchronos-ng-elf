/**************************************************************************

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

****************************************************************************/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

enum display_segstate {
	SEG_OFF		= 1u,
	SEG_ON		= 2u,
	SEG_SET		= 3u,
	BLINK_OFF	= 4u,
	BLINK_ON		= 8u,
	BLINK_SET	= 12u
};

/* 7-segment character bit assignments */
#define SEG_A                	(BIT4)
#define SEG_B                	(BIT5)
#define SEG_C                	(BIT6)
#define SEG_D                	(BIT7)
#define SEG_E                	(BIT2)
#define SEG_F                	(BIT0)
#define SEG_G                	(BIT1)

/* LCD symbols for easier access

  xxx_SEG_xxx	= Seven-segment character (sequence 5-4-3-2-1-0)
  xxx_SYMB_xxx	= Display symbol, e.g. "AM" for ante meridiem
  xxx_UNIT_xxx	= Display unit, e.g. "km/h" for kilometers per hour
  xxx_ICON_xxx	= Display icon, e.g. heart
  xxx_L1_xxx	= Item is part of Line1 information
  xxx_L2_xxx	= Item is part of Line2 information
*/
/* Symbols for Line1 */
#define LCD_SYMB_AM					0
#define LCD_SYMB_PM					1
#define LCD_SYMB_ARROW_UP			2
#define LCD_SYMB_ARROW_DOWN		3
#define LCD_SYMB_PERCENT			4

/* Symbols for Line2 */
#define LCD_SYMB_TOTAL				5
#define LCD_SYMB_AVERAGE			6
#define LCD_SYMB_MAX					7
#define LCD_SYMB_BATTERY			8

/* Units for Line1 */
#define LCD_UNIT_L1_FT				9
#define LCD_UNIT_L1_K				10
#define LCD_UNIT_L1_M				11
#define LCD_UNIT_L1_I				12
#define LCD_UNIT_L1_PER_S			13
#define LCD_UNIT_L1_PER_H			14
#define LCD_UNIT_L1_DEGREE			15

/* Units for Line2 */
#define LCD_UNIT_L2_KCAL			16
#define LCD_UNIT_L2_KM				17
#define LCD_UNIT_L2_MI				18

/* Icons */
#define LCD_ICON_HEART				19
#define LCD_ICON_STOPWATCH			20
#define LCD_ICON_RECORD				21
#define LCD_ICON_ALARM				22
#define LCD_ICON_BEEPER1			23
#define LCD_ICON_BEEPER2			24
#define LCD_ICON_BEEPER3			25

/* Line1 7-segments */
#define LCD_SEG_L1_3					26
#define LCD_SEG_L1_2					27
#define LCD_SEG_L1_1					28
#define LCD_SEG_L1_0					29
#define LCD_SEG_L1_COL				30
#define LCD_SEG_L1_DP1				31
#define LCD_SEG_L1_DP0				32

/* Line2 7-segments */
#define LCD_SEG_L2_5					33
#define LCD_SEG_L2_4					34
#define LCD_SEG_L2_3					35
#define LCD_SEG_L2_2					36
#define LCD_SEG_L2_1					37
#define LCD_SEG_L2_0					38
#define LCD_SEG_L2_COL1				39
#define LCD_SEG_L2_COL0				40
#define LCD_SEG_L2_DP				41


/* Line1 7-segment arrays */
#define LCD_SEG_L1_3_0				70
#define LCD_SEG_L1_2_0				71
#define LCD_SEG_L1_1_0				72
#define LCD_SEG_L1_3_1				73
#define LCD_SEG_L1_3_2				74

/* Line2 7-segment arrays */
#define LCD_SEG_L2_5_0				90
#define LCD_SEG_L2_4_0				91
#define LCD_SEG_L2_3_0				92
#define LCD_SEG_L2_2_0				93
#define LCD_SEG_L2_1_0				94
#define LCD_SEG_L2_5_2				95
#define LCD_SEG_L2_3_2				96
#define LCD_SEG_L2_5_4				97
#define LCD_SEG_L2_4_2				98
#define LCD_SEG_L2_4_3				99


/* Display init / clear */
void lcd_init(void);

/* virtual screens */
struct lcd_screen {
	uint8_t *segmem;
	uint8_t *blkmem;
};

inline void lcd_screen_create(struct lcd_screen *screen);
inline void lcd_screen_destroy(struct lcd_screen *screen);
inline void lcd_screen_real_to_virtual(struct lcd_screen *screen);
inline void lcd_screen_virtual_to_real(struct lcd_screen *screen);

/* Blinking functions */
void start_blink(void);
void stop_blink(void);
void clear_blink_mem(void);
void set_blink_rate(uint8_t bits);

/* Screen update functions */
void display_clear(
	struct lcd_screen *screen,
	uint8_t line
);

void display_char(
	struct lcd_screen *screen,
	uint8_t segment,
	uint8_t chr,
	enum display_segstate state
);

void display_chars(
	struct lcd_screen *screen,
	uint8_t segments,
	uint8_t *str,
	enum display_segstate state
);

void display_symbol(
	struct lcd_screen *screen,
	uint8_t symbol,
	enum display_segstate state
);

/* Integer to string conversion */
extern uint8_t *_itoa(uint32_t n, uint8_t digits, uint8_t blanks);

#endif /* __DISPLAY_H__ */
