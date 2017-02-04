/**
    Copyright (c) 2011 Yohanes Nugroho (yohanes@gmail.com)
    Copyright (c) 2016-17 Benjamin Sølberg (benjamin.soelberg@gmail.com)
    Copyright (c) 2016-17 menehune dune

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
#include <stdbool.h>
#include "otp.h"
#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/rtca.h"
#include "drivers/display.h"
#include "drivers/buzzer.h"

#if defined(CONFIG_RTC_DST)
#include "drivers/rtc_dst.h"
#endif

/* hmac routines*/
#include "hashutils.h"

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

#if defined(CONFIG_MOD_OTP_SOUND_CUE)
int8_t otp_sound_cue = 0;
int8_t otp_first_code = 1;
#endif

uint32_t simple_mktime(int year, int month, int day, int hour, int minute, int second)
{
    //only works for year 2000 - 2032
    uint32_t    result;

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

const  keystore_t otp_keys[]          = CONFIG_MOD_OTP_KEYS;
#define NUM_ELEMS(x) (sizeof(x)/sizeof(x[0]))
#define NUM_KEYS NUM_ELEMS(otp_keys)


static uint8_t current_key_index = 0;
static uint8_t max_key_index = NUM_KEYS;

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

static void otp_get_current_params(char *potp_identifier, 
        const uint8_t **potp_key, uint8_t *potp_key_len)
{
    const keystore_t *current_key = &otp_keys[current_key_index];
    /*only a single char is supported right now*/
    *potp_identifier = current_key->otp_identifier[0];
    *potp_key = (const uint8_t *)current_key->otp_key;
    *potp_key_len = current_key->otp_key_len;
}


static uint32_t calculate_otp(uint32_t time, const uint8_t *otp_key, 
        uint8_t otp_key_len)
{
    uint32_t val = 0;
    int i;

    memset(otp_data, 0, sizeof(otp_data));
    memset(otp_result, 0, sizeof(otp_result));

    otp_data[4] = (time >> 24) & 0xff;
    otp_data[5] = (time >> 16) & 0xff;
    otp_data[6] = (time >> 8 ) & 0xff;
    otp_data[7] = (time      ) & 0xff;
    

    hmac_sha1(otp_key, otp_key_len, otp_data, sizeof(otp_data),
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

    char otp_identifier;
    const uint8_t *otp_key;
    uint8_t otp_key_len;

    otp_get_current_params(&otp_identifier, &otp_key, &otp_key_len);
    // Draw indicator in lower-left corner
    display_bits(0, LCD_SEG_L2_4, indicator[2*segment  ], SEG_SET);
    display_bits(0, LCD_SEG_L2_4, indicator[2*segment+1], BLINK_SET);
    display_char(0 ,LCD_SEG_L1_3, otp_identifier, SEG_SET); 

    // Calculate timestamp
    uint32_t time = simple_mktime(rtca_time.year, rtca_time.mon - 1, rtca_time.day,
                                  rtca_time.hour, rtca_time.min    , rtca_time.sec);
    time = (time - CONFIG_MOD_OTP_OFFSET * 3600) / 30;

    // Check if new code must be calculated
    if(time != last_time) {

        last_time = time;
        uint32_t otp_value = calculate_otp(time,otp_key, otp_key_len);

        // Draw first half on the top line
        uint16_t v = (otp_value / 1000) % 1000;
        _printf(0, LCD_SEG_L1_2_0, "%03u", v);

        // Draw second half on the bottom line
        v = (otp_value % 1000);
        _printf(0, LCD_SEG_L2_2_0,"%03u", v);
#if defined(CONFIG_MOD_OTP_SOUND_CUE)
        extern note welcome[4];
        if (!otp_first_code && otp_sound_cue)
            buzzer_play(welcome);
        otp_first_code = 0;
#endif
    }
}

static void otp_request_update(bool activated)
{
    enum sys_message ev_type = activated ? SYS_MSG_RTC_SECOND : SYS_MSG_NONE;
#if defined(CONFIG_MOD_OTP_SOUND_CUE)
    otp_first_code = 1;
#endif
    // Force generate & display a new OTP
    last_time = 0;
    clock_event(ev_type);
}

static void otp_activated()
{
    sys_messagebus_register(&clock_event, SYS_MSG_RTC_SECOND);
    display_char(0 ,LCD_SEG_L1_3, '8', BLINK_ON);
#if defined(CONFIG_MOD_OTP_SOUND_CUE)
    display_bits(0, LCD_SEG_L2_3, SEG_G, otp_sound_cue ? SEG_ON : SEG_OFF);
#endif
    otp_request_update(true);
}

static void otp_deactivated()
{
    sys_messagebus_unregister_all(&clock_event);
    display_char(0 ,LCD_SEG_L1_3, '8', BLINK_OFF);
    /* clean up screen */
    display_clear(0, 1);
    display_clear(0, 2);
}

static void otp_gen_next()
{
    if (current_key_index == max_key_index - 1)
        current_key_index = 0;
    else
        current_key_index++;
    otp_request_update(false);
}

static void otp_gen_prev()
{
    if (current_key_index == 0)
        current_key_index = max_key_index - 1;
    else
        current_key_index--;
    otp_request_update(false);
}

#if defined(CONFIG_MOD_OTP_SOUND_CUE)
static void otp_toggle_beep()
{
    otp_sound_cue ^= 1;
    display_bits(0, LCD_SEG_L2_3, SEG_G, otp_sound_cue ? SEG_ON : SEG_OFF);
}
#endif

void mod_otp_init()
{
    menu_add_entry("OTP",
                   &otp_gen_next,      /* up         */
                   &otp_gen_prev,      /* down       */
#if defined(CONFIG_MOD_OTP_SOUND_CUE)
                   &otp_toggle_beep,   /* num        */
#else
                   NULL,               /* num        */
#endif
                   NULL,               /* long star  */
                   NULL,               /* long num   */
                   NULL,               /* up-down    */
                   &otp_activated,     /* activate   */
                   &otp_deactivated);  /* deactivate */
}
