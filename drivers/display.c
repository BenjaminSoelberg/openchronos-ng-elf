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

#include <ezchronos.h>
#include <string.h>

#include "display.h"

/* storage for itoa function */
static uint8_t itoa_str[8];


void lcd_init(void)
{
	// Clear entire display memory
	LCDBMEMCTL |= LCDCLRBM + LCDCLRM;

	// LCD_FREQ = ACLK/12/8 = 341.3Hz flickers in the sun
	// LCD_FREQ = ACLK/10/8 = 409.6Hz still flickers in the sun when watch is moving (might be negligible)

	// LCD_FREQ = ACLK/8/8 = 512Hz no flickering, even when watch is moving
	// Frame frequency = 512Hz/2/4 = 64Hz, LCD mux 4, LCD on
	LCDBCTL0 = (LCDDIV0 + LCDDIV1 + LCDDIV2) | (LCDPRE0 + LCDPRE1) | LCD4MUX | LCDON;

	// LCB_BLK_FREQ = ACLK/8/4096 = 1Hz
	LCDBBLKCTL = LCDBLKPRE0 | LCDBLKPRE1 | LCDBLKDIV0 | LCDBLKDIV1 | LCDBLKDIV2 | LCDBLKMOD0;

	// I/O to COM outputs
	P5SEL |= (BIT5 | BIT6 | BIT7);
	P5DIR |= (BIT5 | BIT6 | BIT7);

	// Activate LCD output
	LCDBPCTL0 = 0xFFFF;                         // Select LCD segments S0-S15
	LCDBPCTL1 = 0x00FF;                         // Select LCD segments int16_t-S22

#ifdef USE_LCD_CHARGE_PUMP
	// Charge pump voltage generated internally, internal bias (V2-V4) generation
	LCDBVCTL = LCDCPEN | VLCD_2_72;
#endif
}


void display_clear(uint8_t line)
{
	if (line == 1) {
		display_chars(LCD_SEG_L1_3_0, NULL, SEG_OFF);
		display_symbol(LCD_SEG_L1_DP1, SEG_OFF);
		display_symbol(LCD_SEG_L1_DP0, SEG_OFF);
		display_symbol(LCD_SEG_L1_COL, SEG_OFF);
	} else if (line == 2) {
		display_chars(LCD_SEG_L2_5_0, NULL, SEG_OFF);
		display_symbol(LCD_SEG_L2_DP, SEG_OFF);
		display_symbol(LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(LCD_SEG_L2_COL0, SEG_OFF);
	} else {
		uint8_t *lcdptr = (uint8_t *)LCD_MEM_1;
		uint8_t i = 1;

		for (; i <= 12; i++) {
			*(lcdptr++) = 0x00;
		}
	}
}


static void write_lcd_mem(uint8_t *lcdmem, uint8_t bits,
						uint8_t bitmask, uint8_t state)
{
	if ( (state | SEG_OFF) == state) {
		// Clear all segments
		*lcdmem = (uint8_t)(*lcdmem & ~bitmask);
	}

	if ( (state | SEG_ON) == state) {
		// Set visible segments
		*lcdmem = (uint8_t)(*lcdmem | bits);
	}

	if ( (state | BLINK_OFF) == state) {
		// Clear blink segments
		*(lcdmem + 0x20) = (uint8_t)(*(lcdmem + 0x20) & ~bitmask);
	}

	if ( (state | BLINK_ON) == state) {
		// Set blink segments
		*(lcdmem + 0x20) = (uint8_t)(*(lcdmem + 0x20) | bits);
	}
}


// *************************************************************************************************
// @fn          _itoa
// @brief       Generic integer to array routine. Converts integer n to string.
//				Default conversion result has leading zeros, e.g. "00123"
//				Option to convert leading '0' into whitespace (blanks)
// @param       uint32_t n			integer to convert
//				uint8_t digits		number of digits
//				uint8_t blanks		fill up result string with number of whitespaces instead of leading zeros
// @return      uint8_t				string
// *************************************************************************************************
uint8_t *_itoa(uint32_t n, uint8_t digits, uint8_t blanks)
{
	uint8_t i;
	uint8_t digits1 = digits;

	// Preset result string
	memcpy(itoa_str, "0000000", 7);

	// Return empty string if number of digits is invalid (valid range for digits: 1-7)
	if ((digits == 0) || (digits > 7)) return (itoa_str);

	// Numbers 0 .. 180 can be copied from itoa_conversion_table without conversion
	if (n <= 180) {
		if (digits >= 3) {
			memcpy(itoa_str + (digits - 3), itoa_conversion_table[n], 3);
		} else { // digits == 1 || 2
			memcpy(itoa_str, itoa_conversion_table[n] + (3 - digits), digits);
		}
	} else { // For n > 180 need to calculate string content
		// Calculate digits from least to most significant number
		do {
			itoa_str[digits - 1] = n % 10 + '0';
			n /= 10;
		} while (--digits > 0);
	}

	// Remove specified number of leading '0', always keep last one
	i = 0;

	while ((itoa_str[i] == '0') && (i < digits1 - 1)) {
		if (blanks > 0) {
			// Convert only specified number of leading '0'
			itoa_str[i] = ' ';
			blanks--;
		}

		i++;
	}

	return (itoa_str);
}


// *************************************************************************************************
// @fn          display_symbol
// @brief       Switch symbol on or off on LCD.
// @param       uint8_t symbol		A valid LCD symbol (index 0..42)
//				uint8_t state		SEG_ON, SEG_OFF, SEG_BLINK
// @return      none
// *************************************************************************************************
void display_symbol(uint8_t symbol, uint8_t mode)
{
	uint8_t *lcdmem;
	uint8_t bits;

	if (symbol <= LCD_SEG_L2_DP) {
		// Get LCD memory address for symbol from table
		lcdmem 	= (uint8_t *)segments_lcdmem[symbol];

		// Get bits for symbol from table
		bits 	= segments_bitmask[symbol];

		// Write LCD memory
		// (bitmask for symbols equals bits)
		write_lcd_mem(lcdmem, bits, bits, mode);
	}
}


// *************************************************************************************************
// @fn          display_char
// @brief       Write to 7-segment characters.
// @param       uint8_t segment		A valid LCD segment
//				uint8_t chr			Character to display
//				uint8_t mode		SEG_ON, SEG_OFF, SEG_BLINK
// @return      none
// *************************************************************************************************
void display_char(uint8_t segment, uint8_t chr, uint8_t mode)
{
	uint8_t *lcdmem;			// Pointer to LCD memory
	uint8_t bitmask;			// Bitmask for character
	uint8_t bits, bits1;		// Bits to write

	// Write to single 7-segment character
	if ((segment >= LCD_SEG_L1_3) && (segment <= LCD_SEG_L2_DP)) {
		// Get LCD memory address for segment from table
		lcdmem = (uint8_t *)segments_lcdmem[segment];

		// Get bitmask for character from table
		bitmask = segments_bitmask[segment];

		// Get bits from font set
		if ((chr >= 0x30) && (chr <= 0x5A)) {
			// Use font set
			bits = lcd_font[chr - 0x30];
		} else if (chr == 0x2D) {
			// '-' not in font set
			bits = BIT1;
		} else {
			// Other characters map to ' ' (blank)
			bits = 0;
		}

		// When addressing LINE2 7-segment characters need to swap high- and low-nibble,
		// because LCD COM/SEG assignment is mirrored against LINE1
		if (segment >= LCD_SEG_L2_5) {
			bits1 = ((bits << 4) & 0xF0) | ((bits >> 4) & 0x0F);
			bits = bits1;

			// When addressing LCD_SEG_L2_5, need to convert ASCII '1' and 'L' to 1 bit,
			// because LCD COM/SEG assignment is special for this incomplete character
			if (segment == LCD_SEG_L2_5) {
				if ((chr == '1') || (chr == 'L')) bits = BIT7;
			}
		}

		// Physically write to LCD memory
		write_lcd_mem(lcdmem, bits, bitmask, mode);
	}
}


// *************************************************************************************************
// @fn          display_chars
// @brief       Write to consecutive 7-segment characters.
// @param       uint8_t segments	LCD segment array
//				uint8_t * str		Pointer to a string
//				uint8_t mode		SEG_ON, SEG_OFF, SEG_BLINK
// @return      none
// *************************************************************************************************
void display_chars(uint8_t segments, uint8_t *str, uint8_t mode)
{
	uint8_t i;
	uint8_t length = 0;			// Write length
	uint8_t char_start;			// Starting point for consecutive write

	//single charakter
	if ((segments >= LCD_SEG_L1_3) && (segments <= LCD_SEG_L2_DP)) {
		length = 1;
		char_start = segments;
	}

	/* TODO: Holly crap! Isn't there a more efficient way to do this? */
	// multiple charakters
	switch (segments) {
		// LINE1
	case LCD_SEG_L1_3_0:
		length = 4;
		char_start = LCD_SEG_L1_3;
		break;

	case LCD_SEG_L1_2_0:
		length = 3;
		char_start = LCD_SEG_L1_2;
		break;

	case LCD_SEG_L1_1_0:
		length = 2;
		char_start = LCD_SEG_L1_1;
		break;

	case LCD_SEG_L1_3_1:
		length = 3;
		char_start = LCD_SEG_L1_3;
		break;

	case LCD_SEG_L1_3_2:
		length = 2;
		char_start = LCD_SEG_L1_3;
		break;

		// LINE2
	case LCD_SEG_L2_5_0:
		length = 6;
		char_start = LCD_SEG_L2_5;
		break;

	case LCD_SEG_L2_4_0:
		length = 5;
		char_start = LCD_SEG_L2_4;
		break;

	case LCD_SEG_L2_3_0:
		length = 4;
		char_start = LCD_SEG_L2_3;
		break;

	case LCD_SEG_L2_2_0:
		length = 3;
		char_start = LCD_SEG_L2_2;
		break;

	case LCD_SEG_L2_1_0:
		length = 2;
		char_start = LCD_SEG_L2_1;
		break;

	case LCD_SEG_L2_5_4:
		length = 2;
		char_start = LCD_SEG_L2_5;
		break;

	case LCD_SEG_L2_5_2:
		length = 4;
		char_start = LCD_SEG_L2_5;
		break;

	case LCD_SEG_L2_3_2:
		length = 2;
		char_start = LCD_SEG_L2_3;
		break;

	case LCD_SEG_L2_4_2:
		length = 3;
		char_start = LCD_SEG_L2_4;
		break;

	case LCD_SEG_L2_4_3:
	default:
		length = 2;
		char_start = LCD_SEG_L2_4;	  //So the char_start variable can't be non-initialized.
	}

	// Write to consecutive digits
	for (i = 0; i < length; i++) {
		// Use single character routine to write display memory
		display_char(char_start + i, (str ? *(str + i) : '8'), mode);
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


// *************************************************************************************************
// @fn          set_blink_rate
// @brief       Set blink rate register bits.
// @param       none
// @return      none
// *************************************************************************************************
void set_blink_rate(uint8_t bits)
{
	LCDBBLKCTL &= ~(BIT7 | BIT6 | BIT5);
	LCDBBLKCTL |= bits;
}


