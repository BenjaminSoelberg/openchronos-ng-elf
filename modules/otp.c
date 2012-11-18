/**
    Copyright (c) 2011 Yohanes Nugroho (yohanes@gmail.com)

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

    The SHA1 code is based on public domain code  by
    Uwe Hollerbach <uh@alumni.caltech edu>
    from Peter C. Gutmann's implementation as found in
    Applied Cryptography by Bruce Schneier
*/
#include <openchronos.h>
#include <string.h>

#include <drivers/rtca.h>
#include <drivers/display.h>

/* C is used as variable below */
#undef C 

/* 7-segment character bit assignments */
/* Replicated from drivers/display.c (shouldn't this be in display.h ?) */
#define SEG_A     (BIT4)
#define SEG_B     (BIT5)
#define SEG_C     (BIT6)
#define SEG_D     (BIT7)
#define SEG_E     (BIT2)
#define SEG_F     (BIT0)
#define SEG_G     (BIT1)

#define SHA1_BLOCKSIZE     64
#define SHA1_DIGEST_LENGTH 20

/*in this implementation: MAX = 63*/
#define HMAC_KEY_LENGTH 10
#define HMAC_DATA_LENGTH 8

static uint8_t  hmac_key[HMAC_KEY_LENGTH];
static uint32_t sha1_digest[8];
static uint32_t sha1_count;
static uint8_t  sha1_data[SHA1_BLOCKSIZE];
static uint32_t sha1_W[80];
static uint8_t  hmac_tmp_key[64 + HMAC_DATA_LENGTH];
static uint8_t  hmac_sha[SHA1_DIGEST_LENGTH];

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

/* Truncate to 32 bits -- should be a null op on 32-bit machines */
#define T32(x)      ((x) & 0xffffffffL)
#define R32(x,n)    T32(((x << n) | (x >> (32 - n))))

/* The generic case, for when the overall rotation is not unraveled */
#define FG(n)       T = T32(R32(A,5) + f##n(B,C,D) + E + *WP++ + CONST##n);	\
                	E = D; D = C; C = R32(B,30); B = A; A = T

static void sha1_transform()
{
	int i;
	uint8_t *dp;
	uint32_t T, A, B, C, D, E, *WP;

	dp = sha1_data;

    #define SWAP_DONE
	for (i = 0; i < 16; ++i) {
		T = *((uint32_t *) dp);
		dp += 4;
		sha1_W[i] =
			((T << 24) & 0xff000000) |
			((T <<  8) & 0x00ff0000) |
			((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
	}

	for (i = 16; i < 80; ++i) {
		sha1_W[i] = sha1_W[i-3] ^ sha1_W[i-8] ^ sha1_W[i-14] ^ sha1_W[i-16];
		sha1_W[i] = R32(sha1_W[i], 1);
	}

	A = sha1_digest[0];
	B = sha1_digest[1];
	C = sha1_digest[2];
	D = sha1_digest[3];
	E = sha1_digest[4];
	WP = sha1_W;

	for (i =  0; i < 20; ++i) { FG(1); }
	for (i = 20; i < 40; ++i) { FG(2); }
	for (i = 40; i < 60; ++i) { FG(3); }
	for (i = 60; i < 80; ++i) { FG(4); }

	sha1_digest[0] = T32(sha1_digest[0] + A);
	sha1_digest[1] = T32(sha1_digest[1] + B);
	sha1_digest[2] = T32(sha1_digest[2] + C);
	sha1_digest[3] = T32(sha1_digest[3] + D);
	sha1_digest[4] = T32(sha1_digest[4] + E);

}

static void sha1(const uint8_t* data, uint32_t len, uint8_t digest[20])
{
	int i;
	int count;
	uint32_t lo_bit_count;

	sha1_digest[0] = 0x67452301L;
	sha1_digest[1] = 0xefcdab89L;
	sha1_digest[2] = 0x98badcfeL;
	sha1_digest[3] = 0x10325476L;
	sha1_digest[4] = 0xc3d2e1f0L;
	sha1_count = 0L;

	sha1_count = T32(((uint32_t) len << 3));

	while (len >= SHA1_BLOCKSIZE) {
		memcpy(sha1_data, data, SHA1_BLOCKSIZE);
		data += SHA1_BLOCKSIZE;
		len -= SHA1_BLOCKSIZE;
		sha1_transform();
	}
	memcpy(sha1_data, data, len);

	lo_bit_count = sha1_count;

	count = (int) ((lo_bit_count >> 3) & 0x3f);
	((uint8_t *) sha1_data)[count++] = 0x80;
	if (count > SHA1_BLOCKSIZE - 8) {
		memset(((uint8_t *) sha1_data) + count, 0, SHA1_BLOCKSIZE - count);
		sha1_transform();
		memset((uint8_t *) sha1_data, 0, SHA1_BLOCKSIZE - 8);
	} else {
		memset(((uint8_t *) sha1_data) + count, 0,
		       SHA1_BLOCKSIZE - 8 - count);
	}

	sha1_data[56] = 0;
	sha1_data[57] = 0;
	sha1_data[58] = 0;
	sha1_data[59] = 0;
	sha1_data[60] = (uint8_t)((lo_bit_count >> 24) & 0xff);
	sha1_data[61] = (uint8_t)((lo_bit_count >> 16) & 0xff);
	sha1_data[62] = (uint8_t)((lo_bit_count >>  8) & 0xff);
	sha1_data[63] = (uint8_t)((lo_bit_count >>  0) & 0xff);

	sha1_transform();

	//memcpy(digest, sha1_digest, 20);

	count = 0;
	for(i = 0; i<5; i++) {
		digest[count++] = (unsigned char) ((sha1_digest[i] >> 24) & 0xff);
		digest[count++] = (unsigned char) ((sha1_digest[i] >> 16) & 0xff);
		digest[count++] = (unsigned char) ((sha1_digest[i] >> 8) & 0xff);
		digest[count++] = (unsigned char) ((sha1_digest[i]) & 0xff);
	}

}

//data is in hmac_tmp_key + 64
static uint8_t* hmac_sha1(uint8_t *data)
{
	int i;

	// The key for the inner digest is derived from our key, by padding the key
	// the full length of 64 bytes, and then XOR'ing each byte with 0x36.
	for (i = 0; i < HMAC_KEY_LENGTH; ++i) {
		hmac_tmp_key[i] = hmac_key[i] ^ 0x36;
	}
	memset(hmac_tmp_key + HMAC_KEY_LENGTH, 0x36, 64 - HMAC_KEY_LENGTH);
	memcpy(hmac_tmp_key + 64, data, HMAC_DATA_LENGTH);
	sha1(hmac_tmp_key, 64 + HMAC_DATA_LENGTH, hmac_sha);

	// The key for the outer digest is derived from our key, by padding the key
	// the full length of 64 bytes, and then XOR'ing each byte with 0x5C.
	for (i = 0; i < HMAC_KEY_LENGTH; ++i) {
		hmac_tmp_key[i] = hmac_key[i] ^ 0x5C;
	}
	memset(hmac_tmp_key +  HMAC_KEY_LENGTH, 0x5C, 64 - HMAC_KEY_LENGTH);
	memcpy(hmac_tmp_key + 64, hmac_sha, SHA1_DIGEST_LENGTH);
	sha1(hmac_tmp_key, 64 + SHA1_DIGEST_LENGTH, hmac_sha);

	return hmac_sha;
}

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

	return result;
}

const  char     *key          = CONFIG_MOD_OTP_KEY;
static uint32_t  last_time    = 0;
static uint8_t   otp_data[]   = {0,0,0,0,0,0,0,0};
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

	memcpy(hmac_key, key, HMAC_KEY_LENGTH);

	otp_data[4] = (time >> 24) & 0xff;
	otp_data[5] = (time >> 16) & 0xff;
	otp_data[6] = (time >> 8 ) & 0xff;
	otp_data[7] = (time      ) & 0xff;

	hmac_sha1(otp_data);

	int off = hmac_sha[SHA1_DIGEST_LENGTH - 1] & 0x0f;

	char *cc = (char *)&val;
	for (i =0; i < 4; i++) {
		cc[3-i] = hmac_sha[off+i];
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
    sys_messagebus_unregister(&clock_event);

    /* clean up screen */
    display_clear(0, 1);
    display_clear(0, 2);
}

void mod_otp_init()
{
    menu_add_entry("  OTP",
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

