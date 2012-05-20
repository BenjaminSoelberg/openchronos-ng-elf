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
	\note Line segments are numbered from right to left.
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

	/* Line1 7-segment arrays */
	LCD_SEG_L1_3_0			=	70, /*!< line1, segments 1-4 */
	LCD_SEG_L1_2_0			=	71, /*!< line1, segments 1-3 */
	LCD_SEG_L1_1_0			=	72, /*!< line1, segments 1-2 */
	LCD_SEG_L1_3_1			=	73, /*!< line1, segments 2-4 */
	LCD_SEG_L1_3_2			=	74, /*!< line1, segments 3-4 */
	
	/* Line2 7-segment arrays */
	LCD_SEG_L2_5_0			=	90, /*!< line2, segments 1-6 */
	LCD_SEG_L2_4_0			=	91, /*!< line2, segments 1-5 */
	LCD_SEG_L2_3_0			=	92, /*!< line2, segments 1-4 */
	LCD_SEG_L2_2_0			=	93, /*!< line2, segments 1-3 */
	LCD_SEG_L2_1_0			=	94, /*!< line2, segments 1-2 */
	LCD_SEG_L2_5_2			=	95, /*!< line2, segments 3-6 */
	LCD_SEG_L2_3_2			=	96, /*!< line2, segments 3-4 */
	LCD_SEG_L2_5_4			=	97, /*!< line2, segments 5-6 */
	LCD_SEG_L2_4_2			=	98, /*!< line2, segments 3-5 */
	LCD_SEG_L2_4_3			=	99, /*!< line2, segments 4-5 */
};

/*!
	\brief Initializes the display
	\details This functions is called once upon system initialization.
	\note Modules are strictly forbidden to call this function.
	\internal
*/
void lcd_init(void);

/*!
	\brief Virtual LCD screen
	\sa #lcd_screen_create()
*/
struct lcd_screen {
	uint8_t *segmem; /*!< pointer to segment memory location */
	uint8_t *blkmem; /*!< pointer to blinking memory location */
};

/*!
	\brief Creates a virtual LCD screen
	\details A virtual screen is a pointer to #lcd_screen that you can pass to all the display functions:<br />
	 #display_symbol()<br />
    #display_char()<br />
    #display_chars()<br />
    #display_clear()<br />

	You can do all the operations of #display_segstate in a virtual screen.

	The information written by the above functions is not displayed in the real LCD. The information stored in a virtual screen can be displayed in the real
	screen using the lcd_screen_virtual_to_real(). If you want to directly write to the real screen, just set the screen parameter of any of the above functions to NULL.
	\note Each virtual screen takes 24bytes of memory. It is less than the code that you would actually need to write to handle the cases where these functions are meant to be used. However, RAM memory on the ez430 chronos is limited too so use this with care!
	\sa lcd_screen_destroy(), lcd_screen_real_to_virtual(), lcd_screen_virtual_to_real()
*/
inline void lcd_screen_create(
	struct lcd_screen *screen /*!< pointer to a lcd_screen storage */
);

/*!
	\brief Destroys a virtual LCD screen
	\details Destroys a #lcd_screen created by #lcd_screen_create()
*/
inline void lcd_screen_destroy(
	struct lcd_screen *screen /*!< pointer to the lcd_screen storage */
);

/*!
	\brief Copies the contents of the real screen into a virtual screen
*/
inline void lcd_screen_real_to_virtual(
	struct lcd_screen *screen /*!< pointer to the lcd_screen storage */
);

/*!
	\brief Copies the contents of a virtual screen into the real screen
	\details The copied contents are immediately displayed in the real screen.
*/
inline void lcd_screen_virtual_to_real(
	struct lcd_screen *screen /*!< pointer to the lcd_screen storage */
);

/* Not to be used by modules */
void start_blink(void);
void stop_blink(void);
void clear_blink_mem(void);
void set_blink_rate(uint8_t bits);

/*!
	\brief Clears the screen
	\details Clears the screen as instructed by <i>line</i>. If <i>screen</i> is NULL, then the clear operation affects the real screen. Otherwise it affects the virtual screen pointed by <i>screen</i>.
*/
void display_clear(
	struct lcd_screen *screen, /*!< pointer to the #lcd_screen storage */
	uint8_t line /*!< If zero, clears the entire screen (symbols and lines).<br />If one, clears the first line.<br />If two, clears the second line. */
);

/*!
	\brief Displays a single character
	\details Changes the <i>state</i> of <i>segment</i> state according to bits calculated from <i>chr</i>. If <i>screen</i> is NULL then this operation affects the real screen, otherwise it affects the screen pointed by <i>screen</i>.

	For example, the following line:<br />
	\code
	display_char(NULL, LCD_SEG_L1_3, 'C', SEG_SET);<br />
	\endcode
	Changes the bits of the 4th segment (from the right) of first line to show a 'C' character. The 'C' character is shown in the real screen.

	Another but a little more complex example:<br />
	\code
	// changes the bits of the 4th segment (from the right) of first line to show the '1' character in the real screen.
	display_char(NULL, LCD_SEG_L1_3, '1', SEG_SET);

	// makes all bits of the segment to blink. because only the bits corresponding to the '1' character are being shown, only the displayed '1' blinks.
	display_char(NULL, LCD_SEG_L1_3, '8', BLINK_ON);

	// changes the bits of the segment to show the '8' character. this operation doesn't change the blinking bits, so the displayed '8' will blink.
	display_char(NULL, LCD_SEG_L1_3, '8', SEG_SET);

	// turns off blinking for the bits corresponding to the '-' character. In this case because the bits of '8' were blinking, turning off '-' will make the bits of '0' continue blinking, while the '-' remains static.
	display_char(NULL, LCD_SEG_L1_3, '-', BLINK_OFF);
	\endcode
	\sa #display_chars()
*/
void display_char(
	struct lcd_screen *screen, /*!< pointer to the #lcd_screen storage */
	enum display_segment segment, /*!< A segment */
	uint8_t chr, /*!< The character to be displayed */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);


/*!
	\brief Displays several consecutive characters
	\details Smiliar to #display_char() except it works with a vector of chars.

	Example:<br />
	\code
	// changes the bits of the 4th to 1st segments (from the right) of the first line to show the "4321" string in the real screen.
	display_chars(NULL, LCD_SEG_L1_3_0, "4321", SEG_SET);

	// makes all bits of the first three segments to blink. because only the bits corresponding to the "4321" characters are being shown, only the first three displayed "432" segments blink.
	display_chars(NULL, LCD_SEG_L1_3_0, "888 ", BLINK_ON);

	// changes the bits of the segments to show the "8888" string. this operation doesn't change the blinking bits, so the first three displayed "888" segments will blink, while the last '8' segment is static.
	display_chars(NULL, LCD_SEG_L1_3_0, "8888", SEG_SET);

	// turns off blinking for the bits corresponding to the "----" characters. In this case because the bits of "888 " were blinking, turning off "----" will actually set the blinking bits to "000 ". Because the "8888" string is being displayed, only the first three "000" bits will blink, the first three "---" bits will remain static and the last segment remains static.
	display_chars(NULL, LCD_SEG_L1_3_0, "----", BLINK_OFF);
	\endcode
	Also, passing NULL as the <i>str</i> argument is equivalent of passing a vector of '8' characters. Consider the previous example, where the string "8888" can equivalently be replaced with NULL.

	\note See #_itoa() on how to convert decimals into a string.
	\sa #display_char(), #_itoa()
*/
void display_chars(
	struct lcd_screen *screen, /*!< pointer to the #lcd_screen storage */
	enum display_segment segments, /*!< A segment array */
	uint8_t *str, /*!< A pointer to a vector of chars to be displayed */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);

/*!
	\brief Displays a symbol
	\details Changes the <i>state</i> of the segment of <i>symbol</i>. If <i>screen</i> is NULL then this operation affects the real screen, otherwise it affects the screen pointed by <i>screen</i>.

	Example:
	\code
	// turns on the "heart" segment and make it blink
	display_symbol(NULL, LCD_ICON_HEART, SEG_SET | BLINK_ON);
	\endcode
*/
void display_symbol(
	struct lcd_screen *screen, /*!< pointer to the #lcd_screen storage */
	enum display_segment symbol, /*!< the segment to display */
	enum display_segstate state /*!< A bitfield with state operations to be performed on the segment */
);

/*!
	\brief Converts a decimal into a string
	\details Takes the number <i>n</i> and returns a string representation of that number with <i>digits</i> number of digits. The returned string is padded with <i>blanks</i> number of blank spacing.

	Example:
	\code
	// this returns "32"
	uint8_t *s = _itoa(32, 2, 0);
	\endcode
	\return a string representation of <i>n</i>
*/
uint8_t *_itoa(
	uint32_t n,     /*!< the number to convert to a string */
	uint8_t digits, /*!< the number of output digits */
	uint8_t blanks  /*!< the number of blanks to pad the output */
);

#endif /* __DISPLAY_H__ */
