/**
    openchronos.c: openchronos-ng main loop & user interface

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

// *************************************************************************************************
//
//  Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions
//    are met:
//
//      Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the
//      distribution.
//
//      Neither the name of Texas Instruments Incorporated nor the names of
//      its contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Initialization and control of application.
// *************************************************************************************************

// *************************************************************************************************
// Include section

#include "openchronos.h"

#include "modinit.h"

/* Driver */
#include "messagebus.h"
#include "menu.h"
#include "drivers/display.h"
#include "drivers/vti_as.h"
#include "drivers/vti_ps.h"
#include "drivers/radio.h"
#include "drivers/buzzer.h"
#include "drivers/ports.h"
#include "drivers/timer.h"
#include "drivers/pmm.h"
#include "drivers/rf1a.h"
#include "drivers/rtca.h"
#include "drivers/temperature.h"
#include "drivers/battery.h"

void check_events(void)
{
    enum sys_message msg = SYS_MSG_NONE;

    /* drivers/rtca */
    if (rtca_last_event) {
        msg |= rtca_last_event;
        rtca_last_event = RTCA_EV_NONE;
    }

    /* drivers/timer */
    if (timer0_last_event) {
        msg |= timer0_last_event << 7;
        timer0_last_event = TIMER0_EVENT_NONE;
    }

    /* drivers/accelerometer */
    if(as_last_interrupt){
        msg |= SYS_MSG_AS_INT;
        as_last_interrupt = 0;
    }

#ifdef CONFIG_BATTERY_MONITOR
    /* drivers/battery */
    if ((msg & SYS_MSG_RTC_MINUTE) == SYS_MSG_RTC_MINUTE) {
        msg |= SYS_MSG_BATT;
        battery_measurement();
    }
#endif

    send_events(msg);
}

static void check_buttons(void)
{
#ifdef CONFIG_EASY_DEBUG_RESET
    /* if up and down is pressed then resets the watch */
    if (ports_button_pressed(PORTS_BTN_UP | PORTS_BTN_DOWN, 0))
    {
        /* Forces a reset since a write to WDTCTL isn't allowed without password. */
        WDTCTL = 0;
    }
#endif

#ifdef CONFIG_MOD_ALARM
#endif

    menu_check_buttons();
}

/***************************************************************************
 ************************ INITIALIZATION ROUTINE ***************************
 **************************************************************************/

void init_application(void)
{
    // ---------------------------------------------------------------------
    // Enable watchdog

    // Watchdog triggers after 16 seconds when not cleared
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
#else
    WDTCTL = WDTPW + WDTHOLD;
#endif

    // ---------------------------------------------------------------------
    // Configure port mapping

    // Disable all interrupts
    __disable_interrupt();
    // Get write-access to port mapping registers:
    PMAPPWD = PMAPKEY;
    // Allow reconfiguration during runtime:
    PMAPCTL = PMAPRECFG;

    // P2.7 = TA1CCR0A output (buzzer output)
    P2MAP7 = PM_TA1CCR0A;
    P2OUT &= ~BIT7;
    P2DIR |= BIT7;

    // P1.5 = SPI MISO input
    P1MAP5 = PM_UCA0SOMI;
    // P1.6 = SPI MOSI output
    P1MAP6 = PM_UCA0SIMO;
    // P1.7 = SPI CLK output
    P1MAP7 = PM_UCA0CLK;

    // Disable write-access to port mapping registers:
    PMAPPWD = 0;
    // Re-enable all interrupts
    __enable_interrupt();

    // Init the hardwre real time clock (RTC_A)
    rtca_init();

    // ---------------------------------------------------------------------
    // Configure ports

    // ---------------------------------------------------------------------
    // Reset radio core
    radio_reset();
    radio_powerdown();

#ifdef CONFIG_MOD_ACCELEROMETER
    // ---------------------------------------------------------------------
    // Init acceleration sensor
    as_init();
#else
    as_disconnect();
#endif

    // ---------------------------------------------------------------------
    // Init buttons
    init_buttons();

    // ---------------------------------------------------------------------
    // Configure Timer0 for use by the clock and delay functions
    timer0_init();

    /* Init buzzer */
    buzzer_init();

    // ---------------------------------------------------------------------
    // Init pressure sensor
    ps_init();

    /* drivers/battery */
    battery_init();

    /* drivers/temperature */
    temperature_init();

#ifdef CONFIG_INFOMEM
    if (infomem_ready() == -2) {
        infomem_init(INFOMEM_C, INFOMEM_C + 2 * INFOMEM_SEGMENT_SIZE);
    }
#endif
}

/***************************************************************************
 ************************ ENTRYPOINT AND MAIN LOOP *************************
 **************************************************************************/
int main(void)
{
    /* Show all segments on screen */
    fill_display(0, 0xff);

    /* Init MCU */
    init_application();

    /* clear whole screen */
    display_clear(0, 0);

    /* Init modules */
    mod_init();

    /* main loop */
    while (1) {
        /* Go to LPM3, wait for interrupts */
        _BIS_SR(LPM3_bits + GIE);
        __no_operation();

        /* poll the button driver */
        ports_buttons_poll();

        /* service watchdog on wakeup */
        #ifdef USE_WATCHDOG
            // Service watchdog (reset counter)
            WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
        #endif

        /* check if any driver has events pending */
        check_events();

        /* check for button presses, drive the menu */
        check_buttons();
    }
}


/***************************************************************************
 **************************** HERE BE HELPERS ******************************
 **************************************************************************/
void helpers_loop(uint8_t *value, uint8_t lower, uint8_t upper, int8_t step)
{
    /* Ensure that initial value is between lower and upper interval */
    if (*value > upper) {
        *value = upper;
    }
    if (*value < lower) {
        *value = lower;
    }


    /* for now only increase/decrease on steps of 1 value */
    if (step > 0) {
        /* prevent overflow */
        if (*value == 255) {
            *value = lower;
            return;
        }

        (*value)++;

        if(*value - 1 == upper)
            *value = lower;
    } else {
        /* prevent overflow */
        if (*value == 0) {
            *value = upper;
            return;
        }

        (*value)--;
        if(*value + 1 == lower)
            *value = upper;
    }
}



