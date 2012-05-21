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

#include "display.h"

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

/***************************************************************************
 ***************************** LOCAL STORAGE *******************************
 **************************************************************************/

/* storage for itoa function */
static uint8_t itoa_str[8];

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


/* Quick integer to array conversion table for most common integer values */
static const uint8_t itoa_conversion_table[][3] = {
	"000", "001", "002", "003", "004", "005", "006", "007", "008", "009", "010", "011", "012", "013", "014", "015",
	"016", "017", "018", "019", "020", "021", "022", "023", "024", "025", "026", "027", "028", "029", "030", "031",
	"032", "033", "034", "035", "036", "037", "038", "039", "040", "041", "042", "043", "044", "045", "046", "047",
	"048", "049", "050", "051", "052", "053", "054", "055", "056", "057", "058", "059", "060", "061", "062", "063",
	"064", "065", "066", "067", "068", "069", "070", "071", "072", "073", "074", "075", "076", "077", "078", "079",
	"080", "081", "082", "083", "084", "085", "086", "087", "088", "089", "090", "091", "092", "093", "094", "095",
	"096", "097", "098", "099", "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111",
	"112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125", "126", "127",
	"128", "129", "130", "131", "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142", "143",
	"144", "145", "146", "147", "148", "149", "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
	"160", "161", "162", "163", "164", "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
	"176", "177", "178", "179", "180",
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

/*
	lcd_screen_create()
	creates a virtual screen where modules can display stuff
*/
inline void lcd_screen_create(struct lcd_screen *screen)
{
	screen->segmem = malloc(12);
	screen->blkmem = malloc(12);
	memset(screen->segmem, 0, 12);
	memset(screen->blkmem, 0, 12);
}

/*
	lcd_screen_destroy()
	destroys a virtual screen
*/
inline void lcd_screen_destroy(struct lcd_screen *screen)
{
	free(screen->segmem);
	free(screen->blkmem);
}


/*
	lcd_screen_real_to_virtual()
	copies the real screen into a virtual screen
*/
inline void lcd_screen_real_to_virtual(struct lcd_screen *screen)
{
	memcpy(screen->segmem, LCD_MEM_1, 12);
	memcpy(screen->blkmem, LCD_MEM_1 + 0x20, 12);
}


/*
	lcd_screen_virtual_to_real()
	copies a virtual screen into the real one
*/
inline void lcd_screen_virtual_to_real(struct lcd_screen *screen)
{
	memcpy(LCD_MEM_1, screen->segmem, 12);
	memcpy(LCD_MEM_1 + 0x20, screen->blkmem, 12);
}


void display_clear(struct lcd_screen *screen, uint8_t line)
{
	if (line == 1) {
		display_chars(screen, LCD_SEG_L1_3_0, NULL, SEG_OFF);
		display_symbol(screen, LCD_SEG_L1_DP1, SEG_OFF);
		display_symbol(screen, LCD_SEG_L1_DP0, SEG_OFF);
		display_symbol(screen, LCD_SEG_L1_COL, SEG_OFF);
	} else if (line == 2) {
		display_chars(screen, LCD_SEG_L2_5_0, NULL, SEG_OFF);
		display_symbol(screen, LCD_SEG_L2_DP, SEG_OFF);
		display_symbol(screen, LCD_SEG_L2_COL1, SEG_OFF);
		display_symbol(screen, LCD_SEG_L2_COL0, SEG_OFF);
	} else {
		uint8_t *lcdptr = (screen ? screen->segmem : (uint8_t *)LCD_MEM_1);
		uint8_t i = 1;

		for (; i <= 12; i++) {
			*(lcdptr++) = 0x00;
		}
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


void display_symbol(struct lcd_screen *screen, enum display_segment symbol,
                                               enum display_segstate state)
{
	if (symbol <= LCD_SEG_L2_DP) {
		// Get LCD memory address for symbol from table
		uint8_t *segmem = (uint8_t *)segments_lcdmem[symbol];
		uint8_t *blkmem = segmem + 0x20;

		if (screen) {
			/* get offset */
			uint8_t offset = segmem - LCD_MEM_1;
			
			segmem = screen->segmem + offset;
			blkmem = screen->blkmem + offset;
		}

		// Get bits for symbol from table
		uint8_t bits 	= segments_bitmask[symbol];

		// Write LCD memory
		// (bitmask for symbols equals bits)
		write_lcd_mem(segmem, blkmem, bits, bits, state);
	}
}


void display_char(struct lcd_screen *screen, enum display_segment segment,
                  uint8_t chr, enum display_segstate state)
{
	uint8_t bits, bits1;		// Bits to write

	// Write to single 7-segment character
	if ((segment >= LCD_SEG_L1_3) && (segment <= LCD_SEG_L2_DP)) {
		// Get LCD memory address for segment from table
		uint8_t *segmem = (uint8_t *)segments_lcdmem[segment];
		uint8_t *blkmem = segmem + 0x20;

		if (screen) {
			/* get offset */
			uint8_t offset = segmem - LCD_MEM_1;

			segmem = screen->segmem + offset;
			blkmem = screen->blkmem + offset;
		}

		// Get bitmask for character from table
		uint8_t bitmask = segments_bitmask[segment];

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
		write_lcd_mem(segmem, blkmem, bits, bitmask, state);
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
void display_chars(struct lcd_screen *screen, enum display_segment segments,
                   uint8_t *str, enum display_segstate state)
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
		display_char(screen, char_start + i, (str ? *(str + i) : '8'), state);
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


