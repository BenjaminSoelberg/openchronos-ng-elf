/*
 * Copyright 2010 Google Inc.
 * Author: Markus Gutschke
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 *
 * An earlier version of this file was originally released into the public
 * domain by its authors. It has been modified to make the code compile and
 * link as part of the Google Authenticator project. These changes are
 * copyrighted by Google Inc. and released under the Apache License,
 * Version 2.0.
 *
 * The previous authors' terms are included below:
 */

/*****************************************************************************
 *
 * File:    sha1.c
 *
 * Purpose: Implementation of the SHA1 message-digest algorithm.
 *
 * NIST Secure Hash Algorithm
 *   Heavily modified by Uwe Hollerbach <uh@alumni.caltech edu>
 *   from Peter C. Gutmann's implementation as found in
 *   Applied Cryptography by Bruce Schneier
 *   Further modifications to include the "UNRAVEL" stuff, below
 *
 * This code is in the public domain
 *
 *****************************************************************************
*/
/**
    Copyright (c) 2011 Yohanes Nugroho (yohanes@gmail.com)

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


#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/types.h> // Defines BYTE_ORDER, iff _BSD_SOURCE is defined
#include <string.h>

#include "otp.h"

#ifndef TRUNC32
  #define TRUNC32(x)  ((x) & 0xffffffffL)
#endif

/* SHA f()-functions */
#define f1(x,y,z)    ((x & y) | (~x & z))
#define f2(x,y,z)    (x ^ y ^ z)
#define f3(x,y,z)    ((x & y) | (x & z) | (y & z))
#define f4(x,y,z)    (x ^ y ^ z)

/* SHA constants */
#define CONST1        0x5a827999L
#define CONST2        0x6ed9eba1L
#define CONST3        0x8f1bbcdcL
#define CONST4        0xca62c1d6L

/* truncate to 32 bits -- should be a null op on 32-bit machines */
#define T32(x)    ((x) & 0xffffffffL)

/* 32-bit rotate */
#define R32(x,n)    T32(((x << n) | (x >> (32 - n))))

/* the generic case, for when the overall rotation is not unraveled */
#define FG(n)    \
    T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n);    \
    E = D; D = C; C = R32(B,30); B = A; A = T

static void
sha1_transform(SHA1_INFO *sha1_info)
{
    int i;
    uint8_t *dp;
    uint32_t T, A, B, C, D, E, W[80], *WP;

    dp = sha1_info->data;

    for (i = 0; i < 16; ++i) {
        T = *((uint32_t *) dp);
        dp += 4;
        W[i] = 
            ((T << 24) & 0xff000000) |
            ((T <<  8) & 0x00ff0000) |
            ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
    }

    for (i = 16; i < 80; ++i) {
    W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
    W[i] = R32(W[i], 1);
    }
    A = sha1_info->digest[0];
    B = sha1_info->digest[1];
    C = sha1_info->digest[2];
    D = sha1_info->digest[3];
    E = sha1_info->digest[4];
    WP = W;

    for (i =  0; i < 20; ++i) { FG(1); }
    for (i = 20; i < 40; ++i) { FG(2); }
    for (i = 40; i < 60; ++i) { FG(3); }
    for (i = 60; i < 80; ++i) { FG(4); }
    sha1_info->digest[0] = T32(sha1_info->digest[0] + A);
    sha1_info->digest[1] = T32(sha1_info->digest[1] + B);
    sha1_info->digest[2] = T32(sha1_info->digest[2] + C);
    sha1_info->digest[3] = T32(sha1_info->digest[3] + D);
    sha1_info->digest[4] = T32(sha1_info->digest[4] + E);
}

/* initialize the SHA digest */

void
sha1_init(SHA1_INFO *sha1_info)
{
    sha1_info->digest[0] = 0x67452301L;
    sha1_info->digest[1] = 0xefcdab89L;
    sha1_info->digest[2] = 0x98badcfeL;
    sha1_info->digest[3] = 0x10325476L;
    sha1_info->digest[4] = 0xc3d2e1f0L;
    sha1_info->count_lo = 0L;
    sha1_info->count_hi = 0L;
    sha1_info->local = 0;
}

/* update the SHA digest */

void
sha1_update(SHA1_INFO *sha1_info, const uint8_t *buffer, int count)
{
    int i;
    uint32_t clo;

    clo = T32(sha1_info->count_lo + ((uint32_t) count << 3));
    if (clo < sha1_info->count_lo) {
    ++sha1_info->count_hi;
    }
    sha1_info->count_lo = clo;
    sha1_info->count_hi += (uint32_t) count >> 29;
    if (sha1_info->local) {
    i = SHA1_BLOCKSIZE - sha1_info->local;
    if (i > count) {
        i = count;
    }
    memcpy(((uint8_t *) sha1_info->data) + sha1_info->local, buffer, i);
    count -= i;
    buffer += i;
    sha1_info->local += i;
    if (sha1_info->local == SHA1_BLOCKSIZE) {
        sha1_transform(sha1_info);
    } else {
        return;
    }
    }
    while (count >= SHA1_BLOCKSIZE) {
    memcpy(sha1_info->data, buffer, SHA1_BLOCKSIZE);
    buffer += SHA1_BLOCKSIZE;
    count -= SHA1_BLOCKSIZE;
    sha1_transform(sha1_info);
    }
    memcpy(sha1_info->data, buffer, count);
    sha1_info->local = count;
}


static void
sha1_transform_and_copy(unsigned char digest[20], SHA1_INFO *sha1_info)
{
    sha1_transform(sha1_info);
    digest[ 0] = (unsigned char) ((sha1_info->digest[0] >> 24) & 0xff);
    digest[ 1] = (unsigned char) ((sha1_info->digest[0] >> 16) & 0xff);
    digest[ 2] = (unsigned char) ((sha1_info->digest[0] >>  8) & 0xff);
    digest[ 3] = (unsigned char) ((sha1_info->digest[0]      ) & 0xff);
    digest[ 4] = (unsigned char) ((sha1_info->digest[1] >> 24) & 0xff);
    digest[ 5] = (unsigned char) ((sha1_info->digest[1] >> 16) & 0xff);
    digest[ 6] = (unsigned char) ((sha1_info->digest[1] >>  8) & 0xff);
    digest[ 7] = (unsigned char) ((sha1_info->digest[1]      ) & 0xff);
    digest[ 8] = (unsigned char) ((sha1_info->digest[2] >> 24) & 0xff);
    digest[ 9] = (unsigned char) ((sha1_info->digest[2] >> 16) & 0xff);
    digest[10] = (unsigned char) ((sha1_info->digest[2] >>  8) & 0xff);
    digest[11] = (unsigned char) ((sha1_info->digest[2]      ) & 0xff);
    digest[12] = (unsigned char) ((sha1_info->digest[3] >> 24) & 0xff);
    digest[13] = (unsigned char) ((sha1_info->digest[3] >> 16) & 0xff);
    digest[14] = (unsigned char) ((sha1_info->digest[3] >>  8) & 0xff);
    digest[15] = (unsigned char) ((sha1_info->digest[3]      ) & 0xff);
    digest[16] = (unsigned char) ((sha1_info->digest[4] >> 24) & 0xff);
    digest[17] = (unsigned char) ((sha1_info->digest[4] >> 16) & 0xff);
    digest[18] = (unsigned char) ((sha1_info->digest[4] >>  8) & 0xff);
    digest[19] = (unsigned char) ((sha1_info->digest[4]      ) & 0xff);
}

/* finish computing the SHA digest */
void
sha1_final(SHA1_INFO *sha1_info, uint8_t digest[20])
{
    int count;
    uint32_t lo_bit_count, hi_bit_count;

    lo_bit_count = sha1_info->count_lo;
    hi_bit_count = sha1_info->count_hi;
    count = (int) ((lo_bit_count >> 3) & 0x3f);
    ((uint8_t *) sha1_info->data)[count++] = 0x80;
    if (count > SHA1_BLOCKSIZE - 8) {
    memset(((uint8_t *) sha1_info->data) + count, 0, SHA1_BLOCKSIZE - count);
    sha1_transform(sha1_info);
    memset((uint8_t *) sha1_info->data, 0, SHA1_BLOCKSIZE - 8);
    } else {
    memset(((uint8_t *) sha1_info->data) + count, 0,
        SHA1_BLOCKSIZE - 8 - count);
    }
    sha1_info->data[56] = (uint8_t)((hi_bit_count >> 24) & 0xff);
    sha1_info->data[57] = (uint8_t)((hi_bit_count >> 16) & 0xff);
    sha1_info->data[58] = (uint8_t)((hi_bit_count >>  8) & 0xff);
    sha1_info->data[59] = (uint8_t)((hi_bit_count >>  0) & 0xff);
    sha1_info->data[60] = (uint8_t)((lo_bit_count >> 24) & 0xff);
    sha1_info->data[61] = (uint8_t)((lo_bit_count >> 16) & 0xff);
    sha1_info->data[62] = (uint8_t)((lo_bit_count >>  8) & 0xff);
    sha1_info->data[63] = (uint8_t)((lo_bit_count >>  0) & 0xff);
    sha1_transform_and_copy(digest, sha1_info);
}

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
