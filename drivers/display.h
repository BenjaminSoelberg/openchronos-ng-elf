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

/*!
	\file display.h
	\brief openchronos-ng display driver
*/

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <openchronos.h>

/*!
	\brief Enumeration of segment states
*/
enum display_segstate {
	SEG_OFF		= 1u, /*!< turn selected segment bits to OFF */
	SEG_ON		= 2u, /*!< turn selected segment bits to ON */
	SEG_SET		= 3u, /*!< turn OFF all bits of segment, then turn ON only selected bits */
	BLINK_OFF	= 4u, /*!< turn blinking OFF on selected segment bits */
	BLINK_ON		= 8u, /*!< turn blinking ON on selected segment bits */
	BLINK_SET	= 12u /*!< turn blinking OFF on all bits of segment, then turn blinking ON on only selected bits */
};

/*!
	\brief Enumeration of all ez430 chronos LCD segments
*/
enum display_segment {
	/* Symbols for Line1 */
	LCD_SYMB_AM				=	0, /*!< AM symbol segment */
	LCD_SYMB_PM				=	1, /*!< PM symbol segment */
	LCD_SYMB_ARROW_UP		=	2, /*!< little up arrow symbol segment */
	LCD_SYMB_ARROW_DOWN	=	3, /*!< little down arrow symbol segment */
	LCD_SYMB_PERCENT		=	4, /*!< percent symbol segment */
	
	/* Symbols for Line2 */
	LCD_SYMB_TOTAL			=	5, /*!< TOTAL symbol segment */
	LCD_SYMB_AVERAGE		=	6, /*!< AVG symbol segment */
	LCD_SYMB_MAX			=	7, /*!< MAX symbol segment */
	LCD_SYMB_BATTERY		=	8, /*!< BATT symbol segment */
	
	/* Units for Line1 */
	LCD_UNIT_L1_FT			=	9,  /*!< FT symbol segment */
	LCD_UNIT_L1_K			=	10, /*!< K symbol segment */
	LCD_UNIT_L1_M			=	11, /*!< M symbol segment */
	LCD_UNIT_L1_I			=	12, /*!< I symbol segment */
	LCD_UNIT_L1_PER_S		=	13, /*!< /s symbol segment */
	LCD_UNIT_L1_PER_H		=	14, /*!< /h symbol segment */
	LCD_UNIT_L1_DEGREE	=	15, /*!< º symbol segment */
	
	/* Units for Line2 */
	LCD_UNIT_L2_KCAL		=	16, /*!< kcal symbol segment */
	LCD_UNIT_L2_KM			=	17, /*!< Km symbol segment */
	LCD_UNIT_L2_MI			=	18, /*!< MI symbol segment */
	
	/* Icons */
	LCD_ICON_HEART			=	19, /*!< HEART symbol segment */
	LCD_ICON_STOPWATCH	=	20, /*!< STOPWATCH symbol segment */
	LCD_ICON_RECORD		=	21, /*!< RECORD symbol segment */
	LCD_ICON_ALARM			=	22, /*!< ALARM symbol segment */
	LCD_ICON_BEEPER1		=	23, /*!< segment 1 of ((( symbol */
	LCD_ICON_BEEPER2		=	24, /*!< segment 2 of ((( symbol */
	LCD_ICON_BEEPER3		=	25, /*!< segment 3 of ((( symbol */
	
	/* Line1 7-segments */
	LCD_SEG_L1_3			=	26, /*!< line1, 4th segment */
	LCD_SEG_L1_2			=	27, /*!< line1, 3rd segment */
	LCD_SEG_L1_1			=	28, /*!< line1, 2nd segment */
	LCD_SEG_L1_0			=	29, /*!< line1, 1st segment */
	LCD_SEG_L1_COL			=	30, /*!< line1, : segment */
	LCD_SEG_L1_DP1			=	31, /*!< ?? */
	LCD_SEG_L1_DP0			=	32, /*!< ?? */
	
	/* Line2 7-segments */
	LCD_SEG_L2_5			=	33, /*!< line2, 6th segment */
	LCD_SEG_L2_4			=	34, /*!< line2, 5th segment */
	LCD_SEG_L2_3			=	35, /*!< line2, 4th segment */
	LCD_SEG_L2_2			=	36, /*!< line2, 3rd segment */
	LCD_SEG_L2_1			=	37, /*!< line2, 2nd segment */
	LCD_SEG_L2_0			=	38, /*!< line2, 1st segment */
	LCD_SEG_L2_COL1		=	39, /*!< line2, 2nd : segment */
	LCD_SEG_L2_COL0		=	40, /*!< line2, 1st : segment */
	LCD_SEG_L2_DP			=	41, /*!< ?? */
};

/*!
	\brief Enumeration of LCD segment arrays
	\details The LCD_SEG_L1_3_2 member means the segments on line 1 from position 3 to 2 (inclusive). Segments are numbered from right to left.
	\sa #display_chars()
*/
enum display_segment_array {
	/* Line1 7-segment arrays */
	LCD_SEG_L1_3_2			=	0xc2, /*!< line1, segments 3-2 */
	LCD_SEG_L1_3_1			=	0xc3, /*!< line1, segments 3-1 */
	LCD_SEG_L1_3_0			=	0xc4, /*!< line1, segments 3-0 */
	LCD_SEG_L1_2_1			=	0xb2, /*!< line1, segments 2-1 */
	LCD_SEG_L1_2_0			=	0xb3, /*!< line1, segments 2-0 */
	LCD_SEG_L1_1_0			=	0xa2, /*!< line1, segments 1-0 */
	
	/* Line2 7-segment arrays */
	LCD_SEG_L2_5_4			=	0x52, /*!< line2, segments 5-4 */
	LCD_SEG_L2_5_3			=	0x53, /*!< line2, segments 5-3 */
	LCD_SEG_L2_5_2			=	0x54, /*!< line2, segments 3-2 */
	LCD_SEG_L2_5_1			=	0x55, /*!< line2, segments 3-1 */
	LCD_SEG_L2_5_0			=	0x56, /*!< line2, segments 1-0 */
	LCD_SEG_L2_4_3			=	0x42, /*!< line2, segments 4-3 */
	LCD_SEG_L2_4_2			=	0x43, /*!< line2, segments 4-2 */
	LCD_SEG_L2_4_1			=	0x44, /*!< line2, segments 4-1 */
	LCD_SEG_L2_4_0			=	0x45, /*!< line2, segments 4-0 */
	LCD_SEG_L2_3_2			=	0x32, /*!< line2, segments 3-2 */
	LCD_SEG_L2_3_1			=	0x33, /*!< line2, segments 3-1 */
	LCD_SEG_L2_3_0			=	0x34, /*!< line2, segments 3-0 */
	LCD_SEG_L2_2_1			=	0x22, /*!< line2, segments 2-1 */
	LCD_SEG_L2_2_0			=	0x23, /*!< line2, segments 2-0 */
	LCD_SEG_L2_1_0			=	0x12, /*!< line2, segments 1-0 */
};

/*!
	\brief Virtual LCD screen
	\sa #lcd_screens_create()
*/
struct lcd_screen {
	uint8_t *segmem; /*!< pointer to segment memory location */
	uint8_t *blkmem; /*!< pointer to blinking memory location */
};

/*!
	\brief Creates virtual screens
	\details Virtual screens are used to display data outside of the real screen. After creating <i>nr</i> number of screens, you can select which screen to write data to using the <i>scr_nr</i> argument available in any of the display functions:<br />
	 #display_symbol()<br />
    #display_char()<br />
    #display_chars()<br />
    #display_clear()<br />

	After creating the virtual screens using this function, the screen 0 is always selected as the active screen. This means that any writes to screen 0 will actually be imediately displayed on the real screen, while writes to other screens will be saved until lcd_screen_activate() is called.
	\note Each virtual screen takes 24bytes of memory. It is less than the code that you would actually need to write to handle the cases where these functions are meant to be used. However, RAM memory on the ez430 chronos is limited too so don't use a bazilion of screens.
	\note Never, ever forget to destroy the created screens using lcd_screens_destroy() !
	\sa lcd_screens_destroy(), lcd_screen_activate()
*/
void lcd_screens_create(
	uint8_t nr /*!< the number of screens to create */
);

/*!
	\brief Destroys all virtual screen
	\details Destroys all #lcd_screen created by #lcd_screens_create()
*/
void lcd_screens_destroy(void);

uint8_t get_active_lcd_screen_nr(void);

/*!
	\brief Activates a virtual screen
	\details Virtual screens are used to display data outside of the real screen. See lcd_screens_create() on how to create virtual screens.<br />
	This function selects the active screen. The active screen is the screen where any writes to it will be imediately displayed in the real screen.
	\note If you set the <i>scr_nr</i> to 0xff, the next screen will be automatically activated.
	\sa lcd_screens_destroy(), lcd_screens_create()
*/
void lcd_screen_activate(
	uint8_t scr_nr /*!< the screen number to activate, or 0xff */
);

/* Not to be used by modules */
void start_blink(void);
void stop_blink(void);
void clear_blink_mem(void);

/*!
	\brief Clears the screen
	\details Clears the screen as instructed by <i>line</i>. If no virtual screens are created, the argument <i>scr_nr</i> is ignored, otherwise it selects which screen the operation will affect.
*/
void display_clear(
	uint8_t scr_nr, /*!< the virtual screen number to clear */
	uint8_t line /*!< If zero, clears the entire screen (symbols and lines).<br />If one, clears the first line.<br />If two, clears the second line. */
);

/* 
    \brief Display a custom collection of segments
    \details Changes the <i>state</i> of <i>segment</i> state according to user specified <i>bits</i>. If no virtual screens are created, the argument <i>scr_nr</i> is ignored, otherwise it selects which screen the operation will affect.
    \sa #display_char()
*/
void display_bits(
	uint8_t scr_nr, /*!< the virtual screen number where to display */
	enum display_segment segment, /*!< A segment */
	uint8_t bits, /*!< The bits of the segment */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);

/*!
	\brief Displays a single character
	\details Changes the <i>state</i> of <i>segment</i> state according to bits calculated from <i>chr</i>. If no virtual screens are created, the argument <i>scr_nr</i> is ignored, otherwise it selects which screen the operation will affect.

	For example, the following line:<br />
	\code
	display_char(0, LCD_SEG_L1_3, 'C', SEG_SET);<br />
	\endcode
	Changes the bits of the 4th segment (from the right) of first line to show a 'C' character. The 'C' character is shown in the real screen.

	Another but a little more complex example:<br />
	\code
	// changes the bits of the 4th segment (from the right) of first line to show the '1' character in the real screen.
	display_char(0, LCD_SEG_L1_3, '1', SEG_SET);

	// makes all bits of the segment to blink. because only the bits corresponding to the '1' character are being shown, only the displayed '1' blinks.
	display_char(0, LCD_SEG_L1_3, '8', BLINK_ON);

	// changes the bits of the segment to show the '8' character. this operation doesn't change the blinking bits, so the displayed '8' will blink.
	display_char(0, LCD_SEG_L1_3, '8', SEG_SET);

	// turns off blinking for the bits corresponding to the '-' character. In this case because the bits of '8' were blinking, turning off '-' will make the bits of '0' continue blinking, while the '-' remains static.
	display_char(0, LCD_SEG_L1_3, '-', BLINK_OFF);
	\endcode
	\sa #display_chars()
*/
void display_char(
	uint8_t scr_nr, /*!< the virtual screen number where to display */
	enum display_segment segment, /*!< A segment */
	char chr, /*!< The character to be displayed */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);


/*!
	\brief Displays several consecutive characters
	\details Smiliar to #display_char() except it works with a vector of chars.

	Example:<br />
	\code
	// changes the bits of the 4th to 1st segments (from the right) of the first line to show the "4321" string in the real screen.
	display_chars(0, LCD_SEG_L1_3_0, "4321", SEG_SET);

	// makes all bits of the first three segments to blink. because only the bits corresponding to the "4321" characters are being shown, only the first three displayed "432" segments blink.
	display_chars(0, LCD_SEG_L1_3_0, "888 ", BLINK_ON);

	// changes the bits of the segments to show the "8888" string. this operation doesn't change the blinking bits, so the first three displayed "888" segments will blink, while the last '8' segment is static.
	display_chars(0, LCD_SEG_L1_3_0, "8888", SEG_SET);

	// turns off blinking for the bits corresponding to the "----" characters. In this case because the bits of "888 " were blinking, turning off "----" will actually set the blinking bits to "000 ". Because the "8888" string is being displayed, only the first three "000" bits will blink, the first three "---" bits will remain static and the last segment remains static.
	display_chars(0, LCD_SEG_L1_3_0, "----", BLINK_OFF);
	\endcode
	Also, passing NULL as the <i>str</i> argument is equivalent of passing a vector of '8' characters. Consider the previous example, where the string "8888" can equivalently be replaced with NULL.

	\note See #_sprintf() on how to convert decimals into a string.
	\sa #display_char(), #_sprintf()
*/
void display_chars(
	uint8_t scr_nr, /*!< the virtual screen number where to display */
	enum display_segment_array segments, /*!< A segment array */
	char const * str, /*!< A pointer to a vector of chars to be displayed */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);

/*!
	\brief Displays a symbol
	\details Changes the <i>state</i> of the segment of <i>symbol</i>. If no virtual screens are created, the argument <i>scr_nr</i> is ignored, otherwise it selects which screen the operation will affect.
	Example:
	\code
	// turns on the "heart" segment and make it blink
	display_symbol(0, LCD_ICON_HEART, SEG_SET | BLINK_ON);
	\endcode
*/
void display_symbol(
	uint8_t scr_nr,       /*!< the virtual screen number */
	enum display_segment symbol, /*!< the segment to display */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);

/*!
	\brief pseudo printf function
	\details Displays in screen <i>scr_nr</i>, at segments <i>segments</i>, the string containing the number <i>n</i> formatted according to <i>fmt</i>. This function is equivalent to calling display_chars(scr_nr, segments, _sprintf(fmt, n), SEG_SET).
	\sa #display_chars, #_sprintf
*/

#define _printf(scr_nr, segments, fmt, n) \
	display_chars((scr_nr), (segments), _sprintf((fmt), (n)), SEG_SET)


/*!
	\brief pseudo sprintf function
	\details Returns a pointer to the string containing the number <i>n</i> formatted according to <i>fmt</i>. The format is NOT compatible with stdio's format.
	Example:
	\code
	// returns " 8"
	_sprintf("%2u", 8);
	
	// returns "0020"
	_sprintf("%04u", 20);

	// returns "-048"
	_sprintf("%03s", -48);

	// returns " 048"
	_sprintf("%03s", 48);

	// returns "0xff"
	_sprintf("0x%02", 0xff);

	// returns "st1x"
	_sprintf("st%1ux", 1)
	\endcode
	
	<b>WARNING:</b> You must always specify the number of digits or bad results will happen! "%u" formats are not allowed! 

	\return a pointer to a string
*/

char *_sprintf(
	const char *fmt, /*!< the format specifier */
	int16_t n        /*!< the number to be used in the format specifier */
);

/*!
	\brief Converts an integer from a range into a percent string between 0 and 100
	\details Takes the number <i>n</i> and returns a string representation of that number as a percent between low and high. The returned string is 3 characters long.

	Example:
	\code
	// this returns "4F"
	uint8_t *s = _itoa(0x4F, 2, 0);
	\endcode
	\return a string representation of <i>n</i>
*/
char *_itopct(
		uint32_t low,     /*!< the 0% value */
		uint32_t high,     /*!< the 100% value */
		uint32_t n
);

#endif /* __DISPLAY_H__ */
