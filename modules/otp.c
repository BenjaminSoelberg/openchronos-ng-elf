/**
    Copyright (c) 2011 Yohanes Nugroho (yohanes@gmail.com)
    Copyright (c) 2016 Benjamin Sølberg (benjamin.soelberg@gmail.com)
    Copyright (c) 2016 menehune

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

	This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/


#include <string.h>

#include "sha1.h"
#include "otp.h"

uint8_t tmp_key[64];
uint8_t sha[SHA1_DIGEST_LENGTH];
uint8_t hashed_key[SHA1_DIGEST_LENGTH];

void hmac_sha1(const uint8_t *key, int keyLength,
               const uint8_t *data, int dataLength,
               uint8_t *result, int resultLength) {
  SHA1_INFO ctx;
  int i;
  // Zero out all internal data structures
  memset(hashed_key, 0, sizeof(hashed_key));
  memset(sha, 0, sizeof(sha));
  memset(tmp_key, 0, sizeof(tmp_key));
  memset(&ctx, 0, sizeof(ctx));

#if defined(__COMPILED_OUT__)
  if (keyLength > 64) {
    // The key can be no bigger than 64 bytes. If it is, we'll hash it down to
    // 20 bytes.
    sha1_init(&ctx);
    sha1_update(&ctx, key, keyLength);
    sha1_final(&ctx, hashed_key);
    key = hashed_key;
    keyLength = SHA1_DIGEST_LENGTH;
  }
#endif

  // The key for the inner digest is derived from our key, by padding the key
  // the full length of 64 bytes, and then XOR'ing each byte with 0x36.
  for (i = 0; i < keyLength; ++i) {
    tmp_key[i] = key[i] ^ 0x36;
  }
  if (keyLength < 64) {
    memset(tmp_key + keyLength, 0x36, 64 - keyLength);
  }

  // Compute inner digest
  sha1_init(&ctx);
  sha1_update(&ctx, tmp_key, 64);
  sha1_update(&ctx, data, dataLength);
  sha1_final(&ctx, sha);

  // The key for the outer digest is derived from our key, by padding the key
  // the full length of 64 bytes, and then XOR'ing each byte with 0x5C.
  for (i = 0; i < keyLength; ++i) {
    tmp_key[i] = key[i] ^ 0x5C;
  }
  memset(tmp_key + keyLength, 0x5C, 64 - keyLength);

  // Compute outer digest
  sha1_init(&ctx);
  sha1_update(&ctx, tmp_key, 64);
  sha1_update(&ctx, sha, SHA1_DIGEST_LENGTH);
  sha1_final(&ctx, sha);

  // Copy result to output buffer and truncate or pad as necessary
  memset(result, 0, resultLength);
  if (resultLength > SHA1_DIGEST_LENGTH) {
    resultLength = SHA1_DIGEST_LENGTH;
  }
  memcpy(result, sha, resultLength);

}
/*------------------------Chronos specific stuff-------------------------*/

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/rtca.h"
#include "drivers/display.h"

#if defined(CONFIG_RTC_DST)
#include "drivers/rtc_dst.h"
#endif

/* 7-segment character bit assignments */
/* Replicated from drivers/display.c (shouldn't this be in display.h ?) */
#define SEG_A     (BIT4)
#define SEG_B     (BIT5)
#define SEG_C     (BIT6)
#define SEG_D     (BIT7)
#define SEG_E     (BIT2)
#define SEG_F     (BIT0)
#define SEG_G     (BIT1)

static int days[12] ={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

uint32_t simple_mktime(int year, int month, int day, int hour, int minute, int second)
{
	//only works for year 2000 - 2032
	uint32_t	result;

	year += month / 12;
	month %= 12;
	result = (year - 1970) * 365 + days[month];
	if (month <= 1) year--;

	result += (year - 1968) / 4;
	result += day - 1;
	result  = ((result * 24 + hour) * 60 + minute) * 60 + second;

#if defined(CONFIG_RTC_DST)
        if (rtc_dst_state == RTC_DST_STATE_DST) {
            result = result - 3600;
        }    
#endif

	return result;
}

const  uint8_t *otp_key          = (uint8_t *)CONFIG_MOD_OTP_KEY;
#define CONFIG_MOD_OTP_KEYLEN	(20)
static uint32_t  last_time    = 0;
static uint8_t   otp_data[]   = {0,0,0,0,0,0,0,0};
static uint8_t   otp_result[SHA1_DIGEST_LENGTH];
static uint8_t   indicator[]  = {
    SEG_A+SEG_F+SEG_E+SEG_D+SEG_C+SEG_B, SEG_B,
    SEG_A+SEG_F+SEG_E+SEG_D+SEG_C,       SEG_C,
    SEG_A+SEG_F+SEG_E+SEG_D,             SEG_D,
    SEG_A+SEG_F+SEG_E,                   SEG_E,
    SEG_A+SEG_F,                         SEG_F,
    SEG_A,                               SEG_A
};

static uint32_t calculate_otp(uint32_t time)
{
	uint32_t val = 0;
        int i;
        memset(otp_data, 0, sizeof(otp_data));
        memset(otp_result, 0, sizeof(otp_result));

	otp_data[4] = (time >> 24) & 0xff;
	otp_data[5] = (time >> 16) & 0xff;
	otp_data[6] = (time >> 8 ) & 0xff;
	otp_data[7] = (time      ) & 0xff;

	hmac_sha1(otp_key, CONFIG_MOD_OTP_KEYLEN, otp_data, sizeof(otp_data),
        otp_result, sizeof(otp_result));

	int off = otp_result[SHA1_DIGEST_LENGTH - 1] & 0x0f;

	char *cc = (char *)&val;
	for (i =0; i < 4; i++) {
		cc[3-i] = otp_result[off+i];
	}
	val &= 0x7fffffff;
	val %= 1000000;

	return val;
}

static void clock_event(enum sys_message msg)
{
    // Check how long the current code is valid
    uint8_t segment = (rtca_time.sec / 5) % 6;

    // Draw indicator in lower-left corner
    display_bits(0, LCD_SEG_L2_4, indicator[2*segment  ], SEG_SET);
    display_bits(0, LCD_SEG_L2_4, indicator[2*segment+1], BLINK_SET);

    // Calculate timestamp
	uint32_t time = simple_mktime(rtca_time.year, rtca_time.mon - 1, rtca_time.day,
                                  rtca_time.hour, rtca_time.min    , rtca_time.sec);
	time = (time - CONFIG_MOD_OTP_OFFSET * 3600) / 30;

    // Check if new code must be calculated
    if(time != last_time) {
        last_time = time;
        uint32_t otp_value = calculate_otp(time);

        // Draw first half on the top line
        uint16_t v = (otp_value / 1000) % 1000;
        _printf(0, LCD_SEG_L1_2_0, "%03u", v);

        // Draw second half on the bottom line
        v = (otp_value % 1000);
        _printf(0, LCD_SEG_L2_2_0,"%03u", v);
    }
}

static void otp_activated()
{
    sys_messagebus_register(&clock_event, SYS_MSG_RTC_SECOND);

    // Force generate & display a new OTP
    last_time = 0;
    clock_event(RTCA_EV_SECOND);
}

static void otp_deactivated()
{
    sys_messagebus_unregister_all(&clock_event);

    /* clean up screen */
    display_clear(0, 1);
    display_clear(0, 2);
}

void mod_otp_init()
{
    menu_add_entry("OTP",
        NULL,               /* up         */
        NULL,               /* down       */
        NULL,               /* num        */
        NULL,               /* long star  */
        NULL,               /* long num   */
        NULL,               /* up-down    */
        &otp_activated,     /* activate   */
        &otp_deactivated    /* deactivate */
    );
}
